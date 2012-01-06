// -*- C++ -*-
//
// Package:     Core
// Class  :     FWGeometryTableManager
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  
//         Created:  Wed Jan  4 20:31:25 CET 2012
// $Id$
//

// system include files

// user include files
#include "Fireworks/Core/src/FWGeometryTableManager.h"
#include "Fireworks/Core/interface/FWGeometryTableViewBase.h"
#include "Fireworks/Core/interface/FWGeometryTableViewManager.h"
#include "Fireworks/Core/src/FWGeometryTableView.h"

#include "TEveUtil.h"

FWGeometryTableManager::FWGeometryTableManager(FWGeometryTableView* v):
   FWGeometryTableManagerBase(),
   m_browser(v),
   m_filterOff(true)
{
}


FWGeometryTableManager::~FWGeometryTableManager()
{
}

//____________________________________________________________________________

void FWGeometryTableManager::importChildren(int parent_idx)
{
   NodeInfo& parent        = m_entries[parent_idx];
   TGeoNode* parentGeoNode = parent.m_node; 
   int       parentLevel   = parent.m_level; 
   
   int nV = parentGeoNode->GetNdaughters();
   int dOff = 0; 
   for (int n = 0; n != nV; ++n)
   {         
      NodeInfo& data = m_entries[parent_idx + n + 1 + dOff];    
      data.m_node =   parentGeoNode->GetDaughter(n);
      data.m_level =  parentLevel + 1;
      data.m_parent = parent_idx;
      data.m_color =  data.m_node->GetVolume()->GetLineColor();
      if (data.m_level <=  m_browser->getAutoExpand()) data.setBit(kExpanded);
      
 
      importChildren(parent_idx + n + 1 + dOff);         
      getNNodesTotal(parentGeoNode->GetDaughter(n), dOff);            
   }  
}

//==============================================================================

void FWGeometryTableManager::checkHierarchy()
{
   // Used for debug: in a NodeInfo entry look TGeoNode children from parent index and check
   // if child is found.
   
   for ( size_t i = 0,  e = m_entries.size(); i != e; ++i )
   {
      if ( m_entries[i].m_level > 0)
      {
         TGeoNode* pn = m_entries[m_entries[i].m_parent].m_node;
         bool ok = false;
         for (int d = 0; d < pn->GetNdaughters(); ++d )
         {
            if (m_entries[i].m_node ==  pn->GetDaughter(d))
            {
               ok = true;
               break;
            }
         }
         if (!ok) printf("!!!!!! node %s has false parent %s \n", m_entries[i].name(), pn->GetName());
      }   
   }
}

void FWGeometryTableManager::checkChildMatches(TGeoVolume* vol,  std::vector<TGeoVolume*>& pstack)
{
   if (m_volumes[vol].m_matches)
   {
      for (std::vector<TGeoVolume*>::iterator i = pstack.begin(); i!= pstack.end(); ++i)
      {
         Match& pm =  m_volumes[*i];
         pm.m_childMatches = true; 
      }
   }

   pstack.push_back(vol);

   int nD =  vol->GetNdaughters();//TMath::Min(m_browser->getMaxDaughters(), vol->GetNdaughters());
   for (int i = 0; i!=nD; ++i)
      checkChildMatches(vol->GetNode(i)->GetVolume(), pstack);
   
   pstack.pop_back();
}

// callbacks ______________________________________________________________________________

void FWGeometryTableManager::updateFilter()
{
   std::string filterExp =  m_browser->getFilter();
   m_filterOff =  filterExp.empty();
   //   printf("update filter %s  OFF %d volumes size %d\n",filterExp.c_str(),  m_filterOff , (int)m_volumes.size());

   if (m_filterOff || m_entries.empty()) return;
   
   // update volume-match entries
   //   m_numVolumesMatched = 0;
   for (Volumes_i i = m_volumes.begin(); i!= m_volumes.end(); ++i) 
   {
      if (strcasestr(i->first->GetMaterial()->GetName(), filterExp.c_str()) > 0) {
         i->second.m_matches = true;
         //    m_numVolumesMatched++;
      }
      else {
         i->second.m_matches = false;
      }
      i->second.m_childMatches = false;
   }  

   std::vector<TGeoVolume*> pstack;
   checkChildMatches(m_entries[TMath::Max(0,m_browser->getTopNodeIdx())].m_node->GetVolume(), pstack);
 

   for (Entries_i ni = m_entries.begin(); ni != m_entries.end(); ++ni)
      ni->resetBit(kFilterCached);

}

//==============================================================================

