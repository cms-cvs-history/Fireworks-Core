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
// $Id: FWGeometryTableManager.cc,v 1.1.2.2 2011/02/04 20:24:07 amraktad Exp $
//

// system include files

// user include files
#include <iostream>

#include "Fireworks/Core/interface/FWGeometryTableManager.h"
#include "Fireworks/Core/interface/FWGeometryTable.h"
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

FWGeometryTableManager::FWGeometryTableManager(FWGeometryTable* browser)
   : m_browser(browser),
     m_selectedRow(-1),
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
   printf("=======================================   implSort[%d]\n", (int)m_entries.size());

   // Decide whether or not items match the filter.
   for (size_t i = 0, e = m_entries.size(); i != e; ++i)
   {
      NodeInfo &data = m_entries[i];
      // First of all decide whether or not we match conditions.
      data.m_matches = true;

      /*
      // unique volume
      if (m_browser->m_mode.value() == FWGeometryTable::kVolume)
      {
      if (!strstr(data.name(), "_1"))
      data.m_matches = false;
      }
      // material
      if (data.m_matches)*/

      {

         if (!strstr(data.m_node->GetVolume()->GetMaterial()->GetName(), m_browser->m_searchExp.value().c_str()))
            data.m_matches = false;
         else
            data.m_matches = true;
      }
   }

   // We reset whether or not a given parent has children that match the
   // filter, and we recompute the whole information by checking all the
   // children.
   for (size_t i = 0, e = m_entries.size(); i != e; ++i)
      m_entries[i].m_childMatches = false;

   std::vector<int> stack;
   int previousLevel = 0;
   for (size_t i = 0, e = m_entries.size(); i != e; ++i)
   {
      NodeInfo &data = m_entries[i];
      // Top level.
      if (data.m_parent == -1)
      {
         previousLevel = 0;
         continue;
      }
      // If the level is greater than the previous one,
      // it means we are among the children of the 
      // previous level, hence we push the parent to
      // the stack.
      // If the level is not greater than the previous
      // one it means we have popped out n levels of
      // parents, where N is the difference between the 
      // new and the old level. In this case we
      // pop up N parents from the stack.
      if (data.m_level > previousLevel)
         stack.push_back(data.m_parent);
      else
         for (size_t pi = 0, pe = previousLevel - data.m_level; pi != pe; ++pi)
            stack.pop_back();
 
      if (data.m_matches)
         for (size_t pi = 0, pe = stack.size(); pi != pe; ++pi)
            m_entries[stack[pi]].m_childMatches = true;

      previousLevel = data.m_level;
   }
       
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

   if (m_browser->m_mode.value() == FWGeometryTable::kNode )
      returnValue.push_back("Node Name");
   else
      returnValue.push_back("Volume Name");

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

//==============================================================================

int kMaxDaughters = 5000;
int kMaxDepth = 5;

void FWGeometryTableManager::fillNodeInfo(TGeoManager* geoManager)
{
   //  std::cout<<"fillNodeInfo  " << geoManager->GetTopNode()->GetName() <<std::endl;

   m_entries.clear();
   m_row_to_index.clear();

   NodeInfo topNodeInfo;
   //   topNodeInfo.m_node   = geoManager->GetTopNode();
   topNodeInfo.m_node   = geoManager->GetTopNode()->GetDaughter(0)->GetDaughter(1);
   topNodeInfo.m_level  = 0;
   topNodeInfo.m_parent = -1;
   m_entries.push_back(topNodeInfo);

   importChildren(0, kMaxDepth);
   int maxDepth = 4;
   if (1)
   {
      for (Entries_i i = m_entries.begin(); i != m_entries.end(); ++i)
         if ((*i).m_level < TMath::Min(maxDepth, kMaxDepth-1)) 
            (*i).m_expanded = true;
   }
   printf("size %d \n", m_entries.size());
   reset();
}

//==============================================================================


void dOffset(TGeoNode* geoNode, int level, int maxLevel, int& off)
{
   //  printf("\033[01;36m %s (c:%d,M:%d)\033[22;0m ", geoNode->GetName(), level, maxLevel);

   int nD = TMath::Min(kMaxDaughters, geoNode->GetNdaughters());
   if (level <  (maxLevel))
   {
      off += nD;
      // printf("ddd %d \n", off);
      for (int i = 0; i < nD; ++i )
      {
         dOffset(geoNode->GetDaughter(i), level+1, maxLevel, off);
      }
      //   printf(" %d ", off);
   }
}

