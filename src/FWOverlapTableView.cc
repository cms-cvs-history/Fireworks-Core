// -*- C++ -*-
//
// Package:     Core
// Class  :     FWOverlapTableView
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  
//         Created:  Wed Jan  4 00:06:35 CET 2012
// $Id: FWOverlapTableView.cc,v 1.1.2.7 2012/01/18 02:38:36 amraktad Exp $
//

// system include files
#include <boost/bind.hpp>

// user include files
#include "Fireworks/Core/src/FWOverlapTableView.h"
#include "Fireworks/Core/src/FWOverlapTableManager.h"
#include "Fireworks/Core/interface/FWGeometryTableViewManager.h"
#include "Fireworks/Core/interface/FWGeoTopNode.h"
#include "Fireworks/Core/interface/CmsShowViewPopup.h"
#include "Fireworks/Core/src/FWPopupMenu.cc"

#include "Fireworks/Core/src/FWGUIValidatingTextEntry.h"
#include "Fireworks/Core/src/FWValidatorBase.h"

#include "TEveScene.h"
#include "TEveSceneInfo.h"
#include "TEveWindow.h"

#include "TEvePointSet.h"
#include "TEveManager.h"


#include "TGeoVolume.h"
#include "TGeoMatrix.h"
#include "TGeoShape.h"
#include "TGeoBBox.h"
#include "TGeoMatrix.h"
#include "TGeoManager.h"

#include "TGLViewer.h"
#include "KeySymbols.h"
#include "TGLabel.h"
#include "TGNumberEntry.h"
#include "TGListBox.h"
#include "TGButton.h"
#include "TEveViewer.h"

static const std::string sUpdateMsg = " Please press Apply button to update overlaps.\n";

enum OvlMenuOptions {
   kVisOff,
   kVisOnOvl,
   kVisOnMother,
   kCamera,
   kTableDebug
};

class FWGeoPathValidator : public FWValidatorBase 
{
public:
   FWOverlapTableView* m_browser;
   mutable std::vector<std::string> m_list;

   FWGeoPathValidator( FWOverlapTableView* v) { m_browser = v;}
   virtual ~FWGeoPathValidator() {}


   virtual void fillOptions(const char* iBegin, const char* iEnd, std::vector<std::pair<boost::shared_ptr<std::string>, std::string> >& oOptions) const 
   {
      oOptions.clear();
      TEveGeoManagerHolder gmgr( FWGeometryTableViewManager::getGeoMangeur());
      std::string bPath = m_browser->m_pathEntry->GetText();
      bool fixed = false;
      if (!bPath.empty()) {
         size_t ps = bPath.size();
         if (ps > 1 && bPath[ps-1] == '/') 
         {
            bPath = bPath.substr(0, ps-1);
            fixed = true;
         }

         if (gGeoManager->GetCurrentNavigator()->CheckPath(bPath.c_str()) == 0 )
         {
            printf("Can't complete. Path %s  not valid \n", bPath.c_str());
            return;
         }
         gGeoManager->cd(bPath.c_str());
         TGeoNode* node = gGeoManager->GetCurrentNode();
         for (int i = 0; i < node->GetNdaughters(); ++i)
         {  
            const char* name = node->GetDaughter(i)->GetName();
            if (fixed)
               oOptions.push_back(std::make_pair(boost::shared_ptr<std::string>(new std::string(name)),std::string(name)));
            else
               oOptions.push_back(std::make_pair(boost::shared_ptr<std::string>(new std::string(name)),"/"+ std::string(name)));
         }
      }
   }
   bool isStringValid(std::string& exp) 
   {
     
      return true;
   }
};

//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

