// -*- C++ -*-
//
// Package:     Core
// Class  :     FWGeometryTableManager
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Thomas McCauley, Alja Mrak-Tadel
//         Created:  Thu Jan 27 14:50:57 CET 2011
// $Id: FWGeometryTableManager.cc,v 1.1 2011/01/27 19:43:28 amraktad Exp $
//

// system include files

// user include files
#include <iostream>

#include "Fireworks/Core/interface/FWGeometryTableManager.h"
#include "Fireworks/TableWidget/interface/GlobalContexts.h"
#include "TGeoManager.h"

const char* FWGeometryTableManager::NodeInfo::name() const
{
   return m_node->GetName();
}

FWGeometryTableManager::FWGeometryTableManager()
   : m_selectedRow(-1)
{ 
   m_renderer.setGraphicsContext(&fireworks::boldGC());
   m_daughterRenderer.setGraphicsContext(&fireworks::italicGC());
   reset();
}

void FWGeometryTableManager::implSort(int, bool)
{
   recalculateVisibility();
}

int FWGeometryTableManager::unsortedRowNumber(int unsorted) const
{
   return unsorted;
}

int FWGeometryTableManager::numberOfRows() const 
{
   return m_row_to_index.size();
}

 int FWGeometryTableManager::numberOfColumns() const 
{
   return 2;
}
   

std::vector<std::string> FWGeometryTableManager::getTitles() const 
{
   std::vector<std::string> returnValue;
   returnValue.reserve(numberOfColumns());
   returnValue.push_back("Name");
   returnValue.push_back("Title");

   return returnValue;
}
  
void FWGeometryTableManager::setSelection (int row, int column, int mask) 
{
   if(mask == 4) 
   {
      if( row == m_selectedRow) 
      {
         row = -1;
      }
   }
   changeSelection(row, column);
}

const std::string FWGeometryTableManager::title() const 
{
   return "Geometry";
}

int FWGeometryTableManager::selectedRow() const 
{
   return m_selectedRow;
}

int FWGeometryTableManager::selectedColumn() const 
{
   return m_selectedColumn;
}
 
bool FWGeometryTableManager::rowIsSelected(int row) const 
{
   return m_selectedRow == row;
}

void FWGeometryTableManager::changeSelection(int iRow, int iColumn)
{      
   if (iRow == m_selectedRow && iColumn == m_selectedColumn)
      return;
      
   m_selectedRow = iRow;
   m_selectedColumn = iColumn;

   indexSelected_(iRow, iColumn);
   visualPropertiesChanged();
}    

void FWGeometryTableManager::reset() 
{
   changeSelection(-1, -1);
   recalculateVisibility();
   dataChanged();
   visualPropertiesChanged();
}

//______________________________________________________________________________

void FWGeometryTableManager::fillNodeInfo(TGeoManager* geoManager)
{
   //  std::cout<<"fillNodeInfo  " << geoManager->GetTopNode()->GetName() <<std::endl;

   m_entries.clear();
   m_row_to_index.clear();

   NodeInfo topNodeInfo;
   topNodeInfo.m_node   = geoManager->GetTopNode();
   topNodeInfo.m_level  = 0;
   topNodeInfo.m_parent = -1;
   m_entries.push_back(topNodeInfo);

   importChildren(0);

   reset();
}
  
void FWGeometryTableManager::importChildren(int parent_idx)
{
   NodeInfo& parent = m_entries[parent_idx];
   TGeoNode* geoNode = parent.m_node; 
   parent.m_imported = true;
   int level =  parent.m_level + 1;

   Entries_i it = m_entries.begin();
   std::advance(it, parent_idx+1);
   m_entries.insert(it, geoNode->GetNdaughters(), NodeInfo());


   //  printf("import %s[%d]  \n", geoNode->GetName(), geoNode->GetNdaughters() );
   for (int n = 0; n != geoNode->GetNdaughters(); ++n)
   {
      NodeInfo &nodeInfo = m_entries[parent_idx + n + 1];
      nodeInfo.m_node =   geoNode->GetDaughter(n);
      nodeInfo.m_level =  level;
      nodeInfo.m_parent = parent_idx;
   }
   

   // shift parent indices for array succesors
   for (int i = (parent_idx + geoNode->GetNdaughters() + 1), n = m_entries.size() ; i < n; ++i)
   {
      if (m_entries[i].m_parent > m_entries[parent_idx].m_parent)
      {
         // printf("xxxxxxxxxxxxxxx shhift parent for %s \n",  m_entries[i].name());       
         m_entries[i].m_parent +=  geoNode->GetNdaughters();

      }
   }
   checkHierarchy();
}

void FWGeometryTableManager::checkHierarchy()
{
   // function only for debug purposes 

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
//______________________________________________________________________________

  
FWTableCellRendererBase* FWGeometryTableManager::cellRenderer(int iSortedRowNumber, int iCol) const
{
   if (static_cast<int>(m_row_to_index.size()) <= iSortedRowNumber)
   {
      m_renderer.setData(std::string("FWGeometryTableManager::cellRenderer() Error!"), false);
      return &m_renderer;
   }       

   FWTextTreeCellRenderer* renderer = &m_renderer;
   int unsortedRow =  m_row_to_index[iSortedRowNumber];
   const NodeInfo& data = m_entries[unsortedRow];


   if (iCol == 0)
   {
      renderer->setData(Form("%s [%d]", data.m_node->GetName(), data.m_node->GetNdaughters()), false);
      renderer->setIsParent(data.m_node->GetNdaughters() > 0);
      renderer->setIsOpen(data.m_expanded);
      if (data.m_node->GetNdaughters() == 0)
         renderer->setIndentation(10*data.m_level + FWTextTreeCellRenderer::iconWidth());
      else
         renderer->setIndentation(10*data.m_level);
   }
   else 
   {
      // printf("title %s \n",data.m_node->GetTitle() );
      renderer->setData(Form("level .. %d", data.m_level), false);
      renderer->setIsParent(false);
      renderer->setIndentation(0);
   }

   return renderer;
}

void FWGeometryTableManager:: setExpanded(int row)
{
   if (row == -1)
      return;

   //  printf("click %s \n", m_entries[idx].name());
   int idx = rowToIndex()[row];
   Entries_i it = m_entries.begin();
   std::advance(it, idx);
   NodeInfo& data = *it;
   data.m_expanded = !data.m_expanded;
   if (data.m_expanded  &&  data.m_imported == false)
   {
      importChildren(idx);
   }

   recalculateVisibility();
   dataChanged();
   visualPropertiesChanged();
}


void FWGeometryTableManager::recalculateVisibility()
{
   m_row_to_index.clear();

   for ( size_t i = 0,  e = m_entries.size(); i != e; ++i )
   {   
      NodeInfo &data = m_entries[i];
      // printf("visiblity for %s \n", data.m_node->GetName() );
      if (data.m_parent == -1)
      {
         data.m_visible = true;
      }
      else 
      {
         data.m_visible = m_entries[data.m_parent].m_expanded && m_entries[data.m_parent].m_visible;
         //  printf("visible : parent[%d] expanded %d  vis %d\n",data.m_parent, m_entries[data.m_parent].m_expanded, m_entries[data.m_parent].m_visible);
      }
   }

   // Put in the index only the entries which are visible.
   for (size_t i = 0, e = m_entries.size(); i != e; ++i)
      if (m_entries[i].m_visible)
         m_row_to_index.push_back(i);

   // printf("entries %d \n", m_entries.size());
} 

