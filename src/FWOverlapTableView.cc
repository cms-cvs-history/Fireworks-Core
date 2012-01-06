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
// $Id: FWOverlapTableView.cc,v 1.1.2.1 2012/01/04 02:39:46 amraktad Exp $
//

// system include files

// user include files
#include "Fireworks/Core/src/FWOverlapTableView.h"
#include "Fireworks/Core/src/FWOverlapTableManager.h"
#include "Fireworks/Core/interface/FWGeoTopNode.h"


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

FWOverlapTableView::FWOverlapTableView(TEveWindowSlot* iParent, FWColorManager* colMng, TGeoNode* tn, TObjArray* volumes) : 
   FWGeometryTableViewBase(iParent, FWViewType::kOverlapTable, colMng, tn, volumes),
   m_tableManager(0),
   m_eveTopNode(0)
{

   // top row
      TGHorizontalFrame* hp =  new TGHorizontalFrame(m_frame);

      {
         m_viewBox = new FWViewCombo(hp, this);
         hp->AddFrame( m_viewBox,new TGLayoutHints(kLHintsExpandY, 2, 2, 0, 0));
      }
      m_frame->AddFrame(hp,new TGLayoutHints(kLHintsLeft|kLHintsExpandX, 4, 2, 2, 0));
   m_tableManager = new FWOverlapTableManager(this);
   initGeometry(tn, volumes);
   postConst();
}


FWOverlapTableView::~FWOverlapTableView()
{
   if (m_overlapPnts) m_overlapPnts->DecDenyDestroy();
}

//______________________________________________________________________________
FWGeometryTableManagerBase* FWOverlapTableView::getTableManager()
{
   return m_tableManager;
}
//______________________________________________________________________________

void FWOverlapTableView::initGeometry(TGeoNode* iGeoTopNode, TObjArray* iVolumes)
{ 

   std::cerr << " FWOverlapTableView::initGeometry \n";
   m_overlapPnts = new TEvePointSet();
   m_overlapPnts->SetMarkerSize(5);
   m_overlapPnts->SetMainColor(kRed);
   m_overlapPnts->IncDenyDestroy();
   if (iGeoTopNode) m_tableManager->importOverlaps(iGeoTopNode, iVolumes, m_overlapPnts);
   cdTop();
}

//______________________________________________________________________________
void FWOverlapTableView::assertEveGeoElement()
{
   if (!m_eveTopNode )
   {
      m_eveTopNode = new  FWEveOverlap(this);
      m_eveTopNode->SetElementNameTitle("overlapNode", "opverlapNodetitle");
      m_eveTopNode->IncDenyDestroy();
      gEve->AddElement(m_eveTopNode);
   }
}


TEveElement* FWOverlapTableView::getEveGeoElement() const
{
   return m_eveTopNode;
}