FWOverlapTableView::FWOverlapTableView(TEveWindowSlot* iParent, FWColorManager* colMng) : 
   FWGeometryTableViewBase(iParent, FWViewType::kOverlapTable, colMng),
   m_tableManager(0),
   m_eveTopNode(0),
   m_marker(0),
   m_pathEntry(0),
   m_pathValidator(0),
   m_numEntry(0),
   m_path(this,"Path:", std::string("/cms:World_1/cms:CMSE_1")),
   m_precision(this, "Precision", 1., 0.001, 10.),
   m_rnrOverlap(this, "Overlap", true),
   m_rnrExtrusion(this, "Extrusion", true),
   m_drawPoints(this, "DrawPoints", true),
   m_pointSize(this, "PointSize:", 1l, 0l, 10l)
{

   // top row
   TGHorizontalFrame* hp =  new TGHorizontalFrame(m_frame);

   {
      m_viewBox = new FWViewCombo(hp, this);
      hp->AddFrame( m_viewBox,new TGLayoutHints(kLHintsExpandY, 2, 2, 0, 0));
   }

   {
      TGTextButton* rb = new TGTextButton (hp, "CdTop");
      hp->AddFrame(rb, new TGLayoutHints(kLHintsNormal, 2, 2, 0, 0) );
      rb->Connect("Clicked()","FWOverlapTableView",this,"cdTop()");
   } 
   {
      TGTextButton* rb = new TGTextButton (hp, "CdUp");
      hp->AddFrame(rb, new TGLayoutHints(kLHintsNormal, 2, 2, 0, 0));
      rb->Connect("Clicked()","FWOverlapTableView",this,"cdUp()");
   }

   { 
      hp->AddFrame(new TGLabel(hp, "Path:"), new TGLayoutHints(kLHintsBottom, 10, 0, 0, 2));
      m_pathEntry = new FWGUIValidatingTextEntry(hp);
      m_pathEntry->SetHeight(20);
      m_pathValidator = new FWGeoPathValidator(this);
      m_pathEntry->setValidator(m_pathValidator);
      hp->AddFrame(m_pathEntry, new TGLayoutHints(kLHintsExpandX,  1, 2, 1, 0));
      m_pathEntry->setMaxListBoxHeight(150);
      m_pathEntry->getListBox()->Connect("Selected(int)", "FWOverlapTableView",  this, "pathListCallback()");
      //      m_pathEntry->Connect("ReturnPressed()", "FWOverlapTableView",  this, "pathTextEntryCallback()");
      m_pathEntry->Connect("ReturnPressed()", "FWOverlapTableView",  this, "pathListCallback()");

      gVirtualX->GrabKey( m_pathEntry->GetId(),gVirtualX->KeysymToKeycode((int)kKey_A),  kKeyControlMask, true);
   }

   {
      hp->AddFrame(new TGLabel(hp, "Precision:"), new TGLayoutHints(kLHintsBottom, 10, 0, 0, 2));
      m_numEntry = new TGNumberEntry(hp,  m_precision.value(), 5, -1, TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax, m_precision.min(), m_precision.max());
      hp->AddFrame(m_numEntry, new TGLayoutHints(kLHintsNormal, 2, 2, 0, 0));
     m_numEntry->Connect("ValueSet(Long_t)","FWOverlapTableView",this,"precisionCallback(Long_t)");
     // m_numEntry->Connect("ValueChanged(Long_t)","FWOverlapTableView",this,"precisionCallback1(Long_t)");
   }
   {
      TGTextButton* rb = new TGTextButton (hp, "Apply");
      hp->AddFrame(rb, new TGLayoutHints(kLHintsNormal, 2, 2, 0, 0));
      rb->Connect("Clicked()","FWOverlapTableView",this,"recalculate()");
   }
   m_frame->AddFrame(hp,new TGLayoutHints(kLHintsLeft|kLHintsExpandX, 4, 2, 2, 0));
   m_tableManager = new FWOverlapTableManager(this);

   std::cerr << " FWOverlapTableView::initGeometry \n";
   assertEveGeoElement();
   m_marker = new TEvePointSet();
   m_marker->SetMarkerSize(5);
   m_marker->SetMainColor(kRed);
   m_marker->IncDenyDestroy();

   //   m_tableManager->importOverlaps(m_path.value(), m_precision.value());


   m_drawPoints.changed_.connect(boost::bind(&FWOverlapTableView::drawPoints,this));
  m_pointSize.changed_.connect(boost::bind(&FWOverlapTableView::pointSize,this));
  m_rnrOverlap.changed_.connect(boost::bind(&FWGeometryTableViewBase::refreshTable3D,this));
  m_rnrExtrusion.changed_.connect(boost::bind(&FWGeometryTableViewBase::refreshTable3D,this));
  
   postConst();
}
//______________________________________________________________________________


FWOverlapTableView::~FWOverlapTableView()
{
   if (m_marker) m_marker->DecDenyDestroy();
}

