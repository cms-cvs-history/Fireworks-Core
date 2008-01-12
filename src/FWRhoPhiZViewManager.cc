// -*- C++ -*-
//
// Package:     Core
// Class  :     FWRhoPhiZViewManager
// 
// Implementation:
//     <Notes on implementation>
//
// Original Author:  
//         Created:  Sat Jan  5 14:08:51 EST 2008
// $Id: FWRhoPhiZViewManager.cc,v 1.1 2008/01/07 05:48:46 chrjones Exp $
//

// system include files
#include <stdexcept>
#include "TEveManager.h"
#include "TEveViewer.h"
#include "TEveBrowser.h"
#include "TEveGeoNode.h"
#include "TSystem.h"
#include "TEveProjectionManager.h"
#include "TEveScene.h"
#include "TGLViewer.h"
#include "TClass.h"
#include "TFile.h"
#include "TEveGeoShapeExtract.h"

#include <iostream>
#include <exception>

// user include files
#include "Fireworks/Core/interface/FWRhoPhiZViewManager.h"
#include "Fireworks/Core/interface/FWRPZDataProxyBuilder.h"
#include "Fireworks/Core/interface/FWRPZ2DDataProxyBuilder.h"
#include "Fireworks/Core/interface/FWEventItem.h"

#include "vis_macros.h"

//
//
// constants, enums and typedefs
//
static
const char* const kBuilderPrefixes[] = {
   "Proxy3DBuilder",
   "ProxyRhoPhiZ2DBuilder"
};
//
// static data member definitions
//

//
// constructors and destructor
//
FWRhoPhiZViewManager::FWRhoPhiZViewManager():
  FWViewManagerBase(kBuilderPrefixes,
                    kBuilderPrefixes+sizeof(kBuilderPrefixes)/sizeof(const char*)),
  m_geom(0),
  m_rhoPhiProjMgr(0),
  m_rhoZProjMgr(0)
{
   //setup projection
   TEveViewer* nv = gEve->SpawnNewViewer("Rho Phi");
   nv->GetGLViewer()->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
   TEveScene* ns = gEve->SpawnNewScene("Rho Phi");
   nv->AddScene(ns);
   
   m_rhoPhiProjMgr = new TEveProjectionManager;
   gEve->AddToListTree(m_rhoPhiProjMgr,kTRUE);
   gEve->AddElement(m_rhoPhiProjMgr,ns);
   
   nv = gEve->SpawnNewViewer("Rho Z");
   nv->GetGLViewer()->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
   ns = gEve->SpawnNewScene("Rho Z");
   nv->AddScene(ns);
   
   m_rhoZProjMgr = new TEveProjectionManager;
   m_rhoZProjMgr->SetProjection(TEveProjection::PT_RhoZ);
   gEve->AddToListTree(m_rhoZProjMgr,kTRUE);
   gEve->AddElement(m_rhoZProjMgr,ns);
   

  //kTRUE tells it to reset the camera so we see everything 
  gEve->Redraw3D(kTRUE);  
}

// FWRhoPhiZViewManager::FWRhoPhiZViewManager(const FWRhoPhiZViewManager& rhs)
// {
//    // do actual copying here;
// }

//FWRhoPhiZViewManager::~FWRhoPhiZViewManager()
//{
//}

