// -*- C++ -*-
//
// Package:     Core
// Class  :     FWGeoTopNode
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Matevz Tadel, Alja Mrak Tadel  
//         Created:  Thu Jun 23 01:24:51 CEST 2011
// $Id: FWGeoTopNode.cc,v 1.19.2.14 2012/01/20 02:44:12 amraktad Exp $
//

// system include files

// user include files

#include "TEveTrans.h"
#include "TEveViewer.h"
#include "TEveManager.h"
#include "TEveUtil.h"


#include "TROOT.h"
#include "TBuffer3D.h"
#include "TBuffer3DTypes.h"
#include "TVirtualViewer3D.h"
#include "TColor.h"
#include "TGLScenePad.h"
#include "TGLPhysicalShape.h"
#include "TGLSelectRecord.h"

#include "TGeoShape.h"
#include "TGeoVolume.h"
#include "TGeoNode.h"
#include "TGeoShapeAssembly.h"
#include "TGeoCompositeShape.h"
#include "TGeoBoolNode.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoOverlap.h"
#include "TVirtualGeoPainter.h"
#include "Fireworks/Core/src/FWPopupMenu.cc"

#include "Fireworks/Core/interface/FWGeoTopNode.h"
#include "Fireworks/Core/src/FWGeoTopNodeScene.h"
#include "Fireworks/Core/src/FWGeometryTableView.h"
#include "Fireworks/Core/src/FWOverlapTableView.h"
#include "Fireworks/Core/src/FWGeometryTableManager.h"
#include "Fireworks/Core/src/FWOverlapTableManager.h"
#include "Fireworks/Core/interface/FWGeometryTableViewManager.h"
#include "Fireworks/Core/interface/FWViewType.h"
#include "Fireworks/Core/interface/fwLog.h"

namespace{
   UInt_t phyID(int tableIdx) { return UInt_t(tableIdx + 2);}
   //  int tableIdx(UInt_t phyID) { return phyID - 2; }
   int tableIdx(TGLPhysicalShape* ps) { return ps->ID() - 2; }
}

//______________________________________________________________________________
void FWGeoTopNode::EraseFromSet(std::set<TGLPhysicalShape*>& sset, TGLPhysicalShape* id)
{
   sset.erase(id);
   SetStateOf(id);
}

//______________________________________________________________________________
void FWGeoTopNode::ClearSet(std::set<TGLPhysicalShape*>& sset)
{
   while (!sset.empty())
   {
      TGLPhysicalShape *id = *sset.begin();
      sset.erase(id);
      SetStateOf(id);
   }
}
//______________________________________________________________________________
void FWGeoTopNode::SetStateOf(TGLPhysicalShape* id)
{
   if (fSted.find(id) != fSted.end())
      id->Select(1);
   else if (fHted.find(id) != fHted.end())
      id->Select(3);
   else
      id->Select(0);

}

