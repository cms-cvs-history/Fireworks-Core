#ifndef Fireworks_Core_FWRPZ2DDataProxyBuilder_h
#define Fireworks_Core_FWRPZ2DDataProxyBuilder_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWRPZ2DDataProxyBuilder
// 
/**\class FWRPZ2DDataProxyBuilder FWRPZ2DDataProxyBuilder.h Fireworks/Core/interface/FWRPZ2DDataProxyBuilder.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Original Author:  
//         Created:  Sat Jan  5 15:02:03 EST 2008
// $Id: FWRPZ2DDataProxyBuilder.h,v 1.4 2008/02/21 16:09:54 chrjones Exp $
//

// system include files
#include <vector>

// user include files
#include "Fireworks/Core/interface/FWModelChangeSignal.h"

// forward declarations
class FWEventItem;
class TEveElementList;
class TEveElement;

class FWRPZ2DDataProxyBuilder
{

   public:
      FWRPZ2DDataProxyBuilder();
      virtual ~FWRPZ2DDataProxyBuilder();

      // ---------- const member functions ---------------------

      // ---------- static member functions --------------------

      // ---------- member functions ---------------------------
      void setItem(const FWEventItem* iItem);
      void buildRhoPhi(TEveElementList** product);
      void buildRhoZ(TEveElementList** product);

      void modelChangesRhoPhi(const FWModelIds&);
      void modelChangesRhoZ(const FWModelIds&);
   
      void addRhoPhiProj(TEveElement*);
      void addRhoZProj(TEveElement*);
      void clearRhoPhiProjs();
      void clearRhoZProjs();
   
   protected:
      virtual void buildRhoPhi(const FWEventItem* iItem, 
                               TEveElementList** product) = 0 ;
      virtual void buildRhoZ(const FWEventItem* iItem, 
                               TEveElementList** product) = 0 ;

      //Override this if you need to special handle selection or other changes
      virtual void modelChangesRhoPhi(const FWModelIds&, TEveElement*);
      virtual void modelChangesRhoZ(const FWModelIds&, TEveElement*);

      FWRPZ2DDataProxyBuilder(const FWRPZ2DDataProxyBuilder&); // stop default

      const FWRPZ2DDataProxyBuilder& operator=(const FWRPZ2DDataProxyBuilder&); // stop default

      // ---------- member data --------------------------------
      const FWEventItem* m_item;

      TEveElementList* m_rhoPhiElements;
      TEveElementList* m_rhoPhiZElements;

      std::vector<TEveElement*> m_rhoPhiProjs;
      std::vector<TEveElement*> m_rhoZProjs;

      std::vector<FWModelId> m_ids;

};


#endif