//
// assignment operators
//
// const FWRhoPhiZViewManager& FWRhoPhiZViewManager::operator=(const FWRhoPhiZViewManager& rhs)
// {
//   //An exception safe implementation is
//   FWRhoPhiZViewManager temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void 
FWRhoPhiZViewManager::newEventAvailable()
{
  using namespace std;
  if(0==gEve) {
    cout <<"Eve not initialized"<<endl;
    return;
  }

  {
     //while inside this scope, do not let
     // Eve do any redrawing
     TEveManager::TRedrawDisabler disableRedraw(gEve);

     // build models
     for ( std::vector<FWRPZ3DModelProxy>::iterator proxy = m_3dmodelProxies.begin();
          proxy != m_3dmodelProxies.end(); ++proxy ) {
        proxy->builder->build(&(proxy->product) );
     }

     for ( std::vector<FWRPZ2DModelProxy>::iterator proxy = m_2dmodelProxies.begin();
          proxy != m_2dmodelProxies.end(); ++proxy ) {
        proxy->builder->buildRhoPhi(&(proxy->rhoPhiProduct) );
        proxy->builder->buildRhoZ(&(proxy->rhoZProduct) );
        
     }
    
     // R-Phi projections
     
     // setup the projection
     // each projection knows what model proxies it needs
     // NOTE: this should be encapsulated and made configurable 
     //       somewhere else.
     m_rhoPhiProjMgr->DestroyElements();
     m_rhoZProjMgr->DestroyElements();
     
	  
     // FIXME - standard way of loading geomtry failed
     // ----------- from here 
     if ( ! m_geom ) {
	TFile f("tracker.root");
	if(not f.IsOpen()) {
	   std::cerr <<"failed to open 'tracker.root'"<<std::endl;
	   throw std::runtime_error("Failed to open 'tracker.root' geometry file");
	}
	TEveGeoShapeExtract* gse = dynamic_cast<TEveGeoShapeExtract*>(f.Get("Tracker"));
	TEveGeoShape* gsre = TEveGeoShape::ImportShapeExtract(gse,0);
	f.Close();
	m_geom = gsre;
     }
     // ---------- to here

     hide_tracker_endcap(m_geom);
     m_rhoPhiProjMgr->ImportElements(m_geom);

     show_tracker_endcap(m_geom);
     m_rhoZProjMgr->ImportElements(m_geom);

     for ( std::vector<FWRPZ3DModelProxy>::iterator proxy = m_3dmodelProxies.begin();
	   proxy != m_3dmodelProxies.end(); ++proxy )  {
       m_rhoPhiProjMgr->ImportElements(proxy->product);
        m_rhoZProjMgr->ImportElements(proxy->product);
     }  
     for ( std::vector<FWRPZ2DModelProxy>::iterator proxy = m_2dmodelProxies.begin();
          proxy != m_2dmodelProxies.end(); ++proxy )  {
        m_rhoPhiProjMgr->ImportElements(proxy->rhoPhiProduct);
        m_rhoZProjMgr->ImportElements(proxy->rhoZProduct);
     }  
  }
}

void 
FWRhoPhiZViewManager::newItem(const FWEventItem* iItem)
{
  TypeToBuilder::iterator itFind = m_typeToBuilder.find(iItem->name());
  if(itFind != m_typeToBuilder.end()) {
     if(itFind->second.second) {
        FWRPZDataProxyBuilder* builder = reinterpret_cast<
        FWRPZDataProxyBuilder*>( 
                                createInstanceOf(TClass::GetClass(typeid(FWRPZDataProxyBuilder)),
                                                 itFind->second.first.c_str())
                                );
        if(0!=builder) {
           boost::shared_ptr<FWRPZDataProxyBuilder> pB( builder );
           builder->setItem(iItem);
           m_3dmodelProxies.push_back(FWRPZ3DModelProxy(pB) );
        }
     } else {
        FWRPZ2DDataProxyBuilder* builder = reinterpret_cast<
        FWRPZ2DDataProxyBuilder*>( 
                                createInstanceOf(TClass::GetClass(typeid(FWRPZ2DDataProxyBuilder)),
                                                 itFind->second.first.c_str())
                                );
        if(0!=builder) {
           boost::shared_ptr<FWRPZ2DDataProxyBuilder> pB( builder );
           builder->setItem(iItem);
           m_2dmodelProxies.push_back(FWRPZ2DModelProxy(pB) );
        }
     }
  }
}

void 
FWRhoPhiZViewManager::registerProxyBuilder(const std::string& iType,
					   const std::string& iBuilder)
{
   bool is2dType = false;
   if(iType.find(kBuilderPrefixes[0])) {
      is2dType = true;
   }
   m_typeToBuilder[iType]=make_pair(iBuilder,is2dType);
}

//
// const member functions
//

//
// static member functions
//