void FWGeometryTableManager::loadGeometry( TGeoNode* iGeoTopNode, TObjArray* iVolumes)
{
#ifdef PERFTOOL_GEO_TABLE  
   ProfilerStart("loadGeo");
#endif
   
   // Prepare data for cell render.
   
   // clear entries
   m_entries.clear();
   m_row_to_index.clear();
   m_volumes.clear();
   m_levelOffset = 0;

   // set volume table for filters
   boost::unordered_map<TGeoVolume*, Match>  pipi(iVolumes->GetSize());
   m_volumes.swap(pipi);
   TIter next( iVolumes);
   TGeoVolume* v;
   while ((v = (TGeoVolume*) next()) != 0)
      m_volumes.insert(std::make_pair(v, Match()));

   if (!m_filterOff)
      updateFilter();  

   // add top node to init
 
   int nTotal = 0;
   NodeInfo topNodeInfo;
   topNodeInfo.m_node   = iGeoTopNode;
   topNodeInfo.m_level  = 0;
   topNodeInfo.m_parent = -1;
   if (m_browser->getAutoExpand())
      topNodeInfo.setBit(kExpanded);

   getNNodesTotal(topNodeInfo.m_node , nTotal);
   m_entries.resize(nTotal+1);
   m_entries[0] = topNodeInfo;

   importChildren(0);

   // checkHierarchy();
   
#ifdef PERFTOOL_GEO_TABLE  
   ProfilerStop();
#endif
}

//______________________________________________________________________________

void  FWGeometryTableManager::checkExpandLevel()
{return;
   // check expand state
   int ae = m_browser->getAutoExpand() +  m_levelOffset;
   for (Entries_i i = m_entries.begin(); i != m_entries.end(); ++i)
   {
      if (i->m_level  < ae)
         i->setBit(kExpanded);
      else
         i->resetBit(kExpanded);
   } 
}


//______________________________________________________________________________
void FWGeometryTableManager::printMaterials()
{
   std::map<TGeoMaterial*, std::string> mlist;
   Entries_i it = m_entries.begin();
   std::advance(it, m_selectedIdx );
   int nLevel = it->m_level;
   it++;
   while (it->m_level > nLevel)
   {
      TGeoMaterial* m = it->m_node->GetVolume()->GetMaterial();
      if (mlist.find(m) == mlist.end())
      {
         mlist[m] = m->GetName();
      } 
      it++;
   }

   printf("size %d \n", (int)mlist.size());
   for(std::map<TGeoMaterial*, std::string>::iterator i = mlist.begin(); i != mlist.end(); ++i)
   {
      printf("material %s \n", i->second.c_str());
   }

}
//______________________________________________________________________________
      
void FWGeometryTableManager::recalculateVisibility()
{
   m_row_to_index.clear();


   int i = TMath::Max(0, m_browser->getTopNodeIdx());
   m_row_to_index.push_back(i);

   NodeInfo& data = m_entries[i];

   if (!m_filterOff)
      assertNodeFilterCache(data);

   if ((m_filterOff && data.testBit(kExpanded) == false) ||
       (m_filterOff == false && data.testBit(kChildMatches) == false) )
      return;


  
   switch (m_browser->getMode())
   {
      case FWGeometryTableViewBase::kVolume:
         recalculateVisibilityVolumeRec(i);
         break;
      default:
         recalculateVisibilityNodeRec(i);
   }
   //  printf (" child [%d] FWGeometryTableManagerBase::recalculateVisibility table size %d \n", (int)m_row_to_index.size());
}

//______________________________________________________________________________

void FWGeometryTableManager::recalculateVisibilityVolumeRec(int pIdx)
{
   TGeoNode* parentNode = m_entries[pIdx].m_node;
   int nD = parentNode->GetNdaughters();
   int dOff=0;

   // printf("----------- parent %s\n", parentNode->GetName() );

   std::vector<int> vi; 
   vi.reserve(nD);


   for (int n = 0; n != nD; ++n)
   {
      int idx = pIdx + 1 + n + dOff;
      NodeInfo& data = m_entries[idx];

      bool toAdd = true;
      for (std::vector<int>::iterator u = vi.begin(); u != vi.end(); ++u )
      {
         TGeoVolume* neighbourVolume =  parentNode->GetDaughter(*u)->GetVolume();
         if (neighbourVolume == data.m_node->GetVolume())
         {
            toAdd = false;
            // printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            break;
         }
      }

      if (toAdd)
      {
         vi.push_back(n);
         if (m_filterOff)
         {
            //    std::cout << data.nameIndent() << std::endl;
            m_row_to_index.push_back(idx);
            if (data.testBit(kExpanded)) recalculateVisibilityVolumeRec(idx);
         }
         else
         {
            assertNodeFilterCache(data);
            if (data.testBitAny(kMatches | kChildMatches)) m_row_to_index.push_back(idx); 
            if (data.testBit(kChildMatches) && data.testBit(kExpanded)) recalculateVisibilityVolumeRec(idx);
         }
      }
      FWGeometryTableManagerBase::getNNodesTotal(parentNode->GetDaughter(n), dOff);
   }
}

