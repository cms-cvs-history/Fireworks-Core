#include <iostream>

#include "Fireworks/Core/src/GeometryTableUtils.cc"

#include <boost/bind.hpp>
#include <boost/regex.hpp>

#include "Fireworks/Core/interface/FWGeometryTableViewBase.h"
#include "Fireworks/Core/interface/FWGeoTopNode.h"
#include "Fireworks/Core/interface/FWGeometryTableManager.h"
#include "Fireworks/TableWidget/interface/FWTableWidget.h"
#include "Fireworks/Core/interface/FWGUIManager.h"
#include "Fireworks/Core/interface/FWColorManager.h"
#include "Fireworks/Core/interface/FWParameterSetterBase.h"
#include "Fireworks/Core/src/FWColorSelect.h"



#include "TGFileDialog.h"
#include "TGeoNode.h"
#include "TGeoMatrix.h"
#include "TGStatusBar.h"
#include "TGButton.h"
#include "TGLabel.h"
#include "TGLPhysicalShape.h"
#include "TGMenu.h"
#include "TGComboBox.h"
#include "KeySymbols.h"

// #define PERFTOOL_BROWSER
#include "TGeoShape.h"
#include "TGeoBBox.h"
#include "TEveManager.h"
#include "TEveGeoNode.h"
#include "TEveScene.h"
#include "TEveSceneInfo.h"
#include "TEveViewer.h"
#include "TGLViewer.h"
#include "TGLCamera.h"
#ifdef PERFTOOL_BROWSER 
#include <google/profiler.h>
#endif




//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

