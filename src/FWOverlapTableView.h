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
// $Id$
//

#include "Fireworks/Core/interface/FWGeometryTableViewBase.h"

class TEvePointSet;

class FWOverlapTableView : public FWGeometryTableViewBase
{

public:
   FWOverlapTableView(TEveWindowSlot* iParent, FWColorManager* colMng, TGeoNode* tn, TObjArray* volumes);
   virtual ~FWOverlapTableView();

   // virtual void setFrom(const FWConfiguration&);
   //   virtual void addTo(FWConfiguration&) const;

protected:
   virtual void initGeometry(TGeoNode* iGeoTopNode, TObjArray* iVolumes);

   virtual void populate3DViewsFromConfig();

private:
   FWOverlapTableView(const FWOverlapTableView&); // stop default
   const FWOverlapTableView& operator=(const FWOverlapTableView&); // stop default

   // ---------- member data --------------------------------

   TEvePointSet*           m_overlapPnts;
};


#endif
