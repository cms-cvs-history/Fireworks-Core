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
// $Id: FWGeoTopNode.h,v 1.9.2.3 2012/01/04 02:39:45 amraktad Exp $
//

#include "Fireworks/Core/interface/FWGeometryTableManagerBase.h"
#include "TEveElement.h"

class TGeoHMatrix;


class FWGeometryTableView;
class FWOverlapTableView;
class TBuffer3D;
class TGeoNode;


class FWGeoTopNode : public TEveElementList
{
public:
   FWGeoTopNode(const char* n = "FWGeoTopNode", const char* t = "FWGeoTopNode"){ printf("qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqfff\n");}
   virtual ~FWGeoTopNode(){}

protected:
   void setupBuffMtx(TBuffer3D& buff, const TGeoHMatrix& mat);
   void paintShape(FWGeometryTableManagerBase::NodeInfo& nodeInfo, Int_t idx,  const TGeoHMatrix& nm);

private:   
   FWGeoTopNode(const FWGeoTopNode&); // stop default
   const FWGeoTopNode& operator=(const FWGeoTopNode&); // stop default

};
//==============================================================================
//==============================================================================
//==============================================================================
class FWEveDetectorGeo : public FWGeoTopNode
{
public:
   FWEveDetectorGeo(FWGeometryTableView* v):m_browser(v), m_maxLevel(0), m_filterOff(0){}   
                                             
   virtual ~FWEveDetectorGeo() {}

   virtual void Paint(Option_t* option="");

   virtual TString     GetHighlightTooltip();

private:
   void paintChildNodesRecurse(FWGeometryTableManagerBase::Entries_i pIt, Int_t idx,  const TGeoHMatrix& mtx);
   FWGeometryTableView       *m_browser;
   int m_maxLevel;
   bool m_filterOff;

};
//==============================================================================
//==============================================================================


class FWEveOverlap : public FWGeoTopNode
{
public:
   FWEveOverlap(FWOverlapTableView* v) :  m_browser(v) {}
   virtual ~FWEveOverlap(){}

   virtual void Paint(Option_t* option="");
   virtual TString     GetHighlightTooltip();

private:
   FWOverlapTableView       *m_browser;
};



#endif
