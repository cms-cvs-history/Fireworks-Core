// -*- C++ -*-
//
// Package:     Core
// Class  :     FWOverlapTableManager
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  
//         Created:  Wed Jan  4 20:31:32 CET 2012
// $Id: FWOverlapTableManager.cc,v 1.1.2.13 2012/01/20 02:44:12 amraktad Exp $
//

// system include files

// user include files
#include "Fireworks/Core/src/FWOverlapTableManager.h"
#include "Fireworks/Core/src/FWOverlapTableView.h"
#include "Fireworks/Core/interface/FWGeometryTableViewManager.h"
#include "Fireworks/Core/interface/fwLog.h"

#include "TEvePointSet.h"
#include "TGeoVolume.h"
#include "TGeoMatrix.h"
#include "TGeoShape.h"
#include "TGeoBBox.h"
#include "TGeoMatrix.h"
#include "TEveUtil.h"
#include "TObjString.h"
#include "TGeoNode.h"
#include "TGeoOverlap.h"
#include "TGeoManager.h"
#include "TPolyMarker3D.h"

FWOverlapTableManager::FWOverlapTableManager(FWOverlapTableView* v ):
   FWGeometryTableManagerBase(),
   m_browser(v)
{
}

FWOverlapTableManager::~FWOverlapTableManager()
{
}



std::vector<std::string> FWOverlapTableManager::getTitles() const 
{
   std::vector<std::string> returnValue;
   returnValue.reserve(numberOfColumns());

   returnValue.push_back("Name");
   returnValue.push_back("Color");
   returnValue.push_back("RnrSelf");
   returnValue.push_back("RnrChildren");
   returnValue.push_back("Overlap");
   returnValue.push_back("RnrMarker");
   return returnValue;
}
//---------------------------------------------------------------------------------
void FWOverlapTableManager::importOverlaps(std::string iPath, double iPrecision)
{

   printf("\n Import START eps=%f path %s \n", iPrecision, iPath.c_str());

   TEveGeoManagerHolder gmgr( FWGeometryTableViewManager::getGeoMangeur());
   m_levelOffset = 0;
   
   if (!iPath.empty()) {
      size_t ps = iPath.size();
      if (ps > 1 && iPath[ps-1] == '/') 
         iPath = iPath.substr(0, ps-1);

    
      if (gGeoManager->GetCurrentNavigator()->CheckPath(iPath.c_str()) == 0)
      {
         fwLog(fwlog::kError) << "IPath  [" << iPath <<"] not valid." << std::endl;
         return;
      }
      gGeoManager->cd(iPath.c_str());
   }
   else
   {
      gGeoManager->cd("");
   }
  
   gGeoManager->GetCurrentNode()->CheckOverlaps(iPrecision);
   TObjArray* lOverlaps = gGeoManager->GetListOfOverlaps();
  
   TGeoNode* top_node = gGeoManager->GetCurrentNode();

   m_entries.clear();
   m_browser->m_markerVertices.clear();
   m_browser->m_markerIndices.clear();

   NodeInfo topNodeInfo;
   topNodeInfo.m_node   = top_node; 
   topNodeInfo.m_level  = 0;
   topNodeInfo.m_parent = -1;
   topNodeInfo.resetBit(kVisNodeSelf);
   m_entries.push_back( topNodeInfo);

   std::vector<float> pnts;
   int ovlCnt = 1;
   {
      TIter next_ovl(lOverlaps);
      TGeoOverlap *ovl;

      TGeoIterator top_git   (gGeoManager->GetCurrentVolume());
      TGeoNode*    top_gnode (top_git.Next());
      while((ovl = (TGeoOverlap*)next_ovl())) 
      {
         if (1) printf(Form("[%d/%d] Scanning for Extp=%d, Ovlp=%d: vol1=%-12s vol2=%-12s \n",      
                            ovlCnt++,lOverlaps->GetEntries(),
                            ovl->IsExtrusion(),  ovl->IsOverlap(),
                            ovl->GetFirstVolume()->GetName(),
                            ovl->GetSecondVolume()->GetName()));

      reiterate:
         Overlap fwo(ovl);
         TGeoIterator git(top_git);
         TGeoNode* gnode = top_gnode;
         do {
            TGeoVolume* gvol = gnode->GetVolume();
            if(gvol == ovl->GetFirstVolume())
            {
               top_git = git; top_gnode = gnode;
               fwo.n1 = gnode; fwo.v1 = gvol;  fwo.l1 = git.GetLevel();
               git.GetPath(fwo.iteratorPath1);
               if( ovl->IsOverlap())
                  git.Skip();

               while((gnode = git.Next()) != 0)
               {
                  gvol = gnode->GetVolume();
                  if(gvol == ovl->GetSecondVolume()) {
                     fwo.n2 = gnode; fwo.v2 = gvol; fwo.l2 = git.GetLevel();
                     git.GetPath(fwo.iteratorPath2);
                     if(ovl->IsExtrusion()) 
                     {
                        fwo.motherl = fwo.l1;
                        fwo.mothern = fwo.n1;
                        fwo.motherv = fwo.v1;
                     }
                     else 
                     {
                        fwo.motherl = TMath::Min(fwo.l1, fwo.l2);
                        do {
                           --fwo.motherl;
                           if (fwo.motherl > 0 ) 
                           {
                              fwo.mothern = git.GetNode(fwo.motherl);
                           }
                           else
                           {
                              fwo.mothern = top_node;
                              break;
                           }
                        } while(fwo.mothern->GetVolume()->IsAssembly());
                        fwo.motherv = fwo.mothern->GetVolume();
                     }

                     TGeoNode *node = git.GetTopVolume()->GetNode(git.GetIndex(1));
                     fwo.motherm.Multiply(node->GetMatrix());
                     for (Int_t i=2; i<=fwo.motherl; i++) {
                        node = node->GetDaughter(git.GetIndex(i));
                        fwo.motherm.Multiply(node->GetMatrix());
                     }

                     addOverlapEntry(fwo);
                     break;
                  }
               }
               break; // found first
            }
         } while((gnode = git.Next()) != 0);

         if(fwo.v2 == 0) {
            printf( "  Could not find both volumes, resetting geo-iterator.");
            top_git.Reset();
            top_gnode = top_git.Next();
            goto reiterate;
         }

      }
   }

   printf("Import END eps=%f \n", iPrecision);
}