//______________________________________________________________________________
void FWGeoTopNode::ProcessSelection(TGLSelectRecord& rec, std::set<TGLPhysicalShape*>& sset, TGLPhysicalShape* id)
{
   printf("FWGeoTopNode::ProcessSelection ===============================\n");

   fSceneJebo->BeginUpdate();

   if (sset.empty())
   {
      if (id)
      {
         sset.insert(id);
         rec.SetSecSelResult(TGLSelectRecord::kEnteringSelection);
      }  
   }
   else
   {
      if (id)
      {
         if (rec.GetMultiple())
         {
            if (sset.find(id) == sset.end())
            {
               sset.insert(id);
               rec.SetSecSelResult(TGLSelectRecord::kModifyingInternalSelection);
            }
            else
            {
               EraseFromSet(sset, id);
               if (sset.empty())
                  rec.SetSecSelResult(TGLSelectRecord::kLeavingSelection);
               else
                  rec.SetSecSelResult(TGLSelectRecord::kModifyingInternalSelection);
            }
         }
         else
         {
            if (sset.size() != 1 || sset.find(id) == sset.end())
            {
               ClearSet(sset);
               sset.insert(id);
               rec.SetSecSelResult(TGLSelectRecord::kModifyingInternalSelection);
            }
         }
      }
      else
      {
         if (!rec.GetMultiple())
         {
            ClearSet(sset);
            rec.SetSecSelResult(TGLSelectRecord::kLeavingSelection);
         }
      }
   }

   if (id)
   {
      SetStateOf(id);
   }

   if (rec.GetSecSelResult() != TGLSelectRecord::kNone)
   {
      fSceneJebo->EndUpdate(kTRUE, kFALSE, kTRUE);
      gEve->Redraw3D();


      for (FWGeometryTableManagerBase::Entries_i i = tableManager()->refEntries().begin(); i != tableManager()->refEntries().end(); ++i)
      {
         i->resetBit(FWGeometryTableManagerBase::kHighlighted);
         i->resetBit(FWGeometryTableManagerBase::kSelected);
      }

      for (std::set<TGLPhysicalShape*>::iterator it = fHted.begin(); it != fHted.end(); ++it)
      {
         //  printf("set HIGH \n");
         tableManager()->refEntries().at(tableIdx(*it)).setBit(FWGeometryTableManagerBase::kHighlighted);
      }
      for (std::set<TGLPhysicalShape*>::iterator it = fSted.begin(); it != fSted.end(); ++it)
      {
         //  printf("SET TABLE selected BIT \n");
         tableManager()->refEntries().at(tableIdx(*it)).setBit(FWGeometryTableManagerBase::kSelected);
      }

      tableManager()->dataChanged();
   }
   else
   {
      fSceneJebo->EndUpdate(kFALSE, kFALSE, kFALSE);
   }
}

//______________________________________________________________________________
void FWGeoTopNode::selectPhysicalFromTable( int tableIndex)
{
   //   printf("FWGeoTopNode::selectPhysicalFromTable \n");

   ClearSet(fSted);

   for (FWGeometryTableManagerBase::Entries_i i = tableManager()->refEntries().begin(); i != tableManager()->refEntries().end(); ++i)
   {
      i->resetBit(FWGeometryTableManagerBase::kHighlighted);
      i->resetBit(FWGeometryTableManagerBase::kSelected);
   }
   tableManager()->refEntries().at(tableIndex).setBit(FWGeometryTableManagerBase::kSelected);
   tableManager()->dataChanged(); 

   TGLPhysicalShape* ps = fSceneJebo->FindPhysical(phyID(tableIndex));
   if (ps) {
      fSceneJebo->BeginUpdate();
      fSted.insert(ps);
      ps->Select(1);
      fSceneJebo->EndUpdate(kTRUE, kFALSE, kTRUE);
      gEve->Redraw3D();
   }
   else if ( tableManager()->refEntries().at(tableIndex).testBit(FWGeometryTableManagerBase::kVisNodeSelf));
   {
      fwLog(fwlog::kInfo) << "Object not drawn in GL viewer. \n" ;
   }
}

//______________________________________________________________________________
void FWGeoTopNode::printSelected() 
{
   for (std::set<TGLPhysicalShape*>::iterator it = fSted.begin(); it != fSted.end(); ++it)
   {
      printf("FWGeoTopNode::printSelected %s \n",  tableManager()->refEntries().at(tableIdx(*it)).name() );
   }
}

//______________________________________________________________________________

int FWGeoTopNode::getFirstSelectedTableIndex() 
{
   // Note: if object would be rendered, this would return fSted.begin().

   if (fSted.size() <= 1)
   {
      int cnt = 0;
      for (FWGeometryTableManagerBase::Entries_i i = tableManager()->refEntries().begin(); i != tableManager()->refEntries().end(); ++i, ++cnt)
      {
         if (i->testBit(FWGeometryTableManagerBase::kSelected)) return cnt; 
      }
   }
   return -1;
}

//______________________________________________________________________________
void FWGeoTopNode::ComputeBBox()
{
   // Fill bounding-box information. Virtual from TAttBBox.

   BBoxZero(1.0f);
}