FWGeometryTableViewBase::FWGeometryTableViewBase(TEveWindowSlot* iParent,FWViewType::EType type, FWColorManager* colMng, TGeoNode* tn, TObjArray* volumes )
   : FWViewBase(type),
     m_topNodeIdx(this, "TopNodeIndex", -1l, 0, 1e7),
     m_enableHighlight(this,"EnableHiglight", false),
     m_mode(this, "Mode:", 0l, 0l, 1l),
     m_filter(this,"Materials:",std::string()),
     m_disableTopNode(this,"HideTopNode", true),
     m_autoExpand(this,"ExpandList:", 1l, 0l, 100l),
     m_visLevel(this,"VisLevel:", 3l, 1l, 100l),
     m_visLevelFilter(this,"IgnoreVisLevelOnFilter", true),
     m_colorManager(colMng),
     m_tableManager(0),
     m_eveTopNode(0),
     m_colorPopup(0),
     m_eveWindow(0),
     m_frame(0),
     m_viewBox(0),
     m_filterEntry(0),
     m_filterValidator(0),
     m_viewersConfig(0)
     //     m_overlapPnts()
{
   m_eveWindow = iParent->MakeFrame(0);
   TGCompositeFrame* xf = m_eveWindow->GetGUICompositeFrame();

   m_frame = new TGVerticalFrame(xf);
   xf->AddFrame(m_frame, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
   m_tableManager = new FWGeometryTableManager(this);
  
   m_disableTopNode.changed_.connect(boost::bind(&FWGeometryTableViewBase::refreshTable3D,this));

   //  m_enableHighlight.changed_.connect(boost::bind(&FWGeometryTableViewBase::enableHighlight,this));

   // top row
   {
      TGHorizontalFrame* hp =  new TGHorizontalFrame(m_frame);
      if (rnrOvl() == false) 
      {
         {
            TGTextButton* rb = new TGTextButton (hp, "CdTop");
            hp->AddFrame(rb, new TGLayoutHints(kLHintsNormal, 2, 2, 0, 0) );
            rb->Connect("Clicked()","FWGeometryTableViewBase",this,"cdTop()");
         } {
            TGTextButton* rb = new TGTextButton (hp, "CdUp");
            hp->AddFrame(rb, new TGLayoutHints(kLHintsNormal, 2, 2, 0, 0));
            rb->Connect("Clicked()","FWGeometryTableViewBase",this,"cdUp()");
         }
      }

      {
         m_viewBox = new FWViewCombo(hp, this);
         hp->AddFrame( m_viewBox,new TGLayoutHints(kLHintsExpandY, 2, 2, 0, 0));
      }
      {
         hp->AddFrame(new TGLabel(hp, rnrOvl() ? "Path: " : "Filter:"), new TGLayoutHints(kLHintsBottom, 10, 0, 0, 2));
         m_filterEntry = new FWGUIValidatingTextEntry(hp);
         m_filterEntry->SetHeight(20);
         m_filterValidator = new FWGeoMaterialValidator(m_tableManager);
         m_filterEntry->setValidator(m_filterValidator);
         hp->AddFrame(m_filterEntry, new TGLayoutHints(kLHintsExpandX,  1, 2, 1, 0));
         m_filterEntry->setMaxListBoxHeight(150);
         m_filterEntry->getListBox()->Connect("Selected(int)", "FWGeometryTableViewBase",  this, "filterListCallback()");
         m_filterEntry->Connect("ReturnPressed()", "FWGeometryTableViewBase",  this, "filterTextEntryCallback()");

         gVirtualX->GrabKey( m_filterEntry->GetId(),gVirtualX->KeysymToKeycode((int)kKey_A),  kKeyControlMask, true);
      }
      m_frame->AddFrame(hp,new TGLayoutHints(kLHintsLeft|kLHintsExpandX, 4, 2, 2, 0));
   }

   m_settersFrame = new TGHorizontalFrame(m_frame);
   m_frame->AddFrame( m_settersFrame, new TGLayoutHints(kLHintsExpandX,4,2,2,2));
   m_settersFrame->SetCleanup(kDeepCleanup);

   m_tableWidget = new FWTableWidget(m_tableManager, m_frame); 
   m_frame->AddFrame(m_tableWidget,new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,2,2,0,0));
   m_tableWidget->SetBackgroundColor(0xffffff);
   m_tableWidget->SetLineSeparatorColor(0x000000);
   m_tableWidget->SetHeaderBackgroundColor(0xececec);
   m_tableWidget->Connect("cellClicked(Int_t,Int_t,Int_t,Int_t,Int_t,Int_t)",
                          "FWGeometryTableViewBase",this,
                          "cellClicked(Int_t,Int_t,Int_t,Int_t,Int_t,Int_t)");
   m_tableWidget->disableGrowInWidth();
   resetSetters();
}

FWGeometryTableViewBase::~FWGeometryTableViewBase()
{
   if (m_eveTopNode)
   {
      while ( m_eveTopNode->HasParents()) {
         TEveElement* x =  *m_eveTopNode->BeginParents();
        x->RemoveElement(m_eveTopNode);
      }
      m_eveTopNode->DecDenyDestroy();
   }

   // take out composite frame and delete it directly (zwithout the timeout)
   TGCompositeFrame *frame = m_eveWindow->GetGUICompositeFrame();
   frame->RemoveFrame( m_frame );
   delete m_frame;



   m_eveWindow->DestroyWindowAndSlot();
   delete m_tableManager;
}

//==============================================================================


void
FWGeometryTableViewBase::populate3DViewsFromConfig()
{
   // post-config 
   if (m_viewersConfig) {
      TEveElementList* viewers = gEve->GetViewers();
      const FWConfiguration::KeyValues* keyVals = m_viewersConfig->keyValues();

      if(0!=keyVals)  
      {
         for(FWConfiguration::KeyValuesIt it = keyVals->begin(); it!= keyVals->end(); ++it) {
    
            TString sname = it->first;
            if (strncmp(sname.Data(), "EventScene ", 11) == false) 
            {
               sname = &sname.Data()[11];

            }
            //  std::cerr << sname.Data() << std::endl;
            TEveViewer* v = dynamic_cast<TEveViewer*>(viewers->FindChild(sname));


            TEveScene* s = new PipiScene(this, v->GetElementName());
            gEve->AddElement(s, gEve->GetScenes());

            v->AddScene(s);  
            if (!m_eveTopNode) {
               m_eveTopNode = new FWGeoTopNode(this);
               m_eveTopNode->IncDenyDestroy();
               m_viewBox->setElement(m_eveTopNode);
            }
            s->AddElement(m_eveTopNode);
         }   
      }
   }
}

