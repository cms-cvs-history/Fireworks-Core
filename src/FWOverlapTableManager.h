#ifndef Fireworks_Core_FWOverlapTableManager_h
#define Fireworks_Core_FWOverlapTableManager_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWOverlapTableManager
// 
/**\class FWOverlapTableManager FWOverlapTableManager.h Fireworks/Core/interface/FWOverlapTableManager.h

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  
//         Created:  Wed Jan  4 20:34:38 CET 2012
// $Id$
//

#include "Fireworks/Core/interface/FWGeometryTableManagerBase.h"
class FWOverlapTableView;

class FWOverlapTableManager : public FWGeometryTableManagerBase
{
public:
   FWOverlapTableManager(FWOverlapTableView*);
   virtual ~FWOverlapTableManager();

   virtual void recalculateVisibility();
   void importOverlaps( TGeoNode* , TObjArray*, TEvePointSet*);

protected:
   virtual bool nodeIsParent(const NodeInfo&) const;


private:
   FWOverlapTableManager(const FWOverlapTableManager&); // stop default
   const FWOverlapTableManager& operator=(const FWOverlapTableManager&); // stop default

   // FWOverlapTableView* m_browser;
};


#endif
