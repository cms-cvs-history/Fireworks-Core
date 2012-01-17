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
// $Id: FWGeoTopNode.cc,v 1.19.2.8 2012/01/11 01:12:53 amraktad Exp $
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
#include "Fireworks/Core/src/FWGeometryTableView.h"
#include "Fireworks/Core/src/FWOverlapTableView.h"
#include "Fireworks/Core/src/FWGeometryTableManager.h"
#include "Fireworks/Core/src/FWOverlapTableManager.h"
#include "Fireworks/Core/interface/FWGeometryTableViewManager.h"
#include "Fireworks/Core/interface/FWViewType.h"


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
void FWGeoTopNode::paintShape(FWGeometryTableManagerBase::NodeInfo& data,  Int_t idx, const TGeoHMatrix& nm, bool volumeColor)
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
      buff.fColor        = volumeColor ? data.m_node->GetVolume()->GetLineColor() : data.m_color ;
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
      buff.fColor        = volumeColor ? data.m_node->GetVolume()->GetLineColor() : data.m_color ;
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

//==============================================================================
//==============================================================================
//==============================================================================

void FWEveDetectorGeo::Paint(Option_t* opt)
{
   // printf("PAINPAINTPAINTPAINTPAINTPAINTPAINTPAINTPAINTPAINTT \n");
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

      if (m_browser->drawTopNode() && m_browser->getTableManager()->getVisibility(*sit))
         paintShape(*sit,  topIdx,mtx, m_browser->getVolumeMode() );
   }

   if ( m_browser->getTableManager()->getVisibilityChld(*sit) && sit->testBit(FWGeometryTableManagerBase::kExpanded) )
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
   int idx = m_browser->getTableManager()->m_highlightIdx;
   if (idx > 0)
   {
      FWGeometryTableManagerBase::NodeInfo& data = m_browser->getTableManager()->refEntries().at(idx);
      return data.name();
   }
   return "error";
}

//==============================================================================
//==============================================================================
//==============================================================================

void FWEveOverlap::Paint(Option_t*)
{
   // printf("PAINPAINTPAINTPAINTPAINTPAINTPAINTPAINTPAINTPAINTT \n");
   TEveGeoManagerHolder gmgr( FWGeometryTableViewManager::getGeoMangeur());

   FWGeometryTableManagerBase::Entries_i parentIt = m_browser->getTableManager()->refEntries().begin();
   bool visChld = false;
   int cnt = 0;

   for (FWGeometryTableManagerBase::Entries_i it = parentIt;
        it != m_browser->getTableManager()->refEntries().end(); ++it, ++cnt)
   {
      if (it->m_parent == 0)
      {
        
        if ((m_browser->m_rnrOverlap.value() && (strncmp(it->m_node->GetName(), "Ovlp", 3) == 0)) ||
            (m_browser->m_rnrExtrusion.value() && (strncmp(it->m_node->GetName(), "Extr", 3) == 0)) )
        {          
         if (it->testBit(FWGeometryTableManagerBase::kVisNodeSelf) )
            paintShape(*it, cnt, *(it->m_node->GetMatrix()), false);
          
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

TString  FWEveOverlap::GetHighlightTooltip()
{
  if ( m_browser->getTableManager()->m_highlightIdx < 0) 
  {
    return Form("TopNode ");
  }
  
  FWGeometryTableManagerBase::NodeInfo& data = m_browser->getTableManager()->refEntries().at(m_browser->getTableManager()->m_highlightIdx);
  if (data.m_parent == 0)
  {
    return data.name();
  }
  else {

    TString pname =  m_browser->getTableManager()->refEntries().at(data.m_parent).name();
    int sc =  strlen(pname.Data());
    while (sc > 0) {
      if (pname[sc] == ' ') break;
      sc--;
    }
    pname.Resize(sc);

    return Form("%s, %s", data.name(), pname.Data());
  }

}