//______________________________________________________________________________

void FWOverlapTableManager::addOverlapEntry(Overlap& fwo)
{                      
      TGeoNodeMatrix* mother_node = new TGeoNodeMatrix((const TGeoVolume*) fwo.motherv, new TGeoHMatrix( fwo.motherm));
      TString mname;
      if (fwo.ovl->IsOverlap()) 
      {
         mname  = "Ovl: ";
         if (fwo.motherl > 0)
         {
            TObjArray* x = fwo.iteratorPath1.Tokenize("/");
            for(int i = 0; i<fwo.motherl; ++i)
               mname += ((TObjString*)x->At(i))->GetString();
         }
         else 
            mname += "TOPNODE";
      }
      else
      {
         mname  = "Extr: " + fwo.iteratorPath1;
      }
      mother_node->SetNameTitle(mname.Data(), mname.Data());
      m_entries.push_back(NodeInfo(mother_node, 0, fwo.motherv->GetLineColor(), fwo.motherl+1, kVisNodeChld |  kFlag1));
      int parentIdx = m_entries.size() -1;

      if (fwo.ovl->IsOverlap()) {
         TGeoNodeMatrix* gnode1 = new TGeoNodeMatrix(fwo.v1, fwo.ovl->GetFirstMatrix());
         gnode1->SetNameTitle(fwo.iteratorPath1.Data(), fwo.iteratorPath1.Data());
         m_entries.push_back(NodeInfo(gnode1, parentIdx, fwo.v1->GetLineColor(), fwo.l1+1));
      }


      TGeoNodeMatrix* gnode2 = new TGeoNodeMatrix(fwo.v2, fwo.ovl->GetSecondMatrix());
      gnode2->SetNameTitle(fwo.iteratorPath1.Data(), fwo.iteratorPath1.Data());
      m_entries.push_back(NodeInfo(gnode2, parentIdx, fwo.v2->GetLineColor(), fwo.l2+1));                   


      TPolyMarker3D* pm = fwo.ovl->GetPolyMarker();
      for (int j=0; j<pm->GetN(); ++j )
      {
         double pl[3];
         double pg[3];
         pm->GetPoint(j, pl[0], pl[1], pl[2]);
         fwo.motherm.LocalToMaster(pl, pg);
         m_browser->m_markerIndices.push_back(parentIdx);
         m_browser->m_markerVertices.push_back( pg[0]);
         m_browser->m_markerVertices.push_back( pg[1]);
         m_browser->m_markerVertices.push_back( pg[2]);
      }
}


//_____________________________________________________________________________

void FWOverlapTableManager::recalculateVisibility( )
{
   m_row_to_index.clear();

   m_row_to_index.push_back(0);
   int cnt = 0;
   bool rnrChld = false;
   for (FWGeometryTableManagerBase::Entries_i i = m_entries.begin(); i!= m_entries.end(); ++i, ++cnt)
   {
      if (i->m_parent == 0) {
        if ((m_browser->m_rnrOverlap.value() && (strncmp(i->m_node->GetName(), "Ovlp", 3) == 0)) ||
            (m_browser->m_rnrExtrusion.value() && (strncmp(i->m_node->GetName(), "Extr", 3) == 0)) )
         {
            rnrChld = i->testBit(FWGeometryTableManagerBase::kExpanded);
            m_row_to_index.push_back(cnt);
         }
         else
         {
            rnrChld = false;
         }
      }
      else if (rnrChld)
      {
         m_row_to_index.push_back(cnt);
      }
   }
}

