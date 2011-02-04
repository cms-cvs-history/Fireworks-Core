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
// $Id: FWGeometryTableManager.cc,v 1.1.2.1 2011/02/01 19:00:37 amraktad Exp $
//

// system include files

// user include files
#include <iostream>

#include "Fireworks/Core/interface/FWGeometryTableManager.h"
#include "Fireworks/Core/src/FWColorBoxIcon.h"
#include "Fireworks/TableWidget/interface/GlobalContexts.h"
#include "Fireworks/TableWidget/src/FWTabularWidget.h"

#include "TMath.h"
#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TGeoMatrix.h"
#include "TGeoShape.h"
#include "TGeoBBox.h"

const char* FWGeometryTableManager::NodeInfo::name() const
{
   return m_node->GetName();
}

FWGeometryTableManager::ColorBoxRenderer::ColorBoxRenderer():
   FWTableCellRendererBase(),
   m_width(1),
   m_height(1),
   m_color(0xffffff),
   m_isSelected(false)
{
   GCValues_t gval; 
   gval.fMask       = kGCForeground | kGCBackground | kGCStipple | kGCFillStyle  | kGCGraphicsExposures;
   gval.fStipple    = gClient->GetResourcePool()->GetCheckeredBitmap();
   gval.fGraphicsExposures = kFALSE;
   gval.fBackground = gVirtualX->GetPixel(kGray);
   m_colorContext = gClient->GetResourcePool()->GetGCPool()->GetGC(&gval,kTRUE);
}

FWGeometryTableManager::ColorBoxRenderer::~ColorBoxRenderer()
{
   gClient->GetResourcePool()->GetGCPool()->FreeGC(m_colorContext->GetGC());
}

void FWGeometryTableManager::ColorBoxRenderer::setData(Color_t c, bool s)
{
   m_color = gVirtualX->GetPixel(c);
   m_isSelected = s;
}


void FWGeometryTableManager::ColorBoxRenderer::draw(Drawable_t iID, int iX, int iY, unsigned int iWidth, unsigned int iHeight)
{
   iX -= FWTabularWidget::kTextBuffer;
   iY -= FWTabularWidget::kTextBuffer;
   iWidth += 2*FWTabularWidget::kTextBuffer;
   iHeight += 2*FWTabularWidget::kTextBuffer;

   m_colorContext->SetFillStyle(kFillSolid);
   m_colorContext->SetForeground(m_color);
   gVirtualX->FillRectangle(iID, m_colorContext->GetGC(), iX, iY, iWidth, iHeight);

   if (m_isSelected)
   {
     m_colorContext->SetFillStyle(kFillOpaqueStippled);
     gVirtualX->FillRectangle(iID, m_colorContext->GetGC(), iX, iY, iWidth, iHeight);
   }
}
//==============================================================================
//==============================================================================

FWGeometryTableManager::FWGeometryTableManager()
   : m_selectedRow(-1),
     m_renderer(),
     m_colorBoxRenderer()
{ 
   setGrowInWidth(false);
   // m_renderer.setGraphicsContext(&fireworks::boldGC());

   m_colorBoxRenderer.m_width  =  50;
   m_colorBoxRenderer.m_height =  m_renderer.height();
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
   return kNumCol;
}
   

std::vector<std::string> FWGeometryTableManager::getTitles() const 
{
   std::vector<std::string> returnValue;
   returnValue.reserve(numberOfColumns());
   returnValue.push_back("Name");
   returnValue.push_back("Color");
   returnValue.push_back("RnrSelf");
   returnValue.push_back("RnrChildren");
   returnValue.push_back("Material");
   returnValue.push_back("Position");
   returnValue.push_back("Diagonal");

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
//______________________________________________________________________________

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
   if (iRow < 0) return; 
   if (iRow == m_selectedRow && iColumn == m_selectedColumn)
      return;
      
   //   int unsortedRow =  m_row_to_index[iRow];
   //  printf(":changeSelection %s indices [%d]/[%d]\n", m_entries[unsortedRow].name(),iRow ,unsortedRow );
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

   // start with level 2
   int maxLevel = 3;
   importChildren(0, 3);
   maxLevel--;
   for (Entries_i i = m_entries.begin(); i != m_entries.end(); ++i)
      if ((*i).m_level < maxLevel) 
         (*i).m_expanded = true;
   reset();
}
  
void FWGeometryTableManager::importChildren(int parent_idx, int maxLevel)
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
   checkHierarchy(); // debug

   if (level < maxLevel)
   {
      for (int n = 0; n != geoNode->GetNdaughters(); ++n)
         importChildren(parent_idx + n + 1, maxLevel);
   }
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
   bool isSelected = (iSortedRowNumber == m_selectedRow);

   TGeoNode& gn = *data.m_node;

   if (iCol == kName)
   {
      renderer->setData(Form("%s [%d]", gn.GetName(), gn.GetNdaughters()), isSelected);
      renderer->setIsParent(gn.GetNdaughters() > 0);
      renderer->setIsOpen(data.m_expanded);
      if (data.m_node->GetNdaughters() == 0)
         renderer->setIndentation(10*data.m_level + FWTextTreeCellRenderer::iconWidth());
      else
         renderer->setIndentation(10*data.m_level);

      return renderer;
   }
   else
   {
      // printf("title %s \n",data.m_node->GetTitle() );
      renderer->setIsParent(false);
      renderer->setIndentation(0);
      if (iCol == kColor)
      {
         //renderer->setData(Form("level .. %d", data.m_level),  isSelected);
         m_colorBoxRenderer.setData(gn.GetVolume()->GetLineColor(), isSelected);
         return  &m_colorBoxRenderer;
      }
      else if (iCol == kVisSelf )
      {
         renderer->setData( gn.IsVisible() ? "on" : "off",  isSelected);
         return renderer;
      }
      else if (iCol == kVisChild )
      {
         renderer->setData( gn.IsVisDaughters() ? "on" : "off",  isSelected);
         return renderer;
      }
      else if (iCol == kMaterial )
      { 
         TString d  = gn.GetVolume()->GetMaterial()->GetName();
         d.ReplaceAll("materials:", "");
         renderer->setData( d.Data(),  isSelected);
         return renderer;
      }
      else if (iCol == kPosition )
      { 
         const Double_t* p = gn.GetMatrix()->GetTranslation();
         renderer->setData(Form("[%.3f, %.3f, %.3f]", p[0], p[1], p[2]),  isSelected);
         return renderer;
      }
      else// if (iCol == kPosition  )
      { 
         TGeoBBox* gs = static_cast<TGeoBBox*>( gn.GetVolume()->GetShape());
         renderer->setData( Form("%f", TMath::Sqrt(gs->GetDX()*gs->GetDX() + gs->GetDY()*gs->GetDY() +gs->GetDZ()*gs->GetDZ() )),  isSelected);
         return renderer;
      }
   }
}
//______________________________________________________________________________



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

