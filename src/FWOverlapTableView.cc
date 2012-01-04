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
// $Id$
//

// system include files

// user include files
#include "Fireworks/Core/src/FWOverlapTableView.h"
#include "Fireworks/Core/src/FWGeometryTableManager.h"


#include "TEveViewer.h"
#include "TEveScene.h"
#include "TEveSceneInfo.h"
#include "TEveWindow.h"

#include "TEvePointSet.h"
#include "TEveManager.h"



FWOverlapTableView::FWOverlapTableView(TEveWindowSlot* iParent, FWColorManager* colMng, TGeoNode* tn, TObjArray* volumes) : 
   FWGeometryTableViewBase(iParent, FWViewType::kOverlapTable, colMng, tn, volumes)
{
   initGeometry(tn, volumes);
   m_frame->MapSubwindows();
   m_frame->Layout();
  m_eveWindow->GetGUICompositeFrame()->Layout();
   m_frame->MapWindow();
}


FWOverlapTableView::~FWOverlapTableView()
{
   if (m_overlapPnts) m_overlapPnts->DecDenyDestroy();
}

//______________________________________________________________________________

void FWOverlapTableView::initGeometry(TGeoNode* iGeoTopNode, TObjArray* iVolumes)
{ 
   m_overlapPnts = new TEvePointSet();
   m_overlapPnts->SetMarkerSize(5);
   m_overlapPnts->SetMainColor(kRed);
   m_overlapPnts->IncDenyDestroy();
   if (iGeoTopNode) m_tableManager->importOverlaps(iGeoTopNode, iVolumes, m_overlapPnts);
   cdTop();
}


void
FWOverlapTableView::populate3DViewsFromConfig()
{
   FWGeometryTableViewBase::populate3DViewsFromConfig();
   return;
   /*
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

            gEve->AddElement(m_overlapPnts);
            TEveSceneInfo* gsi = (TEveSceneInfo*)v->FindChild(Form("SI - GeoScene %s",v->GetElementName()));
            gsi->GetScene()->AddElement(m_overlapPnts);

         }   
      }
      }*/
}