//______________________________________________________________________________

void FWGeometryTableManager::recalculateVisibilityNodeRec( int pIdx)
{
   TGeoNode* parentNode = m_entries[pIdx].m_node;
   int nD = parentNode->GetNdaughters();
   int dOff=0;
   for (int n = 0; n != nD; ++n)
   {
      int idx = pIdx + 1 + n + dOff;
      NodeInfo& data = m_entries[idx];

      if (m_filterOff)
      {
         m_row_to_index.push_back(idx);
         if (data.testBit(kExpanded)) recalculateVisibilityNodeRec(idx);
      }
      else
      {
         assertNodeFilterCache(data);
         if (data.testBitAny(kMatches | kChildMatches)) m_row_to_index.push_back(idx); 
         if (data.testBit(kChildMatches) && data.testBit(kExpanded) ) recalculateVisibilityNodeRec(idx);
      }

      FWGeometryTableManagerBase::getNNodesTotal(parentNode->GetDaughter(n), dOff);
   }
} 

//______________________________________________________________________________
void FWGeometryTableManager::assertNodeFilterCache(NodeInfo& data)
{
   if (!data.testBit(kFilterCached))
   {
      bool matches = m_volumes[data.m_node->GetVolume()].m_matches;
      data.setBitVal(kMatches, matches);
      setVisibility(data, matches);

      bool childMatches = m_volumes[data.m_node->GetVolume()].m_childMatches;
      data.setBitVal(kChildMatches, childMatches);
      data.setBitVal(kExpanded, childMatches);
      setVisibilityChld(data, childMatches);

      data.setBit(kFilterCached);
      //  printf("%s matches [%d] childMatches [%d] ................ %d %d \n", data.name(), data.testBit(kMatches), data.testBit(kChildMatches), matches , childMatches);
   }
}

//______________________________________________________________________________
void FWGeometryTableManager::setVisibility(NodeInfo& data, bool x)
{
   if (m_browser->getVolumeMode())
   {
      if (data.m_node->GetVolume()->IsVisible() != x)
      {
         FWGeometryTableViewManager::getGeoMangeur();
         data.m_node->GetVolume()->SetVisibility(x);
      }
   }
   else
   {
      data.setBitVal(kVisNodeSelf, x);
   }
}

//______________________________________________________________________________

void FWGeometryTableManager::setVisibilityChld(NodeInfo& data, bool x)
{
   if (m_browser->getVolumeMode())
   {
      if (data.m_node->GetVolume()->IsVisibleDaughters() != x)
      {
         TEveGeoManagerHolder gmgr( FWGeometryTableViewManager::getGeoMangeur());
         data.m_node->GetVolume()->VisibleDaughters(x);
      }
   }
   else
      data.setBitVal(kVisNodeChld, x);
}

//______________________________________________________________________________

bool  FWGeometryTableManager::getVisibility(const NodeInfo& data) const
{
   if (m_browser->getVolumeMode())
      return data.m_node->GetVolume()->IsVisible();

   return  data.testBit(kVisNodeSelf);   
}

bool  FWGeometryTableManager::getVisibilityChld(const NodeInfo& data) const
{
   if (m_browser->getVolumeMode())
      return data.m_node->GetVolume()->IsVisibleDaughters();

   return  data.testBit(kVisNodeChld);   
}

//______________________________________________________________________________

bool  FWGeometryTableManager::nodeIsParent(const NodeInfo& data) const
{
   return   (data.m_node->GetNdaughters() != 0) && (m_filterOff || data.testBit(kChildMatches) );
}

//______________________________________________________________________________

FWGeometryTableManagerBase::ESelectionState FWGeometryTableManager::nodeSelectionState(int idx) const
{
   if (  m_selectedIdx == idx || ( m_browser->getVolumeMode() && (m_entries[m_selectedIdx].m_node->GetVolume() == m_entries[idx].m_node->GetVolume()) ))
   {
      return FWGeometryTableManagerBase::kSelected;
   }
   if  (  m_highlightIdx == idx )
   {
      return FWGeometryTableManagerBase::kHighlighted;
   }
   else if ( (!m_filterOff &&  m_volumes[m_entries[idx].m_node->GetVolume()].m_matches) )
   {
      return FWGeometryTableManagerBase::kFiltered;
   }

   return FWGeometryTableManagerBase::kNone;
}