//______________________________________________________________________________
void FWGeoTopNode::setupBuffMtx(TBuffer3D& buff, const TGeoHMatrix& mat)
{
   const Double_t *r = mat.GetRotationMatrix();
   const Double_t *t = mat.GetTranslation();
   const Double_t *s = mat.GetScale();
   Double_t       *m = buff.fLocalMaster;
   m[0]  = r[0]*s[0]; m[1]  = r[1]*s[1]; m[2]  = r[2]*s[2]; m[3]  = 0;
   m[4]  = r[3]*s[0]; m[5]  = r[4]*s[1]; m[6]  = r[5]*s[2]; m[7]  = 0;
   m[8]  = r[6]*s[0]; m[9]  = r[7]*s[1]; m[10] = r[8]*s[2]; m[11] = 0;
   m[12] = t[0];      m[13] = t[1];      m[15] = t[2];      m[15] = 1;

   buff.fLocalFrame = kTRUE;
}

// ______________________________________________________________________
void FWGeoTopNode::paintShape(FWGeometryTableManagerBase::NodeInfo& data,  Int_t tableIndex, const TGeoHMatrix& nm, bool volumeColor)
{
   static const TEveException eh("FWGeoTopNode::paintShape ");
 
   //  printf("paint sahpe %s idx %d\n", data.m_node->GetVolume()->GetName(), tableIndex );

   TGeoShape* shape = data.m_node->GetVolume()->GetShape();
   
   TGeoCompositeShape* compositeShape = dynamic_cast<TGeoCompositeShape*>(shape);
   if (compositeShape)
   {
      // fSceneJebo->fNextCompositeID = phyID(tableIndex);

      Double_t halfLengths[3] = { compositeShape->GetDX(), compositeShape->GetDY(), compositeShape->GetDZ() };

      TBuffer3D buff(TBuffer3DTypes::kComposite);
      buff.fID           = data.m_node->GetVolume();
      buff.fColor        = volumeColor ? data.m_node->GetVolume()->GetLineColor() : data.m_color ;
      buff.fTransparency = data.m_node->GetVolume()->GetTransparency(); 

      nm.GetHomogenousMatrix(buff.fLocalMaster);  
      buff.fLocalFrame   = kTRUE; // Always enforce local frame (no geo manager).
      buff.SetAABoundingBox(compositeShape->GetOrigin(), halfLengths);
      buff.SetSectionsValid(TBuffer3D::kCore|TBuffer3D::kBoundingBox);

      Bool_t paintComponents = kTRUE;
      // Start a composite shape, identified by this buffer
      if (TBuffer3D::GetCSLevel() == 0) {
         paintComponents = fSceneJebo->OpenCompositeWithPhyID(phyID(tableIndex), buff);
      }

      TBuffer3D::IncCSLevel();
      
      // Paint the boolean node - will add more buffers to viewer
      TGeoHMatrix xxx;
      TGeoMatrix *gst = TGeoShape::GetTransform();
      TGeoShape::SetTransform(&xxx);

      if (paintComponents) compositeShape->GetBoolNode()->Paint("");
      TGeoShape::SetTransform(gst);
      // Close the composite shape
      if (TBuffer3D::DecCSLevel() == 0)
         gPad->GetViewer3D()->CloseComposite();


      //  fSceneJebo->fNextCompositeID = 0;
   }
   else
   {
      TBuffer3D& buff = (TBuffer3D&) shape->GetBuffer3D (TBuffer3D::kCore, kFALSE);
      setupBuffMtx(buff, nm);
      buff.fID           = data.m_node->GetVolume();
      buff.fColor        = volumeColor ? data.m_node->GetVolume()->GetLineColor() : data.m_color ;
      buff.fTransparency =  data.m_node->GetVolume()->GetTransparency();

      nm.GetHomogenousMatrix(buff.fLocalMaster);
      buff.fLocalFrame   = kTRUE; // Always enforce local frame (no geo manager).

      Int_t sections = TBuffer3D::kBoundingBox | TBuffer3D::kShapeSpecific;
      shape->GetBuffer3D(sections, kTRUE);

      Int_t reqSec = gPad->GetViewer3D()->AddObject(phyID(tableIndex), buff);

      if (reqSec != TBuffer3D::kNone) {
         // This shouldn't happen, but I suspect it does sometimes.
         if (reqSec & TBuffer3D::kCore)
            Warning(eh, "Core section required again for shape='%s'. This shouldn't happen.", GetName());
         shape->GetBuffer3D(reqSec, kTRUE);
         reqSec = gPad->GetViewer3D()->AddObject(phyID(tableIndex), buff);
      }

      if (reqSec != TBuffer3D::kNone)  
         Warning(eh, "Extra section required: reqSec=%d, shape=%s.", reqSec, GetName());
   }
}

