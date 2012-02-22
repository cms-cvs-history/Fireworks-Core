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
// $Id: FWGeoTopNode.h,v 1.9.2.8 2012/02/17 18:18:13 amraktad Exp $
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
   bool selectPhysicalFromTable(int);
   void clearSelection() {fHted.clear(); fSted.clear();}

   void printSelected();

   virtual void UnSelected();
   virtual void UnHighlighted();
   virtual void popupMenu(int x, int y){}

protected:
   static UInt_t phyID(int tableIdx);
   static int tableIdx(TGLPhysicalShape* ps);

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


#endif
