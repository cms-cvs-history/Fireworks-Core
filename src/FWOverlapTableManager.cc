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
// $Id: FWOverlapTableManager.cc,v 1.1.2.1 2012/01/06 00:27:34 amraktad Exp $
//

// system include files

// user include files
#include "Fireworks/Core/src/FWOverlapTableManager.h"
#include "Fireworks/Core/interface/FWGeometryTableViewManager.h"

#include "TEvePointSet.h"
#include "TGeoVolume.h"
#include "TGeoMatrix.h"
#include "TGeoShape.h"
#include "TGeoBBox.h"
#include "TGeoMatrix.h"
#include "TEveUtil.h"

#include "TGeoOverlap.h"
#include "TGeoManager.h"
#include "TPolyMarker3D.h"

FWOverlapTableManager::FWOverlapTableManager(FWOverlapTableView* ):
   FWGeometryTableManagerBase()
{
}

FWOverlapTableManager::~FWOverlapTableManager()
{
}

void FWOverlapTableManager::importOverlaps( TGeoNode*  top_node, TObjArray* iVolumes, TEvePointSet* iPnts)
{
  
   TEveGeoManagerHolder gmgr( FWGeometryTableViewManager::getGeoMangeur());

   //   m_entries[m_browser->getTopNodeIdx()].m_node->CheckOverlaps(1.0);
   //   gGeoManager->cd("/cms:World_1/cms:CMSE_1/muonBase:MUON_1/muonBase:MB_1/");
   gGeoManager->cd("/cms:World_1/cms:CMSE_1/muonBase:MUON_1/muonBase:MB_1/muonBase:MBWheel_1N_2");

   gGeoManager->GetCurrentNode()->CheckOverlaps(1.0);
   TObjArray* lOverlaps = gGeoManager->GetListOfOverlaps();

   //   TEvePointSet* pointSet = m_browser->overlapPnts();


   NodeInfo topNodeInfo;
   topNodeInfo.m_node   = top_node;
   topNodeInfo.m_level  = 0;
   topNodeInfo.m_parent = -1;
   m_entries.push_back( topNodeInfo);

   std::vector<float> pnts;

   {
      TIter next_ovl(lOverlaps);
      TGeoOverlap *ovl;

      TGeoIterator top_git   (gGeoManager->GetTopVolume());
      TGeoNode*    top_gnode (top_git.Next());
      while((ovl = (TGeoOverlap*)next_ovl())) 
      {
         if (1) printf(Form("Scanning for Extp=%d, Ovlp=%d: vol1=%-12s vol2=%-12s \n",
                            ovl->IsExtrusion(),  ovl->IsOverlap(),
                            ovl->GetFirstVolume()->GetName(),
                            ovl->GetSecondVolume()->GetName()));

         TGeoNode    *n1 =  0, *n2 =  0, *gnode;
         TGeoVolume  *v1 =  0, *v2 =  0, *gvol;
         Int_t        l1 = -1,  l2 = -1;
         TGeoIterator git(top_git);
         gnode = top_gnode;
         do {
            gvol = gnode->GetVolume();
            if(gvol == ovl->GetFirstVolume())
            {
               top_git = git; top_gnode = gnode;

               n1 = gnode; v1 = gvol;  l1 = git.GetLevel();
               //printf("  Found first  vol lvl=%d \n", l1);
               if( ovl->IsOverlap())
                  git.Skip();

               while((gnode = git.Next()) != 0) {

                  gvol = gnode->GetVolume();
                  if(gvol == ovl->GetSecondVolume()) {
                     n2 = gnode; v2 = gvol; l2 = git.GetLevel();
                     // printf("  Found second vol lvl=%d \n", l2);


                     Int_t       motherl;
                     TGeoNode*   mothern;
                     TGeoVolume* motherv;
                     if(ovl->IsExtrusion()) {
                        motherl = l1;
                        mothern = n1;
                        motherv = v1;
                     } else {
                        motherl = TMath::Min(l1, l2);
                        do {
                           --motherl;
                           mothern = motherl > 0 ? git.GetNode(motherl) : top_node;
                           motherv = mothern->GetVolume();
                        } while(motherv->IsAssembly());

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
                        TString mname  = ovl->IsExtrusion() ? "Extr: " : "Ovlp: ";
                        mname += motherv->GetName();
                        TString mtitle = top_node->GetVolume()->GetName();
                        for(Int_t l=1; l<motherl; ++l) {
                           mtitle += "/";
                           mtitle += git.GetNode(l)->GetVolume()->GetName();
                        }

                        //  printf("Importing %s '%s' from %s",mname.Data(), mtitle.Data(),ovl->GetName());
                    
                        TGeoNodeMatrix* mother_node = new TGeoNodeMatrix((const TGeoVolume*)motherv, new TGeoHMatrix(motherm));
                        mother_node->SetNameTitle(mname.Data(), mtitle.Data());
                        m_entries.push_back(NodeInfo(mother_node, 0, motherv->GetLineColor(), motherl, kVisNodeChld | kExpanded));
                        int parentIdx = m_entries.size() -1;

                        TGeoNodeMatrix* gnode1 = new TGeoNodeMatrix(v1, ovl->GetFirstMatrix());
                        gnode1->SetName(Form ("%s", v1->GetName()));
                        m_entries.push_back(NodeInfo(gnode1, parentIdx, v1->GetLineColor(), l1));
                        if (ovl->IsOverlap()) {
                           TGeoNodeMatrix* gnode2 = new TGeoNodeMatrix(v2, ovl->GetSecondMatrix());
                           gnode2->SetName(Form ("%s", v2->GetName()));
                           m_entries.push_back(NodeInfo(gnode2, parentIdx, v2->GetLineColor(), l2));
                        }

                        TPolyMarker3D* pm = ovl->GetPolyMarker();
                        for (int j=0; j<pm->GetN(); ++j )
                        {
                           double pl[3];
                           double pg[3];
                           pm->GetPoint(j, pl[0], pl[1], pl[2]);
                           motherm.LocalToMaster(pl, pg);
                           // pnts->SetNextPoint(pg[0], pg[1], pg[2]);
                           // printf("set point %f %f %f\n", pg[0], pg[1], pg[2]);
                           pnts.push_back( pg[0]);
                           pnts.push_back( pg[1]);
                           pnts.push_back( pg[2]);

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
         }

      }
   }

   iPnts->SetPolyMarker(int(pnts.size()/3), &pnts[0], 4);
}

//_____________________________________________________________________________

void FWOverlapTableManager::recalculateVisibility( )
{
   m_row_to_index.clear();

   printf("FWGeometryTableManagerBase::recalculateVisibilityOverlap \n");
   m_row_to_index.push_back(0);
   int cnt = 0;
   bool rnrChld = false;
   for (FWGeometryTableManagerBase::Entries_i i = m_entries.begin(); i!= m_entries.end(); ++i, ++cnt)
   {
      if (i->m_parent == 0) {
         rnrChld = i->testBit(FWGeometryTableManagerBase::kExpanded);
         m_row_to_index.push_back(cnt);
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

const char* FWOverlapTableManager::cellName(const NodeInfo& data) const
{
   return data.name();
}