void FWGeometryTableManager::importChildren(int parent_idx, int maxLevel)
{
   bool debug = false;

   // printf("importing from parent %d entries size %d \n",  parent_idx, m_entries.size());
   NodeInfo& parent  = m_entries[parent_idx];
   TGeoNode* geoNode = parent.m_node; 
   parent.m_imported = true;

   int nD = TMath::Min(kMaxDaughters, geoNode->GetNdaughters());

   Entries_i it = m_entries.begin();
   std::advance(it, parent_idx+1);
   m_entries.insert(it, nD, NodeInfo());


   if (debug) printf("\033[32mlevel %d import \033[0m from parent \033[01;33m %s[%d] \033[0m \n" ,parent.m_level, geoNode->GetName(), parent_idx);
   for (int n = 0; n != nD; ++n)
   {
      NodeInfo &nodeInfo = m_entries[parent_idx + n + 1];
      nodeInfo.m_node =   geoNode->GetDaughter(n);
      nodeInfo.m_level =  parent.m_level + 1;
      nodeInfo.m_parent = parent_idx;
     if (debug)  printf(" add %s\n", nodeInfo.name());
   }

   // shift parent indices for array succesors
   for (int i = (parent_idx + nD + 1), n = m_entries.size() ; i < n; ++i)
   {
      if (m_entries[i].m_parent > m_entries[parent_idx].m_parent)
      {
        if (debug)  printf("\033[22;34m -> shhift %s's parent  \033[0m\n",  m_entries[i].name());       
         m_entries[i].m_parent +=  nD;

      }
   }

   int dOff = 0;
   if ((parent.m_level+1) < maxLevel)
   {
      for (int n = 0; n != nD; ++n)
      {
        if (debug)  printf("begin [%d]to... recursive import of daughter %d %s parent-index-offset %d\n",parent.m_level+1, n, geoNode->GetDaughter(n)->GetName(), dOff );
         importChildren(parent_idx + n + 1 + dOff, maxLevel);
       if (debug)   printf("end [%d]... recursive import of daughter %d %s parent-index-offset %d\n\n\n", parent.m_level+1, n, geoNode->GetDaughter(n)->GetName(), dOff );
       
         if (geoNode->GetNdaughters())
            dOffset(geoNode->GetDaughter(n), parent.m_level+1, maxLevel, dOff);
       if (debug)   printf("\n");
      }
   }
   fflush(stdout);
   checkHierarchy(); // debug
}

//==============================================================================

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
   if (0)
   {
      TGGC* gc = ( TGGC*)m_renderer.graphicsContext();
  
   if (m_browser->m_mode.value() == FWGeometryTable::kVolume)
      gc->SetForeground(gVirtualX->GetPixel(kGray));
   else
      gc->SetForeground(gVirtualX->GetPixel(kBlack));
   }
   //   bool isSelected = (iSortedRowNumber == m_selectedRow);
   bool isSelected = data.m_imported;

   TGeoNode& gn = *data.m_node;

   if (iCol == kName)
   {
      //   printf("redere\n");
      if (m_browser->m_mode.value() == FWGeometryTable::kVolume)
         renderer->setData(Form("%s [%d]", gn.GetVolume()->GetName(), gn.GetVolume()->GetNdaughters() ), isSelected);
      else    
         renderer->setData(Form("%s [%d]", gn.GetName(), gn.GetNdaughters() ), isSelected); 

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

   // We do not want to handle expansion
   // events while in filtering mode.
   if (filterOn())
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
         data.m_visible = data.m_childMatches || data.m_matches || !filterOn();
      }
      else 
      {
         data.m_visible = data.m_matches || data.m_childMatches || ( !filterOn() && m_entries[data.m_parent].m_expanded && m_entries[data.m_parent].m_visible);
      }
   }

   // Put in the index only the entries which are visible.
   for (size_t i = 0, e = m_entries.size(); i != e; ++i)
      if (m_entries[i].m_visible)
         m_row_to_index.push_back(i);

   // printf("entries %d \n", m_entries.size());
} 

bool FWGeometryTableManager::filterOn() const
{
   return !m_browser->m_searchExp.value().empty();
}
