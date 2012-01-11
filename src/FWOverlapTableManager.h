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
// $Id: FWOverlapTableManager.h,v 1.1.2.3 2012/01/07 04:27:41 amraktad Exp $
//

#include "Fireworks/Core/interface/FWGeometryTableManagerBase.h"
class FWOverlapTableView;

class FWOverlapTableManager : public FWGeometryTableManagerBase
{
public:
   FWOverlapTableManager(FWOverlapTableView*);
   virtual ~FWOverlapTableManager();

   virtual void recalculateVisibility();
   void importOverlaps(std::string path, double precision);

protected:
   virtual bool nodeIsParent(const NodeInfo&) const;
   virtual  const char* cellName(const NodeInfo& data) const;

private:
   FWOverlapTableManager(const FWOverlapTableManager&); // stop default
   const FWOverlapTableManager& operator=(const FWOverlapTableManager&); // stop default

   FWOverlapTableView* m_browser;
};


#endif