// ______________________________________________________________________
void FWGeoTopNode::Paint(Option_t* opt)
{
   static const TEveException eh("TEveTopNodeJebo::Paint ");

   TBuffer3D buff(TBuffer3DTypes::kGeneric);

   // Section kCore
   buff.fID           = this;
   buff.fColor        = GetMainColor();
   buff.fTransparency = GetMainTransparency();
   if (HasMainTrans())  RefMainTrans().SetBuffer3D(buff);

   buff.SetSectionsValid(TBuffer3D::kCore);

   Int_t reqSections = gPad->GetViewer3D()->AddObject(1, buff);
   if (reqSections != TBuffer3D::kNone)
   {
      Warning(eh, "IsA='%s'. Viewer3D requires more sections (%d). Only direct-rendering supported.",
              ClassName(), reqSections);
   }
}

// ______________________________________________________________________
void FWGeoTopNode::UnSelected()
{
   ClearSet(fSted);
   for (FWGeometryTableManagerBase::Entries_i i = tableManager()->refEntries().begin(); i != tableManager()->refEntries().end(); ++i)
      i->resetBit(FWGeometryTableManagerBase::kSelected);
      
}
// ______________________________________________________________________
void FWGeoTopNode::UnHighlighted()
{
   ClearSet(fHted);
   for (FWGeometryTableManagerBase::Entries_i i = tableManager()->refEntries().begin(); i != tableManager()->refEntries().end(); ++i)
      i->resetBit(FWGeometryTableManagerBase::kHighlighted);
     
}   

//==============================================================================
//==============================================================================
//==============================================================================
FWEveDetectorGeo::FWEveDetectorGeo(FWGeometryTableView* v):
   m_browser(v), m_maxLevel(0), m_filterOff(0)
{
} 

FWGeometryTableManagerBase* FWEveDetectorGeo::tableManager()
{
   return m_browser->getTableManager();
}
//______________________________________________________________________________

void FWEveDetectorGeo::Paint(Option_t* opt)
{
   FWGeoTopNode::Paint();

   //   printf("PAINPAINTPAINTPAINTPAINTPAINTPAINTPAINTPAINTPAINTT  %d/%d \n",  m_browser->getTopNodeIdx(),  (int)m_browser->getTableManager()->refEntries().size());
   if (m_browser->getTableManager()->refEntries().empty()) return; 

   TEveGeoManagerHolder gmgr( FWGeometryTableViewManager::getGeoMangeur());

   m_maxLevel = m_browser->getVisLevel() + m_browser->getTableManager()->getLevelOffset() -1;
   m_filterOff = m_browser->getFilter().empty();

   Int_t topIdx = m_browser->getTopNodeIdx();
   FWGeometryTableManagerBase::Entries_i sit = m_browser->getTableManager()->refEntries().begin(); 

   TGeoHMatrix mtx;
   if (topIdx >= 0)
   {
      std::advance(sit, topIdx);
      m_browser->getTableManager()->getNodeMatrix(*sit, mtx);

      if (sit->testBit(FWGeometryTableManagerBase::kVisNodeSelf) && m_browser->getTableManager()->getVisibility(*sit))
         paintShape(*sit,  topIdx,mtx, m_browser->getVolumeMode() );
   }

   if ( m_browser->getTableManager()->getVisibilityChld(*sit))
      paintChildNodesRecurse( sit, topIdx, mtx);
}


