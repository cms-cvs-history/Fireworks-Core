// -*- C++ -*-
//
// Package:     Core
// Class  :     FWGeometryTableView
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  
//         Created:  Wed Jan  4 00:05:34 CET 2012
// $Id: FWGeometryTableView.cc,v 1.22.2.12 2012/01/07 04:26:37 amraktad Exp $
//

// system include files
#include <boost/bind.hpp>

// user include files
#include "Fireworks/Core/src/FWGeometryTableView.h"
#include "Fireworks/Core/interface/FWGeometryTableViewManager.h"
#include "Fireworks/Core/interface/FWViewType.h"
#include "Fireworks/Core/interface/FWGeometryTableManagerBase.h"
#include "Fireworks/Core/interface/CmsShowViewPopup.h"
#include "Fireworks/Core/src/FWGeometryTableManager.h"
#include "Fireworks/Core/interface/fwLog.h"

#include "Fireworks/Core/src/FWGUIValidatingTextEntry.h"
#include "Fireworks/Core/interface/FWGUIManager.h"
#include "Fireworks/Core/src/FWValidatorBase.h"
#include "Fireworks/Core/interface/FWGeoTopNode.h"
#include "Fireworks/Core/src/FWPopupMenu.cc"

#include "KeySymbols.h"
#include "TGButton.h"
#include "TGLabel.h"
#include "TGListBox.h"
#include "TGLViewer.h"
#include "TGeoMatrix.h"
#include "TGeoBBox.h"

#include "TEveViewer.h"
#include "TEveManager.h"
#include "TGeoManager.h"
#include "TGLCamera.h"

//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

enum GeoMenuOptions {
   kSetTopNode,
   kSetTopNodeCam,
   kVisOn,
   kVisOff,
   kInspectMaterial,
   kInspectShape,
   kCamera,
   kTableDebug
};

class FWGeoMaterialValidator : public FWValidatorBase 
{
public:
   struct Material
   {
      TGeoMaterial* g;
      std::string n;
      bool operator< (const Material& x) const { return n < x.n ;}
      Material( TGeoMaterial* x) {  g= x; n = x ? x->GetName() : "<show-all>";}
   };

   FWGeometryTableView* m_browser;
   mutable std::vector<Material> m_list;

   FWGeoMaterialValidator( FWGeometryTableView* v) { m_browser = v;}
   virtual ~FWGeoMaterialValidator() {}


   virtual void fillOptions(const char* iBegin, const char* iEnd, std::vector<std::pair<boost::shared_ptr<std::string>, std::string> >& oOptions) const 
   {
      oOptions.clear();
      std::string part(iBegin,iEnd);
      unsigned int part_size = part.size();

      m_list.clear();
      m_list.push_back(0);

      FWGeometryTableManagerBase::Entries_i it = m_browser->getTableManager()->refEntries().begin();
      std::advance(it, m_browser->getTopNodeIdx());
      int nLevel = it->m_level;
      it++;
      while (it->m_level > nLevel)
      {
         TGeoMaterial* g = it->m_node->GetVolume()->GetMaterial();
         bool duplicate = false;
         for (std::vector<Material>::iterator j = m_list.begin(); j!=m_list.end(); ++j) {
            if (j->g == g) {
               duplicate = true;
               break;
            }
         }
         if (!duplicate)
            m_list.push_back(g);

         ++it;
      }
      std::vector<Material>::iterator startIt = m_list.begin();
      startIt++;
      std::sort(startIt, m_list.end());

      std::string h = "";
      oOptions.push_back(std::make_pair(boost::shared_ptr<std::string>(new std::string(m_list.begin()->n)), h));
      for (std::vector<Material>::iterator i = startIt; i!=m_list.end(); ++i) {
         if(part == (*i).n.substr(0,part_size) )
         {
            //  std::cout << i->n <<std::endl;
            oOptions.push_back(std::make_pair(boost::shared_ptr<std::string>(new std::string((*i).n)), (*i).n.substr(part_size, (*i).n.size()-part_size)));
         }
      }

   }

   bool isStringValid(std::string& exp) 
   {
      if (exp.empty()) return true;

      for (std::vector<Material>::iterator i = m_list.begin(); i!=m_list.end(); ++i) {
         if (exp == (*i).n) 
            return true;
    
      }
      return false;
   }
};


//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

