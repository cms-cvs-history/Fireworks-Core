// -*- C++ -*-
//
// Package:     Core
// Class  :     Context
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Tue Sep 30 14:57:12 EDT 2008
// $Id: Context.cc,v 1.7 2010/02/24 19:11:55 amraktad Exp $
//

// system include files

// user include files
#include "TH2.h"
#include "TEveTrackPropagator.h"
#include "TEveCaloData.h"
#include "Fireworks/Core/interface/fw3dlego_xbins.h"

#include "Fireworks/Core/interface/Context.h"
#include "Fireworks/Core/interface/FWMagField.h"

using namespace fireworks;
//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
Context::Context(FWModelChangeManager* iCM,
                 FWSelectionManager* iSM,
                 FWEventItemsManager* iEM,
                 FWColorManager* iColorM
                 ) :
   m_changeManager(iCM),
   m_selectionManager(iSM),
   m_eventItemsManager(iEM),
   m_colorManager(iColorM),
   m_geom(0),
   m_propagator(0),
   m_magField(0),
   m_caloData(0)
{
   m_magField = new FWMagField();

   m_propagator = new TEveTrackPropagator();
   m_propagator->SetMaxR(123.0);
   m_propagator->SetMaxZ(300.0);
   m_propagator->SetMagFieldObj(m_magField);
   m_propagator->IncDenyDestroy();

   m_caloData = new TEveCaloDataHist();
   m_caloData->IncDenyDestroy();
   Bool_t status = TH1::AddDirectoryStatus();
   TH1::AddDirectory(kFALSE); //Keeps histogram from going into memory
   TH2F* background = new TH2F("background","", 82, fw3dlego::xbins, 72/1, -3.1416, 3.1416);
   TH1::AddDirectory(status);
   m_caloData->AddHistogram(background);
}


Context::~Context()
{
   m_propagator->DecDenyDestroy();
   m_caloData->DecDenyDestroy();
}

//
// static member functions
//


//
// static data member definitions
//
//
// const member functions
//
//
// member functions