void
FWGeometryTableViewBase::resetSetters()
{

   if (!m_settersFrame->GetList()->IsEmpty())
   {
      m_setters.clear();

      TGFrameElement *el = (TGFrameElement*) m_settersFrame->GetList()->First();
      TGHorizontalFrame* f = (TGHorizontalFrame*) el->fFrame;
      m_settersFrame->RemoveFrame(f);
   }

   TGHorizontalFrame* frame =  new TGHorizontalFrame(m_settersFrame);
   m_settersFrame->AddFrame(frame, new TGLayoutHints(kLHintsExpandX,4,2,2,2) );
   m_settersFrame->SetCleanup(kDeepCleanup);
   m_settersFrame->MapSubwindows();
   m_frame->Layout();
}

void
FWGeometryTableViewBase::makeSetter(TGCompositeFrame* frame, FWParameterBase* param) 
{
   boost::shared_ptr<FWParameterSetterBase> ptr( FWParameterSetterBase::makeSetterFor(param) );
   ptr->attach(param, this);
 
   TGFrame* m_frame = ptr->build(frame, false);
   frame->AddFrame(m_frame, new TGLayoutHints(kLHintsExpandX));

   m_setters.push_back(ptr);
}


//==============================================================================

void 
FWGeometryTableViewBase::selectView(int idx)
{
   TEveElement::List_i it = gEve->GetViewers()->BeginChildren();
   std::advance(it, idx);
   TEveViewer* v = (TEveViewer*)(*it);
   TEveSceneInfo* si = (TEveSceneInfo*)v->FindChild(Form("SI - %s",v->GetElementName()));

   if (!m_eveTopNode) {
      m_eveTopNode = new FWGeoTopNode(this);
      m_eveTopNode->IncDenyDestroy();
      m_viewBox->setElement(m_eveTopNode);
   }


   if (si == 0) {
      TEveScene* s = new PipiScene(this, v->GetElementName());
      gEve->AddElement(s, gEve->GetScenes());
      s->AddElement(m_eveTopNode);
      v->AddScene(s);
      if (rnrOvl())
      {
         //         TEveSceneInfo* gsi = (TEveSceneInfo*)v->FindChild(Form("SI - GeoScene %s",v->GetElementName()));
         //  gsi->AddElement(m_overlapPnts);
      }
   }
   else
   {
      si->GetScene()->RemoveElement(m_eveTopNode);
      if (rnrOvl())
      {
         //TEveSceneInfo* gsi = (TEveSceneInfo*)v->FindChild(Form("SI - GeoScene %s",v->GetElementName()));
         // gsi->RemoveElement(m_overlapPnts);
      }
   }

   m_eveTopNode->ElementChanged();

   gEve->Redraw3D();
}

