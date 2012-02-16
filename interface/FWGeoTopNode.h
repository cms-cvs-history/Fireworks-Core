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
// $Id: FWGeoTopNode.h,v 1.9.2.5 2012/01/06 23:19:39 amraktad Exp $
//

#ifndef __CINT__
#include "Fireworks/Core/interface/FWGeometryTableManagerBase.h"
#endif
#include "TEveElement.h"
#include "TAttBBox.h"
#include  <set>

class TGeoHMatrix;
class TGLPhysicalShape;
class TGLSelectRecord;

class FWGeometryTableView;
class FWOverlapTableView;
class TBuffer3D;
class TGeoNode;
class FWGeoTopNodeGLScene;


class FWGeoTopNode : public TEveElementList,
                     public TAttBBox
{
   friend class FWGeoTopNodeGL;

public:
   FWGeoTopNode(const char* n = "FWGeoTopNode", const char* t = "FWGeoTopNode"){}
   virtual ~FWGeoTopNode(){}

   virtual void Paint(Option_t* option="");
   FWGeoTopNodeGLScene    *fSceneJebo;

   virtual FWGeometryTableManagerBase* tableManager() { return 0; }
   std::set<TGLPhysicalShape*> fHted;
   std::set<TGLPhysicalShape*> fSted;

   int getFirstSelectedTableIndex();
   void selectPhysicalFromTable(int);
   void clearSelection() {fHted.clear(); fSted.clear();}

   void printSelected();

   virtual void UnSelected(); //{ ClearSet(fSted); }
   virtual void UnHighlighted();// { ClearSet(fHted); }
   virtual void popupMenu(int x, int y){}
protected:

   void ProcessSelection(TGLSelectRecord& rec, std::set<TGLPhysicalShape*>& sset, TGLPhysicalShape* id);

   void EraseFromSet(std::set<TGLPhysicalShape*>& sset, TGLPhysicalShape* id);
   void ClearSet(std::set<TGLPhysicalShape*>& sset);

   void SetStateOf(TGLPhysicalShape* id);


   void setupBuffMtx(TBuffer3D& buff, const TGeoHMatrix& mat);
#ifndef __CINT__
   void paintShape(FWGeometryTableManagerBase::NodeInfo& nodeInfo, Int_t idx,  const TGeoHMatrix& nm, bool volumeColor);
#endif

   virtual void ComputeBBox();
private:   
   FWGeoTopNode(const FWGeoTopNode&); // stop default
   const FWGeoTopNode& operator=(const FWGeoTopNode&); // stop default

   ClassDef(FWGeoTopNode, 0);
};



//==============================================================================
//==============================================================================
//==============================================================================
class FWEveDetectorGeo : public FWGeoTopNode
{
public:

   enum MenuOptions {
      kGeoSetTopNode,
      kGeoSetTopNodeCam,
      kGeoVisOn,
      kGeoVisOff,
      kGeoInspectMaterial,
      kGeoInspectShape,
      kGeoCamera
   };

   FWEveDetectorGeo(FWGeometryTableView* v); 
   virtual ~FWEveDetectorGeo() {}

   virtual void Paint(Option_t* option="");

   virtual TString     GetHighlightTooltip();

   virtual FWGeometryTableManagerBase* tableManager();
   virtual void popupMenu(int x, int y);
private:
#ifndef __CINT__
   void paintChildNodesRecurse(FWGeometryTableManagerBase::Entries_i pIt, Int_t idx,  const TGeoHMatrix& mtx);
#endif
   FWGeometryTableView       *m_browser;
   int m_maxLevel;
   bool m_filterOff;
   ClassDef(FWEveDetectorGeo, 0);

};
//==============================================================================
//==============================================================================


class FWEveOverlap : public FWGeoTopNode
{
public:

   enum MenuOptions {
      kOvlVisOff,
      kOvlVisOnOvl,
      kOvlVisOnAllMother,
      kOvlVisMother,
      kOvlSwitchVis,
      kOvlCamera,
      kOvlPrintOvl,
      kOvlPrintPath
   };

   FWEveOverlap(FWOverlapTableView* v);
   virtual ~FWEveOverlap(){}

   virtual void Paint(Option_t* option="");
   virtual TString     GetHighlightTooltip();

   virtual FWGeometryTableManagerBase* tableManager();
   virtual void popupMenu(int x, int y);
private:
   FWOverlapTableView       *m_browser;
};



#endif
