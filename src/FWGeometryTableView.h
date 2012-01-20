#ifndef Fireworks_Core_FWGeometryTableView_h
#define Fireworks_Core_FWGeometryTableView_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWGeometryTableView
// 
/**\class FWGeometryTableView FWGeometryTableView.h Fireworks/Core/interface/FWGeometryTableView.h

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  
//         Created:  Wed Jan  4 00:05:38 CET 2012
// $Id: FWGeometryTableView.h,v 1.1.2.4 2012/01/07 04:26:37 amraktad Exp $
//

#include "Fireworks/Core/interface/FWGeometryTableViewBase.h"
class FWGeometryTableManagerBase;
class FWGeometryTableManager;
class FWGUIValidatingTextEntry;
class FWGeoMaterialValidator;
class FWEveDetectorGeo;

class FWGeometryTableView : public FWGeometryTableViewBase
{
public:
   FWGeometryTableView(TEveWindowSlot* iParent, FWColorManager* colMng);
   virtual ~FWGeometryTableView();
   virtual void populateController(ViewerParameterGUI&) const;
   virtual  FWGeometryTableManagerBase*  getTableManager();

   void filterListCallback();
   void filterTextEntryCallback();
   void updateFilter(std::string&);

   bool getVolumeMode()      const { return m_mode.value() == kVolume; }
   std::string getFilter ()  const { return m_filter.value(); }
   int getAutoExpand()       const { return m_autoExpand.value(); }
   int getVisLevel()         const  {return m_visLevel.value(); }
   bool getIgnoreVisLevelWhenFilter() const  {return m_visLevelFilter.value(); }

   bool drawTopNode() const { return !m_disableTopNode.value(); }

   void cdNode(int);
   void cdTop();
   void cdUp();
   void setPath(int, std::string&);

   void printTable();

   int getTopNodeIdx() const { return m_topNodeIdx.value(); }

   virtual void setFrom(const FWConfiguration&);

   virtual void popupMenu(int, int);
   void chosenItem(int);

protected:
   // virtual void initGeometry(TGeoNode* iGeoTopNode, TObjArray* iVolumes);
   virtual TEveElement* getEveGeoElement() const;
   virtual void assertEveGeoElement();

private:
   FWGeometryTableView(const FWGeometryTableView&); // stop default
   const FWGeometryTableView& operator=(const FWGeometryTableView&); // stop default

   // ---------- member data --------------------------------
   FWGeometryTableManager *m_tableManager;

   FWGUIValidatingTextEntry* m_filterEntry;
   FWGeoMaterialValidator*   m_filterValidator;

   FWEveDetectorGeo*         m_eveTopNode;

#ifndef __CINT__
   FWLongParameter         m_topNodeIdx;  
   FWEnumParameter         m_mode;
   FWStringParameter       m_filter; 
   FWBoolParameter         m_disableTopNode;
   FWLongParameter         m_autoExpand;
   FWLongParameter         m_visLevel;
   FWBoolParameter         m_visLevelFilter; 
#endif  


   ClassDef(FWGeometryTableView, 0);
};


#endif