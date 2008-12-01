#ifndef Fireworks_Core_FW3DDataProxyBuilder_h
#define Fireworks_Core_FW3DDataProxyBuilder_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FW3DDataProxyBuilder
//
/**\class FW3DDataProxyBuilder FW3DDataProxyBuilder.h Fireworks/Core/interface/FW3DDataProxyBuilder.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Original Author:
//         Created:  Sat Jan  5 15:02:03 EST 2008
// $Id: FW3DDataProxyBuilder.h,v 1.5 2008/11/14 15:32:32 chrjones Exp $
//

// system include files
#include <vector>

// user include files
#include "Fireworks/Core/interface/FW3DDataProxyBuilderFactory.h"
#include "Fireworks/Core/interface/FWModelChangeSignal.h"
#include "Fireworks/Core/interface/FWEvePtr.h"

// forward declarations
class FWEventItem;
class TEveElementList;
class TEveElement;
class FWModelId;
class TEveCalo3D;
class FWEveValueScaler;

class FW3DDataProxyBuilder
{

   public:
      FW3DDataProxyBuilder();
      virtual ~FW3DDataProxyBuilder();

      // ---------- const member functions ---------------------

      // ---------- static member functions --------------------

      // ---------- member functions ---------------------------
      void setItem(const FWEventItem* iItem);
      void setHaveAWindow(bool iFlag);
      void build();

      void modelChanges(const FWModelIds&);
      void itemChanged(const FWEventItem*);

      TEveElement* usedInScene();

   protected:
      const FWEventItem* item() const {
         return m_item;
      }

      std::vector<FWModelId>& ids() {
         return m_ids;
      }
   private:
      virtual void build(const FWEventItem* iItem,
			 TEveElementList** product) = 0 ;


      //Override this if you need to special handle selection or other changes
      virtual void modelChanges(const FWModelIds&, TEveElement*);
      virtual void applyChangesToAllModels(TEveElement* iElements);

      virtual void itemBeingDestroyed(const FWEventItem*);

      void applyChangesToAllModels();

   FW3DDataProxyBuilder(const FW3DDataProxyBuilder&); // stop default

      const FW3DDataProxyBuilder& operator=(const FW3DDataProxyBuilder&); // stop default

      // ---------- member data --------------------------------
      const FWEventItem* m_item;
      FWEvePtr<TEveElementList> m_elementHolder;//Used as a smart pointer for the item created by the builder
      std::vector<FWModelId> m_ids;

      bool m_modelsChanged;
      bool m_haveViews;
      bool m_mustBuild;
};


#endif
