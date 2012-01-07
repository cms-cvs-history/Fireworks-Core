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
// $Id: FWOverlapTableView.h,v 1.1.2.2 2012/01/06 00:27:34 amraktad Exp $
//

#include "Fireworks/Core/interface/FWGeometryTableViewBase.h"


class FWOverlapTableManager;
class TEvePointSet;
class FWEveOverlap;
class FWGUIValidatingTextEntry;
class FWGeoPathValidator;


class FWOverlapTableView : public FWGeometryTableViewBase
{
public:
   FWOverlapTableView(TEveWindowSlot* iParent, FWColorManager* colMng);
   virtual ~FWOverlapTableView();

   // virtual void setFrom(const FWConfiguration&);
   //   virtual void addTo(FWConfiguration&) const;
   virtual  FWGeometryTableManagerBase*  getTableManager(); 

   void pathListCallback();
   void pathTextEntryCallback();
   void recalculate();
   TEvePointSet* getMarker() const { return m_marker; }
   const char* getPath() const { return m_path.value().c_str(); }

   virtual void setFrom(const FWConfiguration&);

protected:
   virtual TEveElement* getEveGeoElement() const;
   virtual void assertEveGeoElement();

private:
   FWOverlapTableView(const FWOverlapTableView&); // stop default
   const FWOverlapTableView& operator=(const FWOverlapTableView&); // stop default

   // ---------- member data --------------------------------

   FWOverlapTableManager *m_tableManager;

   FWEveOverlap*         m_eveTopNode;
   TEvePointSet*         m_marker;

   FWGUIValidatingTextEntry* m_pathEntry;
   FWGeoPathValidator*       m_pathValidator;

#ifndef __CINT__
   FWStringParameter       m_path; 
   FWDoubleParameter       m_precision;
#endif
   ClassDef(FWOverlapTableView, 0);
};


#endif
