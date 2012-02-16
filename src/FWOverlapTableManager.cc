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
// $Id: FWOverlapTableManager.cc,v 1.1.2.16 2012/02/13 02:30:33 amraktad Exp $
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

#include "TStopwatch.h"
#include "TTimer.h"
#include "TGeoPainter.h"

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
  TEveGeoManagerHolder mangeur( FWGeometryTableViewManager::getGeoMangeur());
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
  TGeoNode* topNode =gGeoManager->GetCurrentNode();
  
  m_entries.clear();
  m_browser->m_markerVertices.clear();
  m_browser->m_markerIndices.clear();
  
  NodeInfo topNodeInfo;
  topNodeInfo.m_node   = topNode; 
  topNodeInfo.m_level  = 0;
  topNodeInfo.m_parent = -1;
  topNodeInfo.resetBit(kVisNodeSelf);
  m_entries.push_back( topNodeInfo);
  
    
  
  TGeoVolume* topVol = topNode->GetVolume();
  // ========================================================
  // void TGeoNode::CheckOverlaps(Double_t ovlp, Option_t *option)
  Int_t icheck = 0;
  Int_t ncheck = 0;
  TStopwatch *timer;
  Int_t i;  
  
  TGeoManager *geom = topVol->GetGeoManager();
  ncheck = topNode->CountDaughters(kFALSE);
  timer = new TStopwatch();
  geom->ClearOverlaps();
  geom->SetCheckingOverlaps(kTRUE);

  int oldS = 0;
  timer->Start();
  geom->GetGeomPainter()->OpProgress(topVol->GetName(),icheck,ncheck,timer,kFALSE);
  topVol->CheckOverlaps(iPrecision);
  icheck++;
  TGeoIterator next(topVol);
   if (gGeoManager->GetListOfOverlaps()->GetEntriesFast()) {
     int newCnt =  gGeoManager->GetListOfOverlaps()->GetEntriesFast();
     for (int i=0; i<newCnt; ++i)
       addOverlapEntry((TGeoOverlap*)gGeoManager->GetListOfOverlaps()->At(i), new TGeoHMatrix(*geom->GetCurrentMatrix()), topNode, next); 
     oldS= newCnt;
   }
  
  TGeoNode *node;
  //  geom->GetCurrentMatrix()->Print();
  
  TString path;

  while ((node=next())) {
    icheck++;
    if (!node->GetVolume()->IsSelected()) {
      // next.GetPath(path);
      geom->GetGeomPainter()->OpProgress(node->GetVolume()->GetName(),icheck,ncheck,timer,kFALSE);
      node->GetVolume()->SelectVolume(kFALSE);
      node->GetVolume()->CheckOverlaps(iPrecision);
      
      if (oldS !=  gGeoManager->GetListOfOverlaps()->GetEntriesFast()) {
        int newCnt =  gGeoManager->GetListOfOverlaps()->GetEntriesFast();
        TGeoHMatrix* motherm = new TGeoHMatrix(*geom->GetCurrentMatrix());        
        {
          TGeoNode* ni = topNode;
          for (Int_t i=1; i<=next.GetLevel(); i++) {
            ni = ni->GetDaughter(next.GetIndex(i));
            // printf("mult matrix %s \n", ni->GetName());
            motherm->Multiply(ni->GetMatrix());
          }
        }
        
        for (int i=oldS; i<newCnt; ++i)
          addOverlapEntry((TGeoOverlap*)gGeoManager->GetListOfOverlaps()->At(i), motherm, node, next); 
        
        oldS = newCnt;
      } 
    }   
  }   
  topVol->SelectVolume(kTRUE);
  geom->SetCheckingOverlaps(kFALSE);
  geom->SortOverlaps();
  TObjArray *overlaps = geom->GetListOfOverlaps();
  Int_t novlps = overlaps->GetEntriesFast();     
  TNamed *obj;
  for (i=0; i<novlps; i++) {
    obj = (TNamed*)overlaps->At(i);
    obj->SetName(Form("ov%05d",i));
  }
  geom->GetGeomPainter()->OpProgress("Check overlaps:",icheck,ncheck,timer,kTRUE);
  Info("CheckOverlaps", "Number of illegal overlaps/extrusions : %d\n", novlps);
  delete timer;
}