/*
std::string  FWOverlapTableView::getTextEntryPath()
{
   std::string bPath = m_path.value();
   if (!bPath.empty()) {
      size_t ps = bPath.size();
      if (ps > 1 && bPath[ps-1] == '/') 
      {
         bPath = bPath.substr(0, ps-1);
         m_path.set(bPath);
      }
   }

   }*/
//______________________________________________________________________________
FWGeometryTableManagerBase* FWOverlapTableView::getTableManager()
{
   return m_tableManager;
}

//______________________________________________________________________________
void FWOverlapTableView::assertEveGeoElement()
{
   if (!m_eveTopNode )
   {
      m_eveTopNode = new  FWEveOverlap(this);
      m_eveTopNode->SetElementNameTitle("overlapNode", "opverlapNodetitle");
      m_eveTopNode->IncDenyDestroy();

   }
}
//______________________________________________________________________________


TEveElement* FWOverlapTableView::getEveGeoElement() const
{
   return m_eveTopNode;
}

//______________________________________________________________________________
void FWOverlapTableView::precisionCallback(Long_t )
{
   std::cout << " ----------------------------- PRECISION \n" <<  m_numEntry->GetNumber();
   m_precision.set( m_numEntry->GetNumber());
   std::cout << sUpdateMsg;
}


//______________________________________________________________________________

void FWOverlapTableView::pathListCallback()
{ 
   std::cout << "text click ed " << m_pathEntry->GetText()  <<" \n" ;
   std::string bPath = m_pathEntry->GetText();
   TEveGeoManagerHolder gmgr( FWGeometryTableViewManager::getGeoMangeur());
   if (gGeoManager->GetCurrentNavigator()->CheckPath(bPath.c_str()) == 0 )
   {
      printf("Can't complete. Path %s  not valid \n", bPath.c_str());
      return;
   
   }

   std::cout << sUpdateMsg;
}



void FWOverlapTableView::recalculate()
{
   m_path.set(m_pathEntry->GetText());
   m_precision.set(m_numEntry->GetNumber());
   m_tableManager->importOverlaps(m_path.value(), m_precision.value());
   refreshTable3D();
}

void FWOverlapTableView::cdUp()
{
   TString dt = m_pathEntry->GetDisplayText();
   dt.Remove(dt.Last('/'));
   m_pathEntry->SetText(dt.Data(), true);

   std::cout << sUpdateMsg;
}

void FWOverlapTableView::cdTop()
{
   m_pathEntry->SetText("/", true);
   std::cout << sUpdateMsg;
}


//______________________________________________________________________________
void FWOverlapTableView::setFrom(const FWConfiguration& iFrom)
{
  m_enableRedraw = false;
   for(const_iterator it =begin(), itEnd = end();
       it != itEnd;
       ++it) {
      (*it)->setFrom(iFrom);

   }  
   m_viewersConfig = iFrom.valueForKey("Viewers");

   m_pathEntry->SetText(m_path.value().c_str());
   m_pathEntry->SetCursorPosition(m_path.value().size());
   m_numEntry->SetNumber(m_precision.value());
   m_enableRedraw = true;
   recalculate();
}

//______________________________________________________________________________
void FWOverlapTableView::populateController(ViewerParameterGUI& gui) const
{
   gui.requestTab("Style").
      addParam(&m_enableHighlight).
      separator().
      addParam(&m_rnrOverlap).
      addParam(&m_rnrExtrusion).
      separator().
      addParam(&m_drawPoints).
      addParam(&m_pointSize);
}

//______________________________________________________________________________
void FWOverlapTableView::drawPoints()
{
   m_marker->SetRnrSelf(m_drawPoints.value());
   m_marker->ElementChanged();
   gEve->Redraw3D();
}

//______________________________________________________________________________
void FWOverlapTableView::pointSize()
{
   m_marker->SetMarkerSize(m_pointSize.value());
   m_marker->ElementChanged();
   gEve->Redraw3D();
}
//______________________________________________________________________________

void FWOverlapTableView::popupMenu(int x, int y)
{
   FWPopupMenu* nodePopup = new FWPopupMenu();
   nodePopup->AddEntry("Rnr Off Everyting", kVisOff);
   nodePopup->AddEntry("Rnr On Overlaps, Extrusins", kVisOnOvl);
   nodePopup->AddEntry("Rnr On Mother Volumes", kVisOnMother);
   nodePopup->AddSeparator();
   nodePopup->AddEntry("Set Camera Center", kCamera);


   nodePopup->PlaceMenu(x, y,true,true);
   nodePopup->Connect("Activated(Int_t)",
                      "FWOverlapTableView",
                      this,
                      "chosenItem(Int_t)");
}


