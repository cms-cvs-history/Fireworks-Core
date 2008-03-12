#ifndef Fireworks_Core_FWEveLegoViewManager_h
#define Fireworks_Core_FWEveLegoViewManager_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWEveLegoViewManager
// 
/**\class FWEveLegoViewManager FWEveLegoViewManager.h Fireworks/Core/interface/FWEveLegoViewManager.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Original Author:  
//         Created:  Sun Jan  6 22:01:21 EST 2008
// $Id: FWEveLegoViewManager.h,v 1.9 2008/03/07 09:06:48 dmytro Exp $
//

// system include files
#include <string>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

// user include files
#include "Fireworks/Core/interface/FWViewManagerBase.h"

// forward declarations
class TList;
class THStack;
class TH2;
class TH2F;
class TH2C;
class TCanvas;
class FW3DLegoDataProxyBuilder;
class FWEventItem;
class FWGUIManager;
class TGFrame;
class FWEveLegoView;
class FWViewBase;
class TEveCaloDataHist;

struct FWEveLegoModelProxy
{
   boost::shared_ptr<FW3DLegoDataProxyBuilder>   builder;
   TH2F*                           product; //owned by builder
   FWEveLegoModelProxy():product(0){}
   FWEveLegoModelProxy(boost::shared_ptr<FW3DLegoDataProxyBuilder> iBuilder):
    builder(iBuilder),product(0) {}
};

class FWEveLegoViewManager : public FWViewManagerBase
{

   public:
      FWEveLegoViewManager(FWGUIManager*);
      virtual ~FWEveLegoViewManager();

      // ---------- const member functions ---------------------

      // ---------- static member functions --------------------

      // ---------- member functions ---------------------------
      virtual void newEventAvailable();

      virtual void newItem(const FWEventItem*);

      void registerProxyBuilder(const std::string&, 
				const std::string&);
   
      FWViewBase* buildView(TGFrame* iParent);

   protected:
   virtual void modelChangesComing();
   virtual void modelChangesDone();

   private:
      FWEveLegoViewManager(const FWEveLegoViewManager&); // stop default

      const FWEveLegoViewManager& operator=(const FWEveLegoViewManager&); // stop default

      // ---------- member data --------------------------------
      typedef  std::map<std::string,std::vector<std::string> > TypeToBuilders;
      TypeToBuilders m_typeToBuilders;
      std::vector<FWEveLegoModelProxy> m_modelProxies;

      std::vector<boost::shared_ptr<FWEveLegoView> > m_views;
      TEveCaloDataHist* m_data;
      int  m_legoRebinFactor;
};


#endif
