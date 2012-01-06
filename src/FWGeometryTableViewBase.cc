#include <iostream>

#include "Fireworks/Core/src/GeometryTableUtils.cc"

#include <boost/bind.hpp>
#include <boost/regex.hpp>

#include "Fireworks/Core/interface/FWGeometryTableViewBase.h"
#include "Fireworks/Core/interface/FWGeoTopNode.h"
#include "Fireworks/Core/interface/FWGeometryTableManagerBase.h"
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

//______________________________________________________________________________
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================


Bool_t  FWGeometryTableViewBase::FWViewCombo::HandleButton(Event_t* event) 
{
   if (event->fType == kButtonPress)
   {
      bool map = false;

      FWPopupMenu* m_viewPopup = new FWPopupMenu(0);

      TEveElementList* views = gEve->GetViewers();
      int idx = 0;

      for (TEveElement::List_i it = views->BeginChildren(); it != views->EndChildren(); ++it)
      { 
         TEveViewer* v = ((TEveViewer*)(*it));
         if (strstr( v->GetElementName(), "3D") )
         {     
            bool added = false;          
            m_viewPopup->AddEntry(v->GetElementName(), idx);
            TEveSceneInfo* si = ( TEveSceneInfo*)v->FindChild(Form("SI - %s",v->GetElementName() ));
            if (m_el && si) {
               for (TEveElement::List_i eit = m_el->BeginParents(); eit != m_el->EndParents(); ++eit ){
                  if (*eit == si->GetScene()) {
                     added = true;
                     break;
                  }
               }
            }
            map = true;
            if (added)
               m_viewPopup->CheckEntry(idx);
         }
         ++idx;
      }

      if (map) {

         Window_t wdummy;
         Int_t ax,ay;
         gVirtualX->TranslateCoordinates(GetId(),
                                         gClient->GetDefaultRoot()->GetId(),
                                         event->fX, event->fY, //0,0 in local coordinates
                                         ax,ay, //coordinates of screen
                                         wdummy);


         m_viewPopup->PlaceMenu(ax, ay, true,true);
         m_viewPopup->Connect("Activated(Int_t)",
                              "FWGeometryTableViewBase",
                              const_cast<FWGeometryTableViewBase*>(m_tableView),
                              "selectView(Int_t)");
      }
      else
      {
         fwLog(fwlog::kError) << "No 3D View added. \n";
      }
   }
   return true;
}


FWGeometryTableViewBase::FWGeometryTableViewBase(TEveWindowSlot* iParent,FWViewType::EType type, FWColorManager* colMng, TGeoNode* tn, TObjArray* volumes )
   : FWViewBase(type),
     m_topNodeIdx(this, "TopNodeIndex", -1l, 0, 1e7),
     m_enableHighlight(this,"EnableHiglight", false),
     m_colorManager(colMng),
     m_colorPopup(0),
     m_eveWindow(0),
     m_frame(0),
     m_viewBox(0),
     m_viewersConfig(0)
{
   m_eveWindow = iParent->MakeFrame(0);
   TGCompositeFrame* xf = m_eveWindow->GetGUICompositeFrame();

   m_frame = new TGVerticalFrame(xf);


   xf->AddFrame(m_frame, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));   
}

void FWGeometryTableViewBase::postConst()
{
   m_settersFrame = new TGHorizontalFrame(m_frame);
   m_frame->AddFrame( m_settersFrame, new TGLayoutHints(kLHintsExpandX,4,2,2,2));
   m_settersFrame->SetCleanup(kDeepCleanup);

   m_tableWidget = new FWTableWidget(getTableManager(), m_frame); 
   m_frame->AddFrame(m_tableWidget,new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,2,2,0,0));
   m_tableWidget->SetBackgroundColor(0xffffff);
   m_tableWidget->SetLineSeparatorColor(0x000000);
   m_tableWidget->SetHeaderBackgroundColor(0xececec);
   m_tableWidget->Connect("cellClicked(Int_t,Int_t,Int_t,Int_t,Int_t,Int_t)",
                          "FWGeometryTableViewBase",this,
                          "cellClicked(Int_t,Int_t,Int_t,Int_t,Int_t,Int_t)");
   m_tableWidget->disableGrowInWidth();
   resetSetters();


   m_frame->MapSubwindows();
   m_frame->Layout();
  m_eveWindow->GetGUICompositeFrame()->Layout();
   m_frame->MapWindow();
}
//______________________________________________________________________________

