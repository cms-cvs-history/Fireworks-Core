#ifndef Fireworks_Core_FWGeoTopNode_h
#define Fireworks_Core_FWGeoTopNode_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWGeoTopNode
// 
/**\class FWGeoTopNode FWGeoTopNode.h Fireworks/Core/interface/FWGeoTopNode.h

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Matevz Tadel, Alja Mrak Tadel
//         Created:  Thu Jun 23 01:25:00 CEST 2011
// $Id: FWGeoTopNode.h,v 1.9.2.1 2011/12/07 22:39:58 amraktad Exp $
//

#include "Fireworks/Core/interface/FWGeometryTableManager.h"
#include "TEveElement.h"

class TGeoHMatrix;

class FWGeometryTableManager;
class FWGeometryTableView;
class TBuffer3D;
class TGeoNode;


class FWGeoTopNode : public TEveElementList
{
public:
   FWGeoTopNode(FWGeometryTableView*);
   virtual ~FWGeoTopNode();
   virtual void Paint(Option_t* option="");

   virtual TString     GetHighlightTooltip() ;
private:
   FWGeoTopNode(const FWGeoTopNode&); // stop default
   const FWGeoTopNode& operator=(const FWGeoTopNode&); // stop default


   void setupBuffMtx(TBuffer3D& buff, const TGeoHMatrix& mat);

   void PaintOverlaps();
   void paintChildNodesRecurse(FWGeometryTableManager::Entries_i pIt, Int_t idx,  const TGeoHMatrix& mtx);
   void  paintShape(FWGeometryTableManager::NodeInfo& nodeInfo, Int_t idx,  const TGeoHMatrix& nm);
   FWGeometryTableView       *m_browser;

   // cached
   FWGeometryTableManager::Entries_v* m_entries;
   int m_maxLevel;
   bool m_filterOff;
};


#endif