FWGeometryTableView::FWGeometryTableView(TEveWindowSlot* iParent, FWColorManager* colMng)
   : FWGeometryTableViewBase(iParent, FWViewType::kGeometryTable, colMng),
     m_tableManager(0),
     m_filterEntry(0),
     m_filterValidator(0),
     m_eveTopNode(0),   
     m_topNodeIdx(this, "TopNodeIndex", -1l, 0, 1e7),
     m_mode(this, "Mode:", 0l, 0l, 1l),
     m_filter(this,"Materials:",std::string()),
     m_disableTopNode(this,"HideTopNode", true),
     m_autoExpand(this,"ExpandList:", 1l, 0l, 100l),
     m_visLevel(this,"VisLevel:", 3l, 1l, 100l),
     m_visLevelFilter(this,"IgnoreVisLevelOnFilter", true)
{


   // top row
   TGHorizontalFrame* hp =  new TGHorizontalFrame(m_frame);

   {
      TGTextButton* rb = new TGTextButton (hp, "CdTop");
      hp->AddFrame(rb, new TGLayoutHints(kLHintsNormal, 2, 2, 0, 0) );
      rb->Connect("Clicked()","FWGeometryTableView",this,"cdTop()");
   } 
   {
      TGTextButton* rb = new TGTextButton (hp, "CdUp");
      hp->AddFrame(rb, new TGLayoutHints(kLHintsNormal, 2, 2, 0, 0));
      rb->Connect("Clicked()","FWGeometryTableView",this,"cdUp()");
   }
  
   {
      m_viewBox = new FWViewCombo(hp, this);
      hp->AddFrame( m_viewBox,new TGLayoutHints(kLHintsExpandY, 2, 2, 0, 0));
   }
   {
      hp->AddFrame(new TGLabel(hp, "Filter:"), new TGLayoutHints(kLHintsBottom, 10, 0, 0, 2));
      m_filterEntry = new FWGUIValidatingTextEntry(hp);
      m_filterEntry->SetHeight(20);
      m_filterValidator = new FWGeoMaterialValidator(this);
      m_filterEntry->setValidator(m_filterValidator);
      hp->AddFrame(m_filterEntry, new TGLayoutHints(kLHintsExpandX,  1, 2, 1, 0));
      m_filterEntry->setMaxListBoxHeight(150);
      m_filterEntry->getListBox()->Connect("Selected(int)", "FWGeometryTableView",  this, "filterListCallback()");
      m_filterEntry->Connect("ReturnPressed()", "FWGeometryTableView",  this, "filterTextEntryCallback()");

      gVirtualX->GrabKey( m_filterEntry->GetId(),gVirtualX->KeysymToKeycode((int)kKey_A),  kKeyControlMask, true);
   }
   m_frame->AddFrame(hp,new TGLayoutHints(kLHintsLeft|kLHintsExpandX, 4, 2, 2, 0));

   m_tableManager = new FWGeometryTableManager(this);
   {
      TEveGeoManagerHolder gmgr( FWGeometryTableViewManager::getGeoMangeur());
      m_tableManager->loadGeometry( gGeoManager->GetTopNode(), gGeoManager->GetListOfVolumes());
   }
   cdTop();


   m_mode.addEntry(kNode, "Node");
   m_mode.addEntry(kVolume, "Volume");
   
   m_mode.changed_.connect(boost::bind(&FWGeometryTableView::refreshTable3D,this));
   m_autoExpand.changed_.connect(boost::bind(&FWGeometryTableView::refreshTable3D, this));
   m_visLevel.changed_.connect(boost::bind(&FWGeometryTableView::refreshTable3D,this));
   m_visLevelFilter.changed_.connect(boost::bind(&FWGeometryTableView::refreshTable3D,this));


   m_disableTopNode.changed_.connect(boost::bind(&FWGeometryTableViewBase::refreshTable3D,this));
   postConst();
}


FWGeometryTableView::~FWGeometryTableView()
{
}



FWGeometryTableManagerBase* FWGeometryTableView::getTableManager()
{
   return m_tableManager;
}

   //______________________________________________________________________________
void FWGeometryTableView::filterTextEntryCallback()
{
   // std::cout << "text entry click ed \n" ;
   std::string exp = m_filterEntry->GetText();
   if ( m_filterValidator->isStringValid(exp)) 
   {
      updateFilter(exp);
   }
   else
   {
      fwLog(fwlog::kError) << "filter expression not valid." << std::endl;
      return;
   }
}

void FWGeometryTableView::filterListCallback()
{ 
   //   std::cout << "list click ed \n" ;

   std::string exp = m_filterEntry->GetText();
   updateFilter(exp);
}



void FWGeometryTableView::updateFilter(std::string& exp)
{
   // std::cout << "=FWGeometryTableViewBase::updateFilter()" << m_filterEntry->GetText() <<std::endl;
  
   if (exp == m_filterValidator->m_list.begin()->n) 
      exp.clear();

   if (exp == m_filter.value()) return;

   if (exp.empty())
   {
      // std::cout << "FITLER OFF \n";
      for (FWGeometryTableManagerBase::Entries_i i = m_tableManager->refEntries().begin(); i !=  m_tableManager->refEntries().end(); ++i)
      {
         m_tableManager->setVisibility(*i, true);
         m_tableManager->setVisibilityChld(*i, true);
      }

      // NOTE: entry should be cleared automatically
      m_filterEntry->Clear();
   }
  
   m_filter.set(exp);
   m_tableManager->updateFilter();
   refreshTable3D();

}