// ______________________________________________________________________
void FWEveDetectorGeo::paintChildNodesRecurse (FWGeometryTableManagerBase::Entries_i pIt, Int_t cnt, const TGeoHMatrix& parentMtx)
{ 
   TGeoNode* parentNode =  pIt->m_node;
   int nD = parentNode->GetNdaughters();

   int dOff=0;

   pIt++;
   int pcnt = cnt+1;

   FWGeometryTableManagerBase::Entries_i it;
   for (int n = 0; n != nD; ++n)
   {
      it =  pIt;
      std::advance(it,n + dOff);
      cnt = pcnt + n+dOff;

      TGeoHMatrix nm = parentMtx;
      nm.Multiply(it->m_node->GetMatrix());

  
      if (m_filterOff)
      {
         if ( m_browser->getTableManager()->getVisibility(*it))
            paintShape(*it, cnt , nm, m_browser->getVolumeMode() );

         if  ( m_browser->getTableManager()->getVisibilityChld(*it) && ( it->m_level < m_maxLevel  || it->testBit(FWGeometryTableManagerBase::kExpanded) )) {
            paintChildNodesRecurse(it,cnt , nm);
         }

      }
      else
      {
         if ( m_browser->getTableManager()->getVisibility(*it))
            paintShape(*it,cnt , nm, m_browser->getVolumeMode()  );

         if ( m_browser->getTableManager()->getVisibilityChld(*it) && ( it->m_level < m_maxLevel || m_browser->getIgnoreVisLevelWhenFilter() ))
         {
            paintChildNodesRecurse(it,cnt , nm);
         }
      }


      FWGeometryTableManagerBase::getNNodesTotal(parentNode->GetDaughter(n), dOff);  
   }
}

//______________________________________________________________________________

TString  FWEveDetectorGeo::GetHighlightTooltip()
{
   std::set<TGLPhysicalShape*>::iterator it = fHted.begin();
   int idx = tableIdx(*it);
   if (idx > 0)
   {
      FWGeometryTableManagerBase::NodeInfo& data = m_browser->getTableManager()->refEntries().at(idx);
      return data.name();
   }
   return "error";
}



//______________________________________________________________________________

void FWEveDetectorGeo::popupMenu(int x, int y)
{  
   if (getFirstSelectedTableIndex() < 0)
   {
      if (fSted.empty()) fwLog(fwlog::kInfo) << "No menu -- no node/entry selected \n";
      return;
   }
   
   FWPopupMenu* nodePopup = new FWPopupMenu();
   nodePopup->AddEntry("Set As Top Node", kGeoSetTopNode);
   nodePopup->AddEntry("Set As Top Node And Camera Center", kGeoSetTopNodeCam);
   nodePopup->AddSeparator();
   nodePopup->AddEntry("Rnr Off For All Children", kGeoVisOff);
   nodePopup->AddEntry("Rnr On For All Children", kGeoVisOn);
   nodePopup->AddSeparator();
   nodePopup->AddEntry("Set Camera Center", kGeoCamera);
   nodePopup->AddSeparator();
   //   nodePopup->AddEntry("InspectMaterial", kGeoInspectMaterial); crashes !!!
   nodePopup->AddEntry("InspectShape", kGeoInspectShape);

   nodePopup->PlaceMenu(x, y,true,true);
   nodePopup->Connect("Activated(Int_t)",
                      "FWGeometryTableView",
                       m_browser,
                      "chosenItem(Int_t)");
}
//==============================================================================
//==============================================================================
//==============================================================================
FWEveOverlap::FWEveOverlap(FWOverlapTableView* v):
   m_browser(v)
{
} 
FWGeometryTableManagerBase* FWEveOverlap::tableManager()
{
   return m_browser->getTableManager();
}
//______________________________________________________________________________

