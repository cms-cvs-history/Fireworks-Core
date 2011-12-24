#include <iostream>


#include "TGeoNode.h"
#include "TGButton.h"


#include "TEveManager.h"
#include "TEveGeoNode.h"
#include "TEveScene.h"
#include "TEveSceneInfo.h"
#include "TEveViewer.h"
#include "TGLViewer.h"

#include "Fireworks/Core/interface/FWGeometryTableView.h"
#include "Fireworks/Core/interface/FWGeoTopNode.h"
#include "Fireworks/Core/interface/FWGeometryTableManager.h"
#include "Fireworks/Core/src/FWPopupMenu.cc"
#include "Fireworks/Core/src/FWGUIValidatingTextEntry.h"
#include "Fireworks/Core/src/FWValidatorBase.h"


#include "Fireworks/Core/interface/fwLog.h"

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


class PipiScene :public TEveScene
{
   friend class FWGeometryTableView;

public:
   PipiScene(FWGeometryTableView* v, const char* n="TEveScene", const char* t=""):TEveScene(n,t), m_GeoViewer(v) 
   { fUseEveSelection = kFALSE; }
   virtual ~PipiScene(){}

   FWGeometryTableView* m_GeoViewer;

   std::set<UInt_t>& RefSelected() { return fSelectPhyIDs; }
   virtual   void MouseOverPhysical(UInt_t x) 
   {
      if (m_GeoViewer->getEnableHighlight())
      {
         TEveScene::MouseOverPhysical(x);

         m_GeoViewer->getTableManager()->m_highlightIdx = x -1;
         m_GeoViewer->getTableManager()->dataChanged();


         if (HasChildren()) {
            TEveElement* e =  *BeginChildren();
            e->StampColorSelection();
            gEve->DoRedraw3D();
         }
      }
   }

   virtual const char* GetTooltipForHighlightedPhysical()
   {
      if (fHighlightPhyID > 0 && HasChildren())
         return (*BeginChildren())->GetHighlightTooltip();
      else
         return 0;
   }

   virtual   void ClickedPhysical(UInt_t x, UInt_t button,  UInt_t state) 
   {
      // printf("clicked physical %d button %d state %d \n", x, button, state);
      if (button >= 1)
      {
         if (IsPhySelected(x) ) {
            if(state) fSelectPhyIDs.clear();
            fHighlightPhyID = 0;
         }
         else {
            if(state) fSelectPhyIDs.clear();
            fSelectPhyIDs.insert(x);
         }
         m_GeoViewer->getTableManager()->m_selectedIdx = x -1;
         m_GeoViewer->getTableManager()->dataChanged();

         if (HasChildren()) {
            TEveElement* e =  *BeginChildren();
            e->StampColorSelection();
            gEve->DoRedraw3D();
         }
      }

      if (button > 1) {

         Window_t rootw, childw;
         Int_t root_x, root_y, win_x, win_y;
         UInt_t mask;
 
         gVirtualX->QueryPointer(gClient->GetDefaultRoot()->GetId(),
                                 rootw, childw,
                                 root_x, root_y,
                                 win_x, win_y,
                                 mask);

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

         nodePopup->PlaceMenu(win_x, win_y,true,true);
         nodePopup->Connect("Activated(Int_t)",
                            "FWGeometryTableView",
                            const_cast<FWGeometryTableView*>( m_GeoViewer),
                            "chosenItemFrom3DView(Int_t)");
      }
   }

};

//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

class FWGeoMaterialValidator : public FWValidatorBase {
public:
   struct Material
   {
      TGeoMaterial* g;
      std::string n;
      bool operator< (const Material& x) const { return n < x.n ;}
      Material( TGeoMaterial* x) {  g= x; n = x ? x->GetName() : "<show-all>";}
   };

   FWGeometryTableManager* m_table;
   mutable std::vector<Material> m_list;

   FWGeoMaterialValidator( FWGeometryTableManager* t) { m_table = t;}
   virtual ~FWGeoMaterialValidator() {}


   virtual void fillOptions(const char* iBegin, const char* iEnd, std::vector<std::pair<boost::shared_ptr<std::string>, std::string> >& oOptions) const 
   {
      oOptions.clear();
      std::string part(iBegin,iEnd);
      unsigned int part_size = part.size();

      m_list.clear();
      m_list.push_back(0);

      FWGeometryTableManager::Entries_i it = m_table->refSelected();
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

//______________________________________________________________________________
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

class FWViewCombo : public TGTextButton
{
private:
   FWGeometryTableView* m_tableView;
   TEveElement* m_el;

public:
   FWViewCombo(const TGWindow *p, FWGeometryTableView* t): 
      TGTextButton(p, "Select Views", -1, TGButton::GetDefaultGC()(), TGTextButton::GetDefaultFontStruct(), kRaisedFrame | kDoubleBorder  ), m_tableView(t), m_el(0) {}
   virtual ~FWViewCombo() {}
   void setElement(TEveElement* x) {m_el = x;}

   virtual Bool_t  HandleButton(Event_t* event) 
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
                                 "FWGeometryTableView",
                                 const_cast<FWGeometryTableView*>(m_tableView),
                                 "selectView(Int_t)");
         }
         else
         {
            fwLog(fwlog::kError) << "No 3D View added. \n";
         }
      }
      return true;
   }

};