FWGeometryTableViewBase::~FWGeometryTableViewBase()
{
   // take out composite frame and delete it directly (zwithout the timeout)
   TGCompositeFrame *frame = m_eveWindow->GetGUICompositeFrame();
   frame->RemoveFrame( m_frame );
   delete m_frame;



   m_eveWindow->DestroyWindowAndSlot();
   delete getTableManager();
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
            //             std::cerr << sname.Data() << std::endl;
            TEveViewer* v = dynamic_cast<TEveViewer*>(viewers->FindChild(sname));

            printf("add new SCENE %s =============================\n",v->GetElementName() );
            TEveScene* s = new PipiScene(this, v->GetElementName());
            gEve->AddElement(s, gEve->GetScenes());
            v->AddScene(s);  
            assertEveGeoElement();
            m_viewBox->setElement(getEveGeoElement());
            printf("dddddd %s \n", getEveGeoElement()->GetElementName());
            s->AddElement(getEveGeoElement());
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
  assertEveGeoElement();
   m_viewBox->setElement(getEveGeoElement());


   if (si == 0) {
      TEveScene* s = new PipiScene(this, v->GetElementName());
      gEve->AddElement(s, gEve->GetScenes());
      s->AddElement(getEveGeoElement());
      v->AddScene(s);
   }
   else
   {
      si->GetScene()->RemoveElement(getEveGeoElement());
   }

   getEveGeoElement()->ElementChanged();

   gEve->Redraw3D();
}

int colorHackRowIdx = -1;
//==============================================================================
void 
FWGeometryTableViewBase::cellClicked(Int_t iRow, Int_t iColumn, Int_t iButton, Int_t iKeyMod, Int_t x, Int_t y)
{
   // getTableManager()->setSelection(iRow, iColumn, iButton);
   int idx = getTableManager()->rowToIndex()[iRow];
   FWGeometryTableManagerBase::NodeInfo& ni = getTableManager()->refEntries()[idx];

   if (iButton == kButton1) 
   {
      if (iColumn == FWGeometryTableManagerBase::kName)
      {
         Window_t wdummy;
         Int_t xLoc,yLoc;
         gVirtualX->TranslateCoordinates(gClient->GetDefaultRoot()->GetId(), m_tableWidget->GetId(),  x, y, xLoc, yLoc, wdummy);  


         bool sel =  getTableManager()->firstColumnClicked(iRow, xLoc);

         if (sel) {
            int idx =getTableManager()->rowToIndex()[iRow];
            TEveElementList* views = gEve->GetViewers();
            for (TEveElement::List_i it = views->BeginChildren(); it != views->EndChildren(); ++it)
            { 
               TEveViewer* v = ((TEveViewer*)(*it));
               for (TEveElement::List_i si = v->BeginChildren(); si != v->EndChildren(); ++si )
               {
                  TEveSceneInfo* xs = dynamic_cast<TEveSceneInfo*>(*si);
                  if (xs->GetScene()->GetUseEveSelection() == kFALSE)
                  {
                    
                     {
                        xs->GetScene()->ClickedPhysical(idx+1, 1, 1);
                     }
                  }
               }
            }
         }
         if ( getEveGeoElement()) {
            getEveGeoElement()->ElementChanged();
            gEve->Redraw3D();
         }
      }
      else if (iColumn == FWGeometryTableManagerBase::kColor)
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
         if (iColumn ==  FWGeometryTableManagerBase::kVisSelf)
         {
            if (iRow == 0 )
            {
               fwLog(fwlog::kInfo) << "Top node self-visibility disabled. Change setting in the view controller! \n";
               return;
            }
            else
            {
               getTableManager()->setVisibility(ni, !getTableManager()->getVisibility(ni));
               elementChanged = true;
            }
         }
         if (iColumn ==  FWGeometryTableManagerBase::kVisChild)
         { 
            getTableManager()->setVisibilityChld(ni, !getTableManager()->getVisibilityChld(ni));; 
            elementChanged = true;
         }


         if (elementChanged)
         {
            getEveGeoElement()->ElementChanged();
            gEve->RegisterRedraw3D();
         }
      }
        

      getTableManager()->dataChanged();

   }
   else if (iColumn == FWGeometryTableManagerBase::kName)
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
   FWGeometryTableManagerBase::NodeInfo& ni = *getTableManager()->refSelected();
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
   getTableManager()->setBackgroundToWhite(backgroundIsWhite);
   gClient->NeedRedraw(m_tableWidget);
}