void FWEveOverlap::Paint(Option_t*)
{
   FWGeoTopNode::Paint();

   TEveGeoManagerHolder gmgr( FWGeometryTableViewManager::getGeoMangeur());

   FWGeometryTableManagerBase::Entries_i parentIt = m_browser->getTableManager()->refEntries().begin();
   bool visChld = false;
   int cnt = 0;

   for (FWGeometryTableManagerBase::Entries_i it = parentIt;
        it != m_browser->getTableManager()->refEntries().end(); ++it, ++cnt)
   {
      if (it->m_parent == -1)
      { 
         if (it->testBit(FWGeometryTableManagerBase::kVisNodeSelf) )
            paintShape(*it, cnt, *(it->m_node->GetMatrix()), false);
      }
      else if (it->m_parent == 0)
      {      
         if ((m_browser->m_rnrOverlap.value() && it->testBit(FWOverlapTableManager::kOverlap)) ||
             (m_browser->m_rnrExtrusion.value() && !it->testBit(FWOverlapTableManager::kOverlap)) )
         {          
            if (it->testBit(FWGeometryTableManagerBase::kVisNodeSelf) )
            {
               paintShape(*it, cnt, *(it->m_node->GetMatrix()), false);
            }
            visChld = it->testBit(FWGeometryTableManagerBase::kVisNodeChld);
         }
         else {
            visChld = false;
         }

         parentIt = it;
      }
      else if (visChld && it->testBit(FWGeometryTableManagerBase::kVisNodeSelf) )
      {  
         TGeoHMatrix nm = *(parentIt->m_node->GetMatrix());
         nm.Multiply(it->m_node->GetMatrix());
         paintShape(*it, cnt , nm, false);
      }
   }
}
//______________________________________________________________________________

void FWEveOverlap::popupMenu(int x, int y)
{
 
   if (getFirstSelectedTableIndex() < 0)
   {
      if (fSted.empty()) fwLog(fwlog::kInfo) << "No menu -- no node/entry selected \n";
      return;
   }
   FWPopupMenu* nodePopup = new FWPopupMenu();

   
   nodePopup->AddEntry("Switch Visibility Self ", kOvlSwitchVis);

   nodePopup->AddEntry("Swithc Visibility Mother ", kOvlVisMother);

   nodePopup->AddSeparator();
   nodePopup->AddEntry("Print Path ", kOvlPrintPath);
   nodePopup->AddEntry("Print Overlap Info", kOvlPrintOvl);
   nodePopup->AddSeparator();
   nodePopup->AddEntry("Set Camera Center", kOvlCamera);
   nodePopup->AddSeparator();
   nodePopup->AddEntry("Rnr Off Everything", kOvlVisOff);
   nodePopup->AddEntry("Rnr On Overlaps, Extrusions", kOvlVisOnOvl);
   nodePopup->AddEntry("Rnr On Mother Volumes", kOvlVisOnAllMother);


   nodePopup->PlaceMenu(x, y,true,true); 
   nodePopup->Connect("Activated(Int_t)",
                      "FWOverlapTableView",
                      m_browser,
                      "chosenItem(Int_t)");
}

//______________________________________________________________________________

TString  FWEveOverlap::GetHighlightTooltip()
{
   //   printf("highlight tooltio \n");

   std::set<TGLPhysicalShape*>::iterator it = fHted.begin();
   int idx = tableIdx(*it);
   if ( idx < 0) 
   {
      return Form("TopNode ");
   }
  
   FWGeometryTableManagerBase::NodeInfo& data = m_browser->getTableManager()->refEntries().at(idx);
   if (data.m_parent <= 0)
   {
      return data.name();
   }
   else {
      TString pname =  m_browser->getTableManager()->refEntries().at(data.m_parent).name();
      TString text;
      const TGeoOverlap* ovl =  ((FWOverlapTableManager*)m_browser->getTableManager())->referenceOverlap(idx);
      text =  data.name();
      text += Form("\noverlap = %g cm", ovl->GetOverlap());

      if (ovl->IsOverlap()) 
      {
         int nidx = (idx == (data.m_parent + 1) ) ? (data.m_parent + 2) : (data.m_parent + 1);
         text += Form("\nsister = %s", m_browser->getTableManager()->refEntries().at(nidx).name() );
      }
      else
      {  
         text += Form("\nmother = %s",m_browser->getTableManager()->refEntries().at(data.m_parent).m_node->GetVolume()->GetName());
      }
      return text.Data();
   }
}
