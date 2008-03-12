// -*- C++ -*-
//
// Package:     Core
// Class  :     FWEveLegoView
// 
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Thu Feb 21 11:22:41 EST 2008
// $Id: FWEveLegoView.cc,v 1.3 2008/03/07 09:06:49 dmytro Exp $
//

// system include files
#include <iostream>

#include "TRootEmbeddedCanvas.h"
#include "THStack.h"
#include "TCanvas.h"
#include "TClass.h"
#include "TH2F.h"
#include "TView.h"

#include "TEveScene.h"
#include "TGLViewer.h"
#include "TGLEmbeddedViewer.h"
#include "TEveViewer.h"
#include "TEveManager.h"
#include "TEveElement.h"
#include "TEveCalo.h"

#include "TEveRGBAPalette.h"

// user include files
#include "Fireworks/Core/interface/FWEveLegoView.h"


//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
FWEveLegoView::FWEveLegoView(TGFrame* iParent)
{
   m_pad = new TEvePad;
   TGLEmbeddedViewer* ev = new TGLEmbeddedViewer(iParent, m_pad);
   m_embeddedViewer=ev;
   TEveViewer* nv = new TEveViewer(staticTypeName().c_str());
   nv->SetGLViewer(ev);
   nv->IncDenyDestroy();
   // ev->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
   ev->SetCurrentCamera(TGLViewer::kCameraPerspXOY);
   TEveScene* ns = gEve->SpawnNewScene(staticTypeName().c_str());
   m_scene = ns;
   nv->AddScene(ns);
   m_viewer=nv;
   
   TEveRGBAPalette* pal = new TEveRGBAPalette(0, 100);
   // pal->SetLimits(0, data->GetMaxVal());
   pal->SetLimits(0, 100);
   pal->SetDefaultColor((Color_t)4);
   
   m_lego = new TEveCaloLego();
   m_lego->SetPalette(pal);
   // lego->SetEtaLimits(etaLimLow, etaLimHigh);
   // lego->SetTitle("caloTower Et distribution");
   gEve->AddElement(m_lego, ns);
   gEve->AddToListTree(m_lego, kTRUE);
}

FWEveLegoView::~FWEveLegoView()
{
}

void
FWEveLegoView::draw(TEveCaloDataHist* data)
{
   m_lego->SetData(data);
}


//
// const member functions
//
TGFrame* 
FWEveLegoView::frame() const
{
   return m_embeddedViewer->GetFrame();
}

const std::string& 
FWEveLegoView::typeName() const
{
   return staticTypeName();
}

//
// static member functions
//
const std::string& 
FWEveLegoView::staticTypeName()
{
   static std::string s_name("3D Lego Pro");
   return s_name;
}

