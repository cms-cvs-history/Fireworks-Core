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
// $Id: FWOverlapTableView.h,v 1.1.2.3 2012/01/07 04:27:41 amraktad Exp $
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
   //  void pathTextEntryCallback();
   void recalculate();
   virtual TEvePointSet* getEveMarker() const { return m_marker; }

   virtual void setFrom(const FWConfiguration&);
   virtual void populateController(ViewerParameterGUI&) const;
 
   void drawPoints();
   void pointSize();
   //  std::string getPathTextEntry() const;
  
  virtual void refreshTable3D();
protected:
   virtual TEveElement* getEveGeoElement() const;
   virtual void assertEveGeoElement();

private:  
  
   FWOverlapTableView(const FWOverlapTableView&); // stop default
   const FWOverlapTableView& operator=(const FWOverlapTableView&); // stop default
  
public:
   // ---------- member data --------------------------------

   FWOverlapTableManager *m_tableManager;

   FWEveOverlap*         m_eveTopNode;
   TEvePointSet*         m_marker;

   FWGUIValidatingTextEntry* m_pathEntry;
   FWGeoPathValidator*       m_pathValidator;
  
  std::vector<float>  m_markerVertices;
  std::vector<int>    m_markerIndices;
  
#ifndef __CINT__
   FWStringParameter       m_path; 
   FWDoubleParameter       m_precision;

   FWBoolParameter         m_rnrOverlap;
   FWBoolParameter         m_rnrExtrusion;

   FWBoolParameter         m_drawPoints;
   FWLongParameter         m_pointSize;
   
#endif
   ClassDef(FWOverlapTableView, 0);
};


#endif
