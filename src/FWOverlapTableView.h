#ifndef Fireworks_Core_FWOverlapTableView_h
#define Fireworks_Core_FWOverlapTableView_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWOverlapTableView
// 
/**\class FWOverlapTableView FWOverlapTableView.h Fireworks/Core/interface/FWOverlapTableView.h

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  
//         Created:  Wed Jan  4 00:06:31 CET 2012
// $Id: FWOverlapTableView.h,v 1.1.2.1 2012/01/04 02:39:46 amraktad Exp $
//

#include "Fireworks/Core/interface/FWGeometryTableViewBase.h"


class FWOverlapTableManager;
class TEvePointSet;
class FWEveOverlap;

class FWOverlapTableView : public FWGeometryTableViewBase
{

public:
   FWOverlapTableView(TEveWindowSlot* iParent, FWColorManager* colMng, TGeoNode* tn, TObjArray* volumes);
   virtual ~FWOverlapTableView();

   // virtual void setFrom(const FWConfiguration&);
   //   virtual void addTo(FWConfiguration&) const;
   virtual  FWGeometryTableManagerBase*  getTableManager(); 

protected:
   virtual void initGeometry(TGeoNode* iGeoTopNode, TObjArray* iVolumes);
   virtual TEveElement* getEveGeoElement() const;
   virtual void assertEveGeoElement();

private:
   FWOverlapTableView(const FWOverlapTableView&); // stop default
   const FWOverlapTableView& operator=(const FWOverlapTableView&); // stop default

   // ---------- member data --------------------------------

   FWOverlapTableManager *m_tableManager;

   FWEveOverlap*         m_eveTopNode;
   TEvePointSet*         m_overlapPnts;
};


#endif