void FWGeometryTableViewBase::nodeColorChangeRequested(Color_t col)
{
   //   printf("color change %d \n", colorHackRowIdx);
   if (colorHackRowIdx >= 0) {
      FWGeometryTableManagerBase::NodeInfo& ni = getTableManager()->refEntries()[colorHackRowIdx];
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
   //getTableManager()->printChildren(-1);
   std::cout << "TODO .... \n";
}

//______________________________________________________________________________


void FWGeometryTableViewBase::cdNode(int idx)
{
   std::string p;
   getTableManager()->getNodePath(idx, p);
   setPath(idx, p);
}

void FWGeometryTableViewBase::cdTop()
{
   std::string path = "/" ;
   path += getTableManager()->refEntries().at(0).name();
   setPath(-1, path ); 
}

void FWGeometryTableViewBase::cdUp()
{   
   if ( getTopNodeIdx() != -1)
   {
      int pIdx   = getTableManager()->refEntries()[getTopNodeIdx()].m_parent;
      std::string p;
      getTableManager()->getNodePath(pIdx, p);
      setPath(pIdx, p);
   }
}

void FWGeometryTableViewBase::setPath(int parentIdx, std::string& path)
{
   m_topNodeIdx.set(parentIdx);
#ifdef PERFTOOL_BROWSER  
   ProfilerStart(Form("cdPath%d.prof", parentIdx));
#endif

   getTableManager()->topGeoNodeChanged(parentIdx);


   refreshTable3D();
   printf("END Set Path to [%s], curren node \n", path.c_str()); 

   getTableManager()->redrawTable();
   //  getEveGeoElement()->ElementChanged();
   // gEve->FullRedraw3D(false, true);

   FWGUIManager::getGUIManager()->updateStatus(path.c_str());
#ifdef PERFTOOL_BROWSER  
   ProfilerStop();
#endif 
}
//______________________________________________________________________________

void FWGeometryTableViewBase::refreshTable3D()
{
   getTableManager()->redrawTable();
   //assertEveGeoElement();
   //   if (getEveGeoElement())  getEveGeoElement()->ElementChanged();
   gEve->FullRedraw3D(false, true);
}
 

//______________________________________________________________________________

void FWGeometryTableViewBase::addTo(FWConfiguration& iTo) const
{
   FWConfigurableParameterizable::addTo(iTo);

   FWConfiguration viewers(1);
   for (TEveElement::List_i it = getEveGeoElement()->BeginParents(); it != getEveGeoElement()->EndParents(); ++it )
   {
      FWConfiguration tempArea;
      TEveScene* scene = dynamic_cast<TEveScene*>(*it);
      std::string n = scene->GetElementName();
      viewers.addKeyValue(n, tempArea);
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

   resetSetters();
   cdNode(m_topNodeIdx.value());
   m_viewersConfig = iFrom.valueForKey("Viewers");
}
