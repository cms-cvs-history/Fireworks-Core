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
// $Id: FWGeoTopNode.cc,v 1.19.2.3 2011/12/19 07:21:28 amraktad Exp $
//

// system include files

// user include files

#include "TEveTrans.h"
#include "TEveManager.h"
#include "TEveUtil.h"


#include "TROOT.h"
#include "TBuffer3D.h"
#include "TBuffer3DTypes.h"
#include "TVirtualViewer3D.h"
#include "TColor.h"
#include "TGLScenePad.h"

#include "TGeoShape.h"
#include "TGeoVolume.h"
#include "TGeoNode.h"
#include "TGeoShapeAssembly.h"
#include "TGeoCompositeShape.h"
#include "TGeoBoolNode.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TVirtualGeoPainter.h"

#include "Fireworks/Core/interface/FWGeoTopNode.h"
#include "Fireworks/Core/interface/FWGeometryTableView.h"
#include "Fireworks/Core/interface/FWGeometryTableManager.h"
#include "Fireworks/Core/interface/FWGeometryTableViewManager.h"
#include "Fireworks/Core/interface/FWViewType.h"

FWGeoTopNode::FWGeoTopNode(FWGeometryTableView* t):
   m_browser(t),
   m_maxLevel(0),
   m_filterOff(0)
{
   m_entries = &(m_browser->getTableManager()->refEntries());
}

FWGeoTopNode::~FWGeoTopNode()
{
}

TString  FWGeoTopNode::GetHighlightTooltip()
{
   int idx = m_browser->getTableManager()->m_highlightIdx;
   if (idx > 0)
   {
      if (idx < (int)m_entries->size()) {
         FWGeometryTableManager::NodeInfo& data = m_entries->at(idx);
         return data.name();

      }
   }
   return "error";
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

//______________________________________________________________________________
void FWGeoTopNode::Paint(Option_t* opt)
{
   TEveGeoManagerHolder gmgr( FWGeometryTableViewManager::getGeoMangeur());

   m_maxLevel = m_browser->getVisLevel() + m_browser->getTableManager()->getLevelOffset() -1;
   m_filterOff = m_browser->getFilter().empty();

   if ( m_browser->typeId() == FWViewType::kGeometryTable)
   {
      Int_t topIdx = m_browser->getTopNodeIdx();
      FWGeometryTableManager::Entries_i sit = m_entries->begin(); 

      TGeoHMatrix mtx;
      if (topIdx >= 0)
      {
         std::advance(sit, topIdx);
         m_browser->getTableManager()->getNodeMatrix(*sit, mtx);

         // paint this node
         if ( m_filterOff == false)
            m_browser->getTableManager()->assertNodeFilterCache(*sit);

         if (m_browser->drawTopNode() && m_browser->getTableManager()->getVisibility(*sit))
            paintShape(*sit,  topIdx,mtx);
      }
      if (m_entries->size() > 0)
      {
         if ( m_browser->getTableManager()->getVisibilityChld(*sit) && sit->testBit(FWGeometryTableManager::kExpanded) )
            paintChildNodesRecurse( sit, topIdx, mtx);
      }
   }
   else
   {
      PaintOverlaps();
   }
}

//______________________________________________________________________________
void FWGeoTopNode::PaintOverlaps()
{
   int N = m_entries->size();
   int parentIdx = 0;
   bool visChld = false;
   for (int i = 1; i < N; ++i)
   {
      if (m_entries->at(i).m_parent == 0)
      {
         if (visChld && m_browser->getTableManager()->getVisibility(m_entries->at(i)) )
            paintShape(m_entries->at(i), i, *(m_entries->at(i).m_node->GetMatrix()));
         parentIdx = i;
         visChld = m_browser->getTableManager()->getVisibilityChld(m_entries->at(i));
      }
      else if (visChld && m_browser->getTableManager()->getVisibility(m_entries->at(i)) )
      {
         TGeoHMatrix nm = *m_entries->at(parentIdx).m_node->GetMatrix();
         nm.Multiply(m_entries->at(i).m_node->GetMatrix());
         paintShape(m_entries->at(i), i , nm);
      }
   }

}

// ______________________________________________________________________
void FWGeoTopNode::paintChildNodesRecurse (FWGeometryTableManager::Entries_i pIt, Int_t cnt, const TGeoHMatrix& parentMtx)
{ 
   TGeoNode* parentNode =  pIt->m_node;
   int nD = parentNode->GetNdaughters();

   int dOff=0;

   pIt++;
   int pcnt = cnt+1;

   FWGeometryTableManager::Entries_i it;
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
            paintShape(*it, cnt , nm);

         if  ( m_browser->getTableManager()->getVisibilityChld(*it) && ( it->m_level < m_maxLevel  || it->testBit(FWGeometryTableManager::kExpanded) )) {
            paintChildNodesRecurse(it,cnt , nm);
         }

      }
      else
      {
         m_browser->getTableManager()->assertNodeFilterCache(*it);
         if ( m_browser->getTableManager()->getVisibility(*it))
            paintShape(*it,cnt , nm);

         if ( m_browser->getTableManager()->getVisibilityChld(*it) && ( it->m_level < m_maxLevel || m_browser->getIgnoreVisLevelWhenFilter() ))
         {
            paintChildNodesRecurse(it,cnt , nm);
         }
      }


      FWGeometryTableManager::getNNodesTotal(parentNode->GetDaughter(n), dOff);  
   }
}
   /*
   // TEST ITERATON
     int cnt = 0;
     for ( FWGeometryTableManager::Entries_i sit = m_entries->begin(); sit  != m_entries->end(); ++sit, ++cnt)
     {
     if ( &(*sit) == (&data))
     {
     //       printf("%d \n", cnt);
     break;
     }
     }

     if (cnt != idx) printf("ERRROT IDX %d %d \n", cnt, idx);
   */