//______________________________________________________________________________

void FWOverlapTableView::chosenItem(int x)
{
   if (x < 0) {  printf("ERROT FWOverlapTableView::chosenItem"); return;}

   //0.00  if (getTableManager()->refSelected() == 0) return;

   FWGeometryTableManagerBase::NodeInfo& ni = *getTableManager()->refSelected();
   printf("chosen item %s \n", ni.name());

   TGeoVolume* gv = ni.m_node->GetVolume();
   bool resetHome = false;
   if (gv)
   {
      switch (x) {
         case kVisOff:
            // std::cout << "VIS OFF \n";
            for (FWGeometryTableManagerBase::Entries_i i = m_tableManager->refEntries().begin(); i !=  m_tableManager->refEntries().end(); ++i)
            {
               i->resetBit(FWGeometryTableManagerBase::kVisNodeSelf);
               i->resetBit(FWGeometryTableManagerBase::kFlag1);

            }
            break;
         case kVisOnOvl:
            // std::cout << "VIS ON ovl \n";
            for (FWGeometryTableManagerBase::Entries_i i = m_tableManager->refEntries().begin(); i !=  m_tableManager->refEntries().end(); ++i)
            {
               if (i->m_parent > 0 )i->setBit(FWGeometryTableManagerBase::kVisNodeSelf);
               i->setBit(FWGeometryTableManagerBase::kFlag1);
            }
            break;
         case kVisOnMother:
            // std::cout << "VIS On mOTH \n";
            for (FWGeometryTableManagerBase::Entries_i i = m_tableManager->refEntries().begin(); i !=  m_tableManager->refEntries().end(); ++i)
               if (i->m_parent == 0 )i->setBit(FWGeometryTableManagerBase::kVisNodeSelf);
            break;

         case kCamera:
         {
            TGeoHMatrix mtx;
            getTableManager()->getNodeMatrix( ni, mtx);

            static double pnt[3];
            TGeoBBox* bb = static_cast<TGeoBBox*>( ni.m_node->GetVolume()->GetShape());
            const double* origin = bb->GetOrigin();
            mtx.LocalToMaster(origin, pnt);

            TEveElementList* vl = gEve->GetViewers();
            for (TEveElement::List_i it = vl->BeginChildren(); it != vl->EndChildren(); ++it)
            {
               TEveViewer* v = ((TEveViewer*)(*it));
               TString name = v->GetElementName();
               if (name.Contains("3D"))
               {
                  v->GetGLViewer()->SetDrawCameraCenter(true);
                  TGLCamera& cam = v->GetGLViewer()->CurrentCamera();
                  cam.SetExternalCenter(true);
                  cam.SetCenterVec(pnt[0], pnt[1], pnt[2]);
               }
            }
            if (resetHome) gEve->FullRedraw3D(true, true);
            break;
         }
      }
   }
   refreshTable3D();
}


//______________________________________________________________________________
void FWOverlapTableView::refreshTable3D()
{
   if (!m_enableRedraw) return;
  
   getTableManager()->redrawTable(false);
   getEveGeoElement()->ElementChanged();
  
   std::vector<float> pnts;
   int cnt = 0;

   //   std::cout << "WOverlapTableView::refreshTable3D() "<< std::endl;
   for (std::vector<int>::iterator i = m_markerIndices.begin(); i!=m_markerIndices.end(); i++, cnt+=3)
   {
      FWGeometryTableManagerBase::NodeInfo& data = m_tableManager->refEntries().at(*i);
      if ( data.testBit(FWGeometryTableManagerBase::kFlag1) )//&& ((strncmp(data.name(),  "Ovl", 3) && m_rnrExtrusion.value()) ||  (strncmp(data.name(),  "Ext", 3) && m_rnrOverlap.value() ))) 
      {
         pnts.push_back(m_markerVertices[cnt]);
         pnts.push_back(m_markerVertices[cnt+1]);
         pnts.push_back(m_markerVertices[cnt+2]);
      }
   } 
  
   m_marker->SetPolyMarker(int(pnts.size()/3), &pnts[0], 4);
   m_marker->ElementChanged();
   gEve->FullRedraw3D(false, true);
  
}
