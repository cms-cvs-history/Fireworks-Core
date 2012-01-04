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
// $Id$
//

// system include files
#include <boost/bind.hpp>

// user include files
#include "Fireworks/Core/src/FWGeometryTableView.h"
#include "Fireworks/Core/interface/FWViewType.h"
#include "Fireworks/Core/interface/FWGeometryTableManager.h"
#include "Fireworks/Core/interface/CmsShowViewPopup.h"

#include "TEveWindow.h"
FWGeometryTableView::FWGeometryTableView(TEveWindowSlot* iParent, FWColorManager* colMng, TGeoNode* tn, TObjArray* volumes)
   : FWGeometryTableViewBase(iParent, FWViewType::kGeometryTable, colMng, tn, volumes)
{

   m_mode.addEntry(kNode, "Node");
   m_mode.addEntry(kVolume, "Volume");
   
   m_mode.changed_.connect(boost::bind(&FWGeometryTableView::modeChanged,this));
   m_autoExpand.changed_.connect(boost::bind(&FWGeometryTableView::autoExpandChanged, this));
   m_visLevel.changed_.connect(boost::bind(&FWGeometryTableViewBase::refreshTable3D,this));
   m_visLevelFilter.changed_.connect(boost::bind(&FWGeometryTableViewBase::refreshTable3D,this));


   initGeometry(tn, volumes);
   m_frame->MapSubwindows();
   m_frame->Layout();
   m_eveWindow->GetGUICompositeFrame()->Layout();
   m_frame->MapWindow();
}


//______________________________________________________________________________
FWGeometryTableView::~FWGeometryTableView()
{
}

//______________________________________________________________________________

void FWGeometryTableView::initGeometry(TGeoNode* iGeoTopNode, TObjArray* iVolumes)
{ 
   m_tableManager->loadGeometry(iGeoTopNode, iVolumes);
   cdTop();
}

//______________________________________________________________________________
void FWGeometryTableView::modeChanged()
{
   // reset filter when change mode
    std::cout << "chage mode \n";

   m_tableManager->updateFilter();
   refreshTable3D();
}

//______________________________________________________________________________

void FWGeometryTableView::autoExpandChanged()
{
   m_tableManager->checkExpandLevel();
   m_tableManager->redrawTable();
}


//______________________________________________________________________________

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
