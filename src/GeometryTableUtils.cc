#include <iostream>


#include "TGeoNode.h"
#include "TGButton.h"


#include "TEveManager.h"
#include "TEveGeoNode.h"
#include "TEveScene.h"
#include "TEveSceneInfo.h"
#include "TEveViewer.h"
#include "TGLViewer.h"

#include "Fireworks/Core/interface/FWGeometryTableViewBase.h"
#include "Fireworks/Core/interface/FWGeoTopNode.h"
#include "Fireworks/Core/interface/FWGeometryTableManagerBase.h"


#include "Fireworks/Core/interface/fwLog.h"

class PipiScene :public TEveScene
{
   friend class FWGeometryTableViewBase;

public:
   PipiScene(FWGeometryTableViewBase* v, const char* n="TEveScene", const char* t=""):TEveScene(n,t), m_GeoViewer(v) 
   { fUseEveSelection = kFALSE; }
   virtual ~PipiScene(){}

   FWGeometryTableViewBase* m_GeoViewer;

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


         m_GeoViewer->popupMenu(win_x, win_y);
      }
   }

};