// ______________________________________________________________________
void FWGeoTopNode::paintShape(FWGeometryTableManager::NodeInfo& data,  Int_t idx, const TGeoHMatrix& nm)
{ 
   static const TEveException eh("FWGeoTopNode::paintShape ");

 
   UInt_t phyID = UInt_t(idx+1);

   TGeoShape* shape = data.m_node->GetVolume()->GetShape();
   TGeoCompositeShape* compositeShape = dynamic_cast<TGeoCompositeShape*>(shape);
   if (compositeShape)
   {
      Double_t halfLengths[3] = { compositeShape->GetDX(), compositeShape->GetDY(), compositeShape->GetDZ() };

      TBuffer3D buff(TBuffer3DTypes::kComposite);
      buff.fID           = data.m_node->GetVolume();
      buff.fColor        =  m_browser->getVolumeMode() ? data.m_node->GetVolume()->GetLineColor(): data.m_color;
      buff.fTransparency = data.m_node->GetVolume()->GetTransparency(); 

      nm.GetHomogenousMatrix(buff.fLocalMaster);  
      buff.fLocalFrame   = kTRUE; // Always enforce local frame (no geo manager).
      buff.SetAABoundingBox(compositeShape->GetOrigin(), halfLengths);
      buff.SetSectionsValid(TBuffer3D::kCore|TBuffer3D::kBoundingBox);

      Bool_t paintComponents = kTRUE;

      // Start a composite shape, identified by this buffer
      if (TBuffer3D::GetCSLevel() == 0) {
         TGLScenePad* sPad = dynamic_cast<TGLScenePad*>( gPad->GetViewer3D());
         paintComponents = sPad->OpenCompositeWithPhyID(phyID, buff);
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
   }
   else
   {
      TBuffer3D& buff = (TBuffer3D&) shape->GetBuffer3D (TBuffer3D::kCore, kFALSE);
      setupBuffMtx(buff, nm);
      buff.fID           = data.m_node->GetVolume();
      buff.fColor        =  m_browser->getVolumeMode() ? data.m_node->GetVolume()->GetLineColor(): data.m_color;
      buff.fTransparency =  data.m_node->GetVolume()->GetTransparency();

      nm.GetHomogenousMatrix(buff.fLocalMaster);
      buff.fLocalFrame   = kTRUE; // Always enforce local frame (no geo manager).

      Int_t sections = TBuffer3D::kBoundingBox | TBuffer3D::kShapeSpecific;
      shape->GetBuffer3D(sections, kTRUE);

      Int_t reqSec = gPad->GetViewer3D()->AddObject(phyID, buff);

      if (reqSec != TBuffer3D::kNone) {
         // This shouldn't happen, but I suspect it does sometimes.
         if (reqSec & TBuffer3D::kCore)
            Warning(eh, "Core section required again for shape='%s'. This shouldn't happen.", GetName());
         shape->GetBuffer3D(reqSec, kTRUE);
         reqSec = gPad->GetViewer3D()->AddObject(phyID, buff);
      }

      if (reqSec != TBuffer3D::kNone)  
         Warning(eh, "Extra section required: reqSec=%d, shape=%s.", reqSec, GetName());
   }
}
