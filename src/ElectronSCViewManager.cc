// -*- C++ -*-
//
// Package:     Core
// Class  :     ElectronSCViewManager
// 
// Implementation:
//     <Notes on implementation>
//
// Original Author:  
//         Created:  Sun Jan  6 22:01:27 EST 2008
// $Id: ElectronSCViewManager.cc,v 1.7 2008/02/03 07:20:09 dmytro Exp $
//

// system include files
#include <iostream>
#include "THStack.h"
#include "TCanvas.h"
#include "TVirtualHistPainter.h"
#include "TH2F.h"
#include "TView.h"
#include "TList.h"
#include "TEveManager.h"
#include "TEveViewer.h"
#include "TGLViewer.h"
#include "TClass.h"
#include "TColor.h"

// user include files
#include "Fireworks/Core/interface/ElectronSCViewManager.h"
#include "Fireworks/Core/interface/FWEventItem.h"


//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
ElectronSCViewManager::ElectronSCViewManager():
     FWViewManagerBase("ProxySCBuilder")
{
     //setup projection
     TEveViewer* nv = gEve->SpawnNewViewer("Electron");
     nv->GetGLViewer()->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
     TEveScene* ns = gEve->SpawnNewScene("Electron");
     nv->AddScene(ns);
//      m_projMgr = new TEveProjectionManager;
//      gEve->AddToListTree(m_projMgr, true);
//      gEve->AddElement(m_projMgr, ns);
//      gEve->Redraw3D(true);
}

// ElectronSCViewManager::ElectronSCViewManager(const ElectronSCViewManager& rhs)
// {
//    // do actual copying here;
// }

ElectronSCViewManager::~ElectronSCViewManager()
{
}

//
// assignment operators
//
// const ElectronSCViewManager& ElectronSCViewManager::operator=(const ElectronSCViewManager& rhs)
// {
//   //An exception safe implementation is
//   ElectronSCViewManager temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void 
ElectronSCViewManager::newEventAvailable()
{
     for (std::vector<ElectronSCModelProxy>::iterator proxy = 
	       m_modelProxies.begin();
	  proxy != m_modelProxies.end(); ++proxy ) {
	  proxy->builder->build( &(proxy->product) );
     }
}

void 
ElectronSCViewManager::newItem(const FWEventItem* iItem)
{
     TypeToBuilder::iterator itFind = m_typeToBuilder.find(iItem->name());
     if(itFind != m_typeToBuilder.end()) {
	  printf("ElectronSCViewManager: adding item... ");
     	  ElectronsProxySCBuilder *builder = 
	       reinterpret_cast<ElectronsProxySCBuilder *>( 
		    createInstanceOf(
			 TClass::GetClass(typeid(ElectronsProxySCBuilder)),
			 itFind->second.c_str()));
	  if (builder != 0) {
	       printf("added\n");
	       boost::shared_ptr<ElectronsProxySCBuilder> pB( builder );
	       builder->setItem(iItem);
	       m_modelProxies.push_back(ElectronSCModelProxy(pB) );
	  } else printf("not added\n");
     }
}

void 
ElectronSCViewManager::registerProxyBuilder(const std::string& iType,
					    const std::string& iBuilder)
{
     m_typeToBuilder[iType] = iBuilder;
     printf("ElectronSCViewManager: registering %s, %s\n", iType.c_str(), 
	    iBuilder.c_str());
}

void 
ElectronSCViewManager::modelChangesComing()
{
}
void 
ElectronSCViewManager::modelChangesDone()
{
   newEventAvailable();
}

//
// const member functions
//

//
// static member functions
//