//______________________________________________________________________________

bool  FWOverlapTableManager::nodeIsParent(const NodeInfo& data) const
{
   return   data.m_parent <= 0 ;
}

//______________________________________________________________________________

const TGeoOverlap*  FWOverlapTableManager::referenceOverlap(int x) const 
{
   int ovlIdx = -1;
   for (int i = 0; i <= x; ++i)
   {
      if (m_entries[i].m_parent == 0) ovlIdx++;
   }

   TEveGeoManagerHolder gmgr( FWGeometryTableViewManager::getGeoMangeur());
   if (ovlIdx < 0 && gGeoManager->GetListOfOverlaps() == 0 ) return 0;
   return (TGeoOverlap*)gGeoManager->GetListOfOverlaps()->At(ovlIdx);
}

//______________________________________________________________________________

const char* FWOverlapTableManager::cellName(const NodeInfo& data) const
{
   if (data.m_parent == -1)
   {
     int ne = 0;
     int no = 0;
     TGeoOverlap* ovl;
     TEveGeoManagerHolder gmgr( FWGeometryTableViewManager::getGeoMangeur());
     TIter next_ovl(gGeoManager->GetListOfOverlaps());
     while((ovl = (TGeoOverlap*)next_ovl())) 
       ovl->IsOverlap() ? no++ : ne++;
     
     return Form("%s Ovl[%d] Ext[%d]", data.m_node->GetName(), no, ne);
   }
   else
   {
      return data.name();
   }
}

//______________________________________________________________________________

FWTableCellRendererBase* FWOverlapTableManager::cellRenderer(int iSortedRowNumber, int iCol) const
{  
   if (m_row_to_index.empty()) return &m_renderer;

   int unsortedRow =  m_row_to_index[iSortedRowNumber];
   ESelectionState sstate = nodeSelectionState(unsortedRow);
   if (sstate == kSelected)
   {
      m_highlightContext->SetBackground(0xc86464);
   }
   else if (sstate == kHighlighted )
   {
      m_highlightContext->SetBackground(0x6464c8);
   }
   else if ( sstate == kFiltered )
   {
      if (iCol == kMaterial)
         m_highlightContext->SetBackground(0xdddddd);
      else 
         sstate = kNone;
   }
   bool isSelected = sstate != kNone;

   //      FWTextTreeCellRenderer* renderer = &m_renderer;
   if (unsortedRow < 0) printf("!!!!!!!!!!!!!!!! error %d %d \n",unsortedRow,  iSortedRowNumber);


   const NodeInfo& data = m_entries[unsortedRow];

   if (iCol == 0)
   {
      m_renderer.setData(cellName(data), isSelected); 

      m_renderer.setIsParent(nodeIsParent(data));

      m_renderer.setIsOpen( data.testBit(FWGeometryTableManagerBase::kExpanded));

      int level = data.m_level ;
      if (nodeIsParent(data))
         m_renderer.setIndentation(20*level);
      else
         m_renderer.setIndentation(20*level + FWTextTreeCellRenderer::iconWidth());
   }
   else
   {
      // printf("title %s \n",data.m_node->GetTitle() );
      m_renderer.setIsParent(false);
      m_renderer.setIndentation(0);

      if (iCol == 4)
      {
         if (data.m_parent == 0 ) 
            m_renderer.setData(Form("%g ", referenceOverlap(unsortedRow)->GetOverlap() ),  isSelected);
         else
            m_renderer.setData("",  isSelected);
      }
      if (iCol == 1)
      {
         // m_colorBoxRenderer.setData(data.m_node->GetVolume()->GetLineColor(), isSelected);
         m_colorBoxRenderer.setData(data.m_color, isSelected);
         return  &m_colorBoxRenderer;
      }
      else if (iCol == 2 )
      {
         m_renderer.setData(getVisibility(data)  ? "On" : "-",  isSelected );

      }
      else if (iCol == 3 )
      {
         m_renderer.setData( getVisibilityChld(data) ? "On" : "-",  isSelected);

      }
      else if (iCol == 5)
      { 
         if (data.m_parent == 0) 
            m_renderer.setData(data.testBit(FWGeometryTableManagerBase::kFlag1) ? "On" : "-" , isSelected);
         else 
            m_renderer.setData("", isSelected);
      }
   }
   return &m_renderer;
}
