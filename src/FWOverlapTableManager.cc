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
// $Id: FWOverlapTableManager.cc,v 1.1.2.10 2012/01/19 00:07:25 amraktad Exp $
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

#include "TGeoNode.h"
#include "TGeoOverlap.h"
#include "TGeoManager.h"
#include "TPolyMarker3D.h"

std::vector<std::string> cell_names;

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

      std::string::iterator bi = iPath.begin();
      bi++;
      // first level offset  0
      while (bi != iPath.end())  { if (*bi == '/') { bi++; break;};  bi++;} 
      // printf("ffff %s \n", iPath.c_str());
      for (std::string::iterator i = bi; i != iPath.end(); ++i)
      {
         if (*i == '/') m_levelOffset++;
      }
      //   printf("level offset %d \n", m_levelOffset);
    
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

   cell_names.clear();

   NodeInfo topNodeInfo;
   topNodeInfo.m_node   = top_node; 
   topNodeInfo.m_level  = m_levelOffset;
   topNodeInfo.m_parent = -1;
   topNodeInfo.resetBit(kVisNodeSelf);
   m_entries.push_back( topNodeInfo);
   cell_names.push_back(top_node->GetName());

   std::vector<float> pnts;
   int ovlCnt = 1;
   {
      TIter next_ovl(lOverlaps);
      TGeoOverlap *ovl;

      TGeoIterator top_git   (gGeoManager->GetTopVolume());
      TGeoNode*    top_gnode (top_git.Next());
      while((ovl = (TGeoOverlap*)next_ovl())) 
      {
         if (1) printf(Form("[%d/%d] Scanning for Extp=%d, Ovlp=%d: vol1=%-12s vol2=%-12s \n",      
                            ovlCnt++,lOverlaps->GetEntries(),
                            ovl->IsExtrusion(),  ovl->IsOverlap(),
                            ovl->GetFirstVolume()->GetName(),
                            ovl->GetSecondVolume()->GetName()));

      reiterate:
         TGeoNode    *n1 =  0, *n2 =  0, *gnode;
         TGeoVolume  *v1 =  0, *v2 =  0, *gvol;
         Int_t        l1 = -1,  l2 = -1;
         TGeoIterator git(top_git);
         TString path, path1, path2;
         gnode = top_gnode;
         do {
            gvol = gnode->GetVolume();
            if(gvol == ovl->GetFirstVolume())
            {
               git.GetPath(path1);
               top_git = git; top_gnode = gnode;

               n1 = gnode; v1 = gvol;  l1 = git.GetLevel();
               //printf("  Found first  vol lvl=%d \n", l1); 
               if( ovl->IsOverlap())
                  git.Skip();

               while((gnode = git.Next()) != 0) {

                  gvol = gnode->GetVolume();
                  if(gvol == ovl->GetSecondVolume()) {

                     git.GetPath(path2);
                     n2 = gnode; v2 = gvol; l2 = git.GetLevel();
                     // printf("  Found second vol lvl=%d \n", l2);


                     Int_t       motherl;
                     TGeoNode*   mothern;
                     TGeoVolume* motherv;
                     if(ovl->IsExtrusion()) {
                        motherl = l1;
                        mothern = n1;
                        motherv = v1;

                        path = path1;
                     } else {
                        motherl = TMath::Min(l1, l2);
                        do {
                           --motherl;
                           mothern = motherl > 0 ? git.GetNode(motherl) : top_node;
                           motherv = mothern->GetVolume();
                        } while(motherv->IsAssembly());

                        int ldiff = l1- motherl;
                        int scs = path1.Length();
                        do
                        {
                           scs--;
                           if (path1[scs] == '/') ldiff--;
                        }while(scs > 0 && ldiff >0);
                        path = path1;
                        path.Resize(scs);
                     }

                     TGeoHMatrix motherm;
                     {
                        TGeoNode *node = git.GetTopVolume()->GetNode(git.GetIndex(1));
                        motherm.Multiply(node->GetMatrix());
                        for (Int_t i=2; i<=motherl; i++) {
                           node = node->GetDaughter(git.GetIndex(i));
                           motherm.Multiply(node->GetMatrix());
                        }
                     }

                     //______________________________________________________________________________
                     //______________________________________________________________________________
                     //______________________________________________________________________________
                     {
                        TGeoNodeMatrix* mother_node = new TGeoNodeMatrix((const TGeoVolume*)motherv, new TGeoHMatrix(motherm));

                        m_entries.push_back(NodeInfo(mother_node, 0, motherv->GetLineColor(), motherl, kVisNodeChld |  kFlag1));
                        cell_names.push_back(path.Data());
                        int parentIdx = m_entries.size() -1;

                        TString mname  = ovl->IsExtrusion() ? "Extr: " : "Ovlp: ";
                        mname += &path[iPath.size()];
                        mother_node->SetName(mname);


                        // if(ovl->IsOverlap())  path1=&path1[path.Length()+1];
                        // path2=&path2[path.Length()+1];
                        if (ovl->IsOverlap()) {
                           TGeoNodeMatrix* gnode1 = new TGeoNodeMatrix(v1, ovl->GetFirstMatrix());
                           gnode1->SetName(Form ("%s", v1->GetName()));
                           m_entries.push_back(NodeInfo(gnode1, parentIdx, v1->GetLineColor(), l1));
                           cell_names.push_back(path1.Data());

                           TGeoNodeMatrix* gnode2 = new TGeoNodeMatrix(v2, ovl->GetSecondMatrix());
                           gnode2->SetName(Form ("%s", v2->GetName()));
                           m_entries.push_back(NodeInfo(gnode2, parentIdx, v2->GetLineColor(), l2));
                           cell_names.push_back(path2.Data());
                        }
                        else
                        {   
                           TGeoNodeMatrix* gnode2 = new TGeoNodeMatrix(v2, ovl->GetSecondMatrix());
                           gnode2->SetName(Form ("%s", v2->GetName()));
                           m_entries.push_back(NodeInfo(gnode2, parentIdx, v2->GetLineColor(), l2));
                           cell_names.push_back(path2.Data());
                   
                        }

                        TPolyMarker3D* pm = ovl->GetPolyMarker();
                        for (int j=0; j<pm->GetN(); ++j )
                        {
                           double pl[3];
                           double pg[3];
                           pm->GetPoint(j, pl[0], pl[1], pl[2]);
                           motherm.LocalToMaster(pl, pg);
                           m_browser->m_markerIndices.push_back(parentIdx);
                           m_browser->m_markerVertices.push_back( pg[0]);
                           m_browser->m_markerVertices.push_back( pg[1]);
                           m_browser->m_markerVertices.push_back( pg[2]);

                        }

                     }
                     break;
                  }
               }
               break; // found first
            }
         } while((gnode = git.Next()) != 0);

         if(v2 == 0) {
            printf( "  Could not find both volumes, resetting geo-iterator.");
            top_git.Reset();
            top_gnode = top_git.Next();
            goto reiterate;
         }

      }
   }

   printf("Import END eps=%f \n", iPrecision);
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
   return   data.m_parent == 0;
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

      int level = data.m_level - m_levelOffset;
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
            m_renderer.setData(Form("%.3f ", referenceOverlap(unsortedRow)->GetOverlap() ),  isSelected);
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