int colorHackRowIdx = -1;
//==============================================================================
void 
FWGeometryTableViewBase::cellClicked(Int_t iRow, Int_t iColumn, Int_t iButton, Int_t iKeyMod, Int_t x, Int_t y)
{
   // m_tableManager->setSelection(iRow, iColumn, iButton);
   int idx = m_tableManager->rowToIndex()[iRow];
   FWGeometryTableManager::NodeInfo& ni = m_tableManager->refEntries()[idx];

   if (iButton == kButton1) 
   {
      if (iColumn == FWGeometryTableManager::kName)
      {
         Window_t wdummy;
         Int_t xLoc,yLoc;
         gVirtualX->TranslateCoordinates(gClient->GetDefaultRoot()->GetId(), m_tableWidget->GetId(),  x, y, xLoc, yLoc, wdummy);  


         bool sel =  m_tableManager->firstColumnClicked(iRow, xLoc);

         if (sel) {
            int idx =m_tableManager->rowToIndex()[iRow];
            TEveElementList* views = gEve->GetViewers();
            for (TEveElement::List_i it = views->BeginChildren(); it != views->EndChildren(); ++it)
            { 
               TEveViewer* v = ((TEveViewer*)(*it));
               for (TEveElement::List_i si = v->BeginChildren(); si != v->EndChildren(); ++si )
               {
                  TEveSceneInfo* xs = dynamic_cast<TEveSceneInfo*>(*si);
                  if (xs->GetScene()->GetUseEveSelection() == kFALSE)
                  {
                     // check volume mode
                     if (m_mode.value() == kVolume)
                     {

                        ((PipiScene*)xs->GetScene())->RefSelected().clear();
                        TGeoVolume* v = ni.m_node->GetVolume();
                        int cnt = 0;
                        for (FWGeometryTableManager::Entries_i j = m_tableManager->refEntries().begin(); j != m_tableManager->refEntries().end(); ++j, ++cnt)
                        {
                           if (j->m_node->GetVolume() == v)
                           {
                              ((PipiScene*)xs->GetScene())->RefSelected().insert(cnt+1);
                           }
                        }
                        xs->GetScene()->ClickedPhysical(idx+1, 1, 0);
                     }/*
                     else if (m_mode.value() == kOverlap)
                     {
                        ((PipiScene*)xs->GetScene())->RefSelected().clear();
                        int np = m_tableManager->overlapPair(idx);
                        if (np >= 0) {
                           ((PipiScene*)xs->GetScene())->RefSelected().insert(np+1);
                        }
                        xs->GetScene()->ClickedPhysical(idx+1, 1, 0);
                        }*/
                     else
                     {
                        xs->GetScene()->ClickedPhysical(idx+1, 1, 1);
                     }
                  }
               }
            }
         }
         else if (m_eveTopNode) {
            m_eveTopNode->ElementChanged();
            gEve->Redraw3D();
            return;
         }
      }
      else if (iColumn == FWGeometryTableManager::kColor)
      { 
         std::vector<Color_t> colors;
         m_colorManager->fillLimitedColors(colors);
      
         if (!m_colorPopup) {
            m_colorPopup = new FWColorPopup(gClient->GetDefaultRoot(), colors.front());
            m_colorPopup->InitContent("", colors);
            m_colorPopup->Connect("ColorSelected(Color_t)","FWGeometryTableViewBase", const_cast<FWGeometryTableViewBase*>(this), "nodeColorChangeRequested(Color_t");
         }
         colorHackRowIdx = idx;
         m_colorPopup->SetName("Selected");
         m_colorPopup->ResetColors(colors, m_colorManager->backgroundColorIndex()==FWColorManager::kBlackIndex);
         m_colorPopup->PlacePopup(x, y, m_colorPopup->GetDefaultWidth(), m_colorPopup->GetDefaultHeight());
         return;
      }
      else
      {
         //         printf("set visibility %s \n", ni.name()); 

         bool elementChanged = false;
         if (iColumn ==  FWGeometryTableManager::kVisSelf)
         {
            if (iRow == 0 && m_disableTopNode.value() )
            {
               fwLog(fwlog::kInfo) << "Top node self-visibility disabled. Change setting in the view controller! \n";
               return;
            }
            else
            {
               m_tableManager->setVisibility(ni, !m_tableManager->getVisibility(ni));
               elementChanged = true;
            }
         }
         if (iColumn ==  FWGeometryTableManager::kVisChild)
         { 
            m_tableManager->setVisibilityChld(ni, !m_tableManager->getVisibilityChld(ni));; 
            elementChanged = true;
         }


         if (m_eveTopNode && elementChanged)
         {
            m_eveTopNode->ElementChanged();
            gEve->RegisterRedraw3D();
         }
      }
        

      m_tableManager->dataChanged();

   }
   else if (iColumn == FWGeometryTableManager::kName)
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

      nodePopup->PlaceMenu(x,y,true,true);
      nodePopup->Connect("Activated(Int_t)",
                         "FWGeometryTableViewBase",
                         const_cast<FWGeometryTableViewBase*>(this),
                         "chosenItem(Int_t)");
   }
}