//______________________________________________________________________________



void FWOverlapTableManager::addOverlapEntry(TGeoOverlap* ovl, TGeoHMatrix* motherm, TGeoNode* mothern, TGeoIterator& it)
{                      
  int motherl = 1;//it.GetLevel();
  TGeoNodeMatrix* mother_node = new TGeoNodeMatrix((const TGeoVolume*) mothern->GetVolume(), motherm);
  TString mname = (ovl->IsOverlap()) ? "Ovl: " : "Extr";
  TString path; it.GetPath(path);
  mname += path;
  mother_node->SetNameTitle(mname.Data(), mname.Data());
  m_entries.push_back(NodeInfo(mother_node, 0, mothern->GetVolume()->GetLineColor(), motherl, kVisNodeChld |  kVisMarker));

  int parentIdx = m_entries.size() -1;
  if (ovl->IsOverlap()) m_entries.back().setBit(kOverlap);

  TString t = ovl->GetTitle();
  TObjArray* tx = t.Tokenize(" ");
  TString p1 = ((TObjString*)tx->At(0))->GetString();
  TString p2 = ((TObjString*)tx->At(2))->GetString();
  
  if (ovl->IsOverlap()) {
    TGeoNodeMatrix* gnode1 = new TGeoNodeMatrix(ovl->GetFirstVolume(), new TGeoHMatrix(*ovl->GetFirstMatrix()));
    //gnode1->SetNameTitle(ovl->GetFirstVolume()->GetName(), ovl->GetFirstVolume()->GetName());
    gnode1->SetNameTitle(p1.Data(), p1.Data());
    m_entries.push_back(NodeInfo(gnode1, parentIdx, ovl->GetFirstVolume()->GetLineColor(), motherl+1));
  }
  
  
  TGeoNodeMatrix* gnode2 = new TGeoNodeMatrix(ovl->GetSecondVolume(), new TGeoHMatrix(*ovl->GetSecondMatrix()));
//  gnode2->SetNameTitle(ovl->GetSecondVolume()->GetName(), ovl->GetSecondVolume()->GetName());
  gnode2->SetNameTitle(p2.Data(), p2.Data());
  m_entries.push_back(NodeInfo(gnode2, parentIdx, ovl->GetSecondVolume()->GetLineColor(), motherl+1));                   
  
  
  TPolyMarker3D* pm = ovl->GetPolyMarker();
  for (int j=0; j<pm->GetN(); ++j )
  {
    double pl[3];
    double pg[3];
    pm->GetPoint(j, pl[0], pl[1], pl[2]);
    motherm->LocalToMaster(pl, pg);
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
        if ((m_browser->m_rnrOverlap.value() &&  i->testBit(FWOverlapTableManager::kOverlap)) ||
            (m_browser->m_rnrExtrusion.value() && !i->testBit(FWOverlapTableManager::kOverlap)) )
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

   if (unsortedRow < 0) printf("!!!!!!!!!!!!!!!! error %d %d \n",unsortedRow,  iSortedRowNumber);


   const NodeInfo& data = m_entries[unsortedRow];
   bool isSelected = data.testBit(kHighlighted) ||  data.testBit(kSelected);
   if (data.testBit(kSelected))
   {
      m_highlightContext->SetBackground(0xc86464);
   }
   else if (data.testBit(kHighlighted) )
   {
      m_highlightContext->SetBackground(0x6464c8);
   }

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
            m_renderer.setData(data.testBit(kVisMarker) ? "On" : "-" , isSelected);
         else 
            m_renderer.setData("", isSelected);
      }
   }
   return &m_renderer;
}