//==============================================================================

void FWGeometryTableView::populateController(ViewerParameterGUI& gui) const
{
   gui.requestTab("Style").
      addParam(&m_disableTopNode).
      addParam(&m_mode).
      addParam(&m_autoExpand).
      addParam(&m_visLevel).
      addParam(&m_visLevelFilter).
      separator().
      addParam(&m_enableHighlight);
}

//______________________________________________________________________________
void  FWGeometryTableView::assertEveGeoElement()
{
 if (!m_eveTopNode )
   {
      printf("NEWFWEveDetectorGeo -------------------------------\n ");
      m_eveTopNode = new  FWEveDetectorGeo(this);
      m_eveTopNode->IncDenyDestroy();
   }
}

TEveElement* FWGeometryTableView::getEveGeoElement() const
{
   return m_eveTopNode;
}

void
FWGeometryTableView::printTable()
{
   // print all entries
   //getTableManager()->printChildren(-1);
   std::cout << "TODO .... \n";
}

//______________________________________________________________________________


void FWGeometryTableView::cdNode(int idx)
{
   std::string p;
   getTableManager()->getNodePath(idx, p);
   setPath(idx, p);
}

void FWGeometryTableView::cdTop()
{
   std::string path = "/" ;
   path += getTableManager()->refEntries().at(0).name();
   setPath(-1, path ); 
}

void FWGeometryTableView::cdUp()
{   
   if ( getTopNodeIdx() != -1)
   {
      int pIdx   = getTableManager()->refEntries()[getTopNodeIdx()].m_parent;
      std::string p;
      getTableManager()->getNodePath(pIdx, p);
      setPath(pIdx, p);
   }
}
//______________________________________________________________________________

void FWGeometryTableView::setPath(int parentIdx, std::string& path)
{
   m_topNodeIdx.set(parentIdx);
#ifdef PERFTOOL_BROWSER  
   ProfilerStart(Form("cdPath%d.prof", parentIdx));
#endif

   getTableManager()->topGeoNodeChanged(parentIdx);

   printf("END Set Path to [%s], curren node \n", path.c_str()); 

   FWGUIManager::getGUIManager()->updateStatus(path.c_str());
#ifdef PERFTOOL_BROWSER  
   ProfilerStop();
#endif 
   refreshTable3D();

}

void FWGeometryTableView::setFrom(const FWConfiguration& iFrom)
{ 
   FWGeometryTableViewBase::setFrom(iFrom);
   cdNode(m_topNodeIdx.value());
}

//______________________________________________________________________________

void FWGeometryTableView::popupMenu(int x, int y)
{

   FWPopupMenu* nodePopup = new FWPopupMenu();
   nodePopup->AddEntry("Set As Top Node", kSetTopNode);
   nodePopup->AddEntry("Set As Top Node And Camera Center", kSetTopNodeCam);
   nodePopup->AddSeparator();
   nodePopup->AddEntry("Rnr Off For All Children", kVisOff);
   nodePopup->AddEntry("Rnr On For All Children", kVisOn);
   nodePopup->AddSeparator();
   nodePopup->AddEntry("Set Camera Center", kCamera);
   nodePopup->AddSeparator();
   nodePopup->AddEntry("InspectMaterial", kInspectMaterial);
   nodePopup->AddEntry("InspectShape", kInspectShape);
   nodePopup->AddEntry("Table Debug", kTableDebug);

   nodePopup->PlaceMenu(x, y,true,true);
   nodePopup->Connect("Activated(Int_t)",
                      "FWGeometryTableView",
                      this,
                      "chosenItem(Int_t)");
}

//______________________________________________________________________________

void FWGeometryTableView::chosenItem(int x)
{
   FWGeometryTableManagerBase::NodeInfo& ni = *getTableManager()->refSelected();
   printf("chosen item %s \n", ni.name());

   TGeoVolume* gv = ni.m_node->GetVolume();
   bool visible = true;
   bool resetHome = false;
   if (gv)
   {
      switch (x) {
         case kVisOff:
            visible = false;
         case kVisOn:
            getTableManager()->setDaughtersSelfVisibility(visible);
            refreshTable3D();
            break;

         case kInspectMaterial:
            gv->InspectMaterial();
            break;
         case kInspectShape:
            gv->InspectShape();
            break;
         case kTableDebug:
            std::cout << "node name " << ni.name() << "parent " <<getTableManager()->refEntries()[ni.m_parent].name() <<  std::endl;
            break;

         case kSetTopNode:
            cdNode(getTableManager()->m_selectedIdx);
            break;

         case kSetTopNodeCam:
            cdNode(getTableManager()->m_selectedIdx);
            resetHome = true;
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
}