void FWGeometryTableViewBase::chosenItemFrom3DView(int x)
{
  assert(x >=0);
  chosenItem(x);
}

void FWGeometryTableViewBase::chosenItem(int x)
{
   FWGeometryTableManager::NodeInfo& ni = *m_tableManager->refSelected();
   // printf("chosen item %s \n", ni.name());

   TGeoVolume* gv = ni.m_node->GetVolume();
   bool visible = true;
   bool resetHome = false;
   if (gv)
   {
      switch (x) {
        case kVisOff:
            visible = false;
        case kVisOn:
           m_tableManager->setDaughtersSelfVisibility(visible);
            refreshTable3D();
            break;

         case kInspectMaterial:
            gv->InspectMaterial();
            break;
         case kInspectShape:
            gv->InspectShape();
            break;
         case kTableDebug:
            // std::cout << "node name " << ni.name() << "parent " <<m_tableManager->refEntries()[ni.m_parent].name() <<  std::endl;
            // printf("node expanded [%d] imported[%d] children[%d]\n", ni.m_expanded,m_tableManager->nodeImported(m_tableManager->m_selectedIdx) ,  ni.m_node->GetNdaughters());
            //            m_tableManager->printChildren(
            // m_tableManager->m_selectedIdx);
            m_tableManager->printMaterials();
            break;

         case kSetTopNode:
            cdNode(m_tableManager->m_selectedIdx);
            break;

         case kSetTopNodeCam:
            cdNode(m_tableManager->m_selectedIdx);
            resetHome = true;
         case kCamera:
         {
            TGeoHMatrix mtx;
            m_tableManager->getNodeMatrix( ni, mtx);

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

void FWGeometryTableViewBase::setBackgroundColor()
{
   bool backgroundIsWhite = m_colorManager->backgroundColorIndex()==FWColorManager::kWhiteIndex;
   if(backgroundIsWhite) {
      m_tableWidget->SetBackgroundColor(0xffffff);
      m_tableWidget->SetLineSeparatorColor(0x000000);
   } else {
      m_tableWidget->SetBackgroundColor(0x000000);
      m_tableWidget->SetLineSeparatorColor(0xffffff);
   }
   m_tableManager->setBackgroundToWhite(backgroundIsWhite);
   gClient->NeedRedraw(m_tableWidget);
}

void FWGeometryTableViewBase::nodeColorChangeRequested(Color_t col)
{
   //   printf("color change %d \n", colorHackRowIdx);
   if (colorHackRowIdx >= 0) {
      FWGeometryTableManager::NodeInfo& ni = m_tableManager->refEntries()[colorHackRowIdx];
      ni.m_color = col;
      ni.m_node->GetVolume()->SetLineColor(col);
      refreshTable3D();
      colorHackRowIdx = -1;
   }
}

void
FWGeometryTableViewBase::printTable()
{
   // print all entries
   m_tableManager->printChildren(-1);
}

//______________________________________________________________________________


void FWGeometryTableViewBase::cdNode(int idx)
{
   std::string p;
   m_tableManager->getNodePath(idx, p);
   setPath(idx, p);
}

void FWGeometryTableViewBase::cdTop()
{
   std::string path = "/" ;
   path += m_tableManager->refEntries().at(0).name();
   setPath(-1, path ); 
}

void FWGeometryTableViewBase::cdUp()
{   
   if ( getTopNodeIdx() != -1)
   {
      int pIdx   = m_tableManager->refEntries()[getTopNodeIdx()].m_parent;
      std::string p;
      m_tableManager->getNodePath(pIdx, p);
      setPath(pIdx, p);
   }
}

void FWGeometryTableViewBase::setPath(int parentIdx, std::string& path)
{
   m_topNodeIdx.set(parentIdx);
#ifdef PERFTOOL_BROWSER  
   ProfilerStart(Form("cdPath%d.prof", parentIdx));
#endif

   m_tableManager->topGeoNodeChanged(parentIdx);
   m_tableManager->updateFilter();

   m_tableManager->checkExpandLevel();

   refreshTable3D();
   // printf("END Set Path to [%s], curren node %s \n", m_path.value().c_str(), topNode->GetName()); 

   m_tableManager->redrawTable();
   if ( m_eveTopNode) {
      m_eveTopNode->ElementChanged();
      gEve->FullRedraw3D(false, true);
   } 

   FWGUIManager::getGUIManager()->updateStatus(path.c_str());
#ifdef PERFTOOL_BROWSER  
   ProfilerStop();
#endif 
}
//______________________________________________________________________________
void FWGeometryTableViewBase::filterTextEntryCallback()
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

void FWGeometryTableViewBase::filterListCallback()
{ 
   //   std::cout << "list click ed \n" ;

   std::string exp = m_filterEntry->GetText();
   updateFilter(exp);

}



void FWGeometryTableViewBase::updateFilter(std::string& exp)
{
   // std::cout << "=FWGeometryTableViewBase::updateFilter()" << m_filterEntry->GetText() <<std::endl;
  
   if (exp == m_filterValidator->m_list.begin()->n) 
      exp.clear();

   if (exp == m_filter.value()) return;

   if (exp.empty())
   {
      // std::cout << "FITLER OFF \n";
      for (FWGeometryTableManager::Entries_i i = m_tableManager->refEntries().begin(); i !=  m_tableManager->refEntries().end(); ++i)
      {
         m_tableManager->setVisibility(*i, true);
         m_tableManager->setVisibilityChld(*i, true);
      }

      // NOTE: entry should be cleared automatically
      m_filterEntry->Clear();

      m_tableManager->checkExpandLevel();
   }
  
   m_filter.set(exp);
   m_tableManager->updateFilter();
   refreshTable3D();

}

//______________________________________________________________________________

void FWGeometryTableViewBase::refreshTable3D()
{
   m_tableManager->redrawTable();

   if ( m_eveTopNode) {
      m_eveTopNode->ElementChanged();
      gEve->FullRedraw3D(false, true);
   } 
}


//______________________________________________________________________________

void FWGeometryTableViewBase::addTo(FWConfiguration& iTo) const
{
   FWConfigurableParameterizable::addTo(iTo);

   FWConfiguration viewers(1);
   if (m_eveTopNode)
   { 
      for (TEveElement::List_i it = m_eveTopNode->BeginParents(); it != m_eveTopNode->EndParents(); ++it )
      {
         FWConfiguration tempArea;
         TEveScene* scene = dynamic_cast<TEveScene*>(*it);
         std::string n = scene->GetElementName();
         viewers.addKeyValue(n, tempArea);
      }
   }
   iTo.addKeyValue("Viewers", viewers, true);
}
  
//______________________________________________________________________________

void FWGeometryTableViewBase::setFrom(const FWConfiguration& iFrom)
{ 
   for(const_iterator it =begin(), itEnd = end();
       it != itEnd;
       ++it) {
      (*it)->setFrom(iFrom);

   }  
   m_filterEntry->SetText(m_filter.value().c_str(), false);
   resetSetters();
   cdNode(m_topNodeIdx.value());
   m_viewersConfig = iFrom.valueForKey("Viewers");
}
