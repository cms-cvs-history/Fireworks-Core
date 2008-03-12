// -*- C++ -*-
//
// Package:     Core
// Class  :     FWEveLegoViewManager
// 
// Implementation:
//     <Notes on implementation>
//
// Original Author:  
//         Created:  Sun Jan  6 22:01:27 EST 2008
// $Id: FWEveLegoViewManager.cc,v 1.17 2008/03/10 07:29:26 dmytro Exp $
//

// system include files
#include <iostream>
#include <boost/bind.hpp>
#include <algorithm>
#include "THStack.h"
#include "TCanvas.h"
#include "TVirtualHistPainter.h"
#include "TH2F.h"
#include "TView.h"
#include "TList.h"
#include "TEveManager.h"
#include "TClass.h"
#include "TColor.h"
#include "TRootEmbeddedCanvas.h"
#include "TEveCaloData.h"

// user include files
#include "Fireworks/Core/interface/FWEveLegoViewManager.h"
#include "Fireworks/Core/interface/FWEveLegoView.h"
#include "Fireworks/Core/interface/FW3DLegoDataProxyBuilder.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/FWGUIManager.h"


//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
FWEveLegoViewManager::FWEveLegoViewManager(FWGUIManager* iGUIMgr):
  FWViewManagerBase("Proxy3DLegoBuilder"),
  m_data(0),
  m_legoRebinFactor(1)
{
   FWGUIManager::ViewBuildFunctor f;
   f=boost::bind(&FWEveLegoViewManager::buildView,
                 this, _1);
   iGUIMgr->registerViewBuilder(FWEveLegoView::staticTypeName(), f);
   
}

FWEveLegoViewManager::~FWEveLegoViewManager()
{
}

//
// member functions
//
FWViewBase* 
FWEveLegoViewManager::buildView(TGFrame* iParent)
{
   if(0==m_data) {
      m_data = new TEveCaloDataHist();
      // m_data->SetMaximum(100);
   }
   boost::shared_ptr<FWEveLegoView> view( new FWEveLegoView(iParent) );
   m_views.push_back(view);
   return view.get();

}


void 
FWEveLegoViewManager::newEventAvailable()
{
  
   if(0==m_data || 0==m_views.size()) return;
   
   m_data = new TEveCaloDataHist(); // it's a smart object, so it will clean up
   
   for ( std::vector<FWEveLegoModelProxy>::iterator proxy =  m_modelProxies.begin();
	 proxy != m_modelProxies.end(); ++proxy ) {
      bool firstTime = (proxy->product == 0);
      TH2* pointer = proxy->product;
      proxy->builder->build( &pointer );
      proxy->product = dynamic_cast<TH2F*>(pointer);
      if (! proxy->product) printf("WARNING: proxy builder failed to initialize product for FWEveLegoViewManager\n");
      if ( firstTime && 0!= proxy->product ){
	 proxy->product->Rebin2D();
      }
      if ( proxy->product ) m_data->AddHistogram(proxy->product);
   }
   
   std::for_each(m_views.begin(), m_views.end(),
                 boost::bind(&FWEveLegoView::draw,_1, m_data) );
}

void 
FWEveLegoViewManager::newItem(const FWEventItem* iItem)
{
  TypeToBuilders::iterator itFind = m_typeToBuilders.find(iItem->name());
  if(itFind != m_typeToBuilders.end()) {
     for ( std::vector<std::string>::const_iterator builderName = itFind->second.begin();
	   builderName != itFind->second.end(); ++builderName )
       {
	  FW3DLegoDataProxyBuilder* builder = 
	    reinterpret_cast<FW3DLegoDataProxyBuilder*>(
							createInstanceOf(TClass::GetClass(typeid(FW3DLegoDataProxyBuilder)),
									 builderName->c_str())
							);
	  if(0!=builder) {
	     boost::shared_ptr<FW3DLegoDataProxyBuilder> pB( builder );
	     builder->setItem(iItem);
	     m_modelProxies.push_back(FWEveLegoModelProxy(pB) );
	  }
       }
  }
}

void 
FWEveLegoViewManager::registerProxyBuilder(const std::string& iType,
					  const std::string& iBuilder)
{
   m_typeToBuilders[iType].push_back(iBuilder);
}

void 
FWEveLegoViewManager::modelChangesComing()
{
}
void 
FWEveLegoViewManager::modelChangesDone()
{
   newEventAvailable();
}


