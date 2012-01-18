// -*- C++ -*-
//
// Package:     Core
// Class  :     FWGeometryTableManagerBase
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Alja Mrak-Tadel, Matevz Tadel
//         Created:  Thu Jan 27 14:50:57 CET 2011
// $Id: FWGeometryTableManagerBase.cc,v 1.1.2.4 2012/01/17 06:29:34 amraktad Exp $
//

//#define PERFTOOL_GEO_TABLE

// user include files
#include <iostream>
#include <boost/bind.hpp>
#include <stack>
#ifdef PERFTOOL_GEO_TABLE 
#include <google/profiler.h>
#endif
#include "Fireworks/Core/interface/FWGeometryTableManagerBase.h"
#include "Fireworks/Core/interface/FWGeometryTableViewBase.h"
#include "Fireworks/Core/interface/FWGeometryTableViewManager.h"
#include "Fireworks/Core/src/FWColorBoxIcon.h"
#include "Fireworks/TableWidget/interface/GlobalContexts.h"
#include "Fireworks/TableWidget/src/FWTabularWidget.h"
#include "Fireworks/Core/interface/fwLog.h"

#include "TMath.h"
#include "TGeoVolume.h"
#include "TGeoMatrix.h"
#include "TGeoShape.h"
#include "TGeoBBox.h"
#include "TGeoMatrix.h"

#include "TGFrame.h"
#include "TEveUtil.h"


const char* FWGeometryTableManagerBase::NodeInfo::name() const
{
   return m_node->GetName();
}


FWGeometryTableManagerBase::ColorBoxRenderer::ColorBoxRenderer():
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

FWGeometryTableManagerBase::ColorBoxRenderer::~ColorBoxRenderer()
{
   gClient->GetResourcePool()->GetGCPool()->FreeGC(m_colorContext->GetGC());
}

void FWGeometryTableManagerBase::ColorBoxRenderer::setData(Color_t c, bool s)
{
   m_color = gVirtualX->GetPixel(c);
   m_isSelected = s;
}


void FWGeometryTableManagerBase::ColorBoxRenderer::draw(Drawable_t iID, int iX, int iY, unsigned int iWidth, unsigned int iHeight)
{
   iX -= FWTabularWidget::kTextBuffer;
   iY -= FWTabularWidget::kTextBuffer;
   iWidth += 2*FWTabularWidget::kTextBuffer;
   iHeight += 2*FWTabularWidget::kTextBuffer;

   m_colorContext->SetFillStyle(kFillSolid);
   Pixel_t baq =  m_colorContext->GetForeground();
   m_colorContext->SetForeground(m_color);
   gVirtualX->FillRectangle(iID, m_colorContext->GetGC(), iX, iY, iWidth, iHeight);

   if (m_isSelected)
   {
      m_colorContext->SetFillStyle(kFillOpaqueStippled);
      gVirtualX->FillRectangle(iID, m_colorContext->GetGC(), iX, iY, iWidth, iHeight);
   }
   m_colorContext->SetForeground(baq);
}

//==============================================================================
//==============================================================================
//
// class FWGeometryTableManagerBase
//
//==============================================================================
//==============================================================================

FWGeometryTableManagerBase::FWGeometryTableManagerBase()
   :   
   m_highlightIdx(-1),
   m_selectedIdx(0),
   m_selectedColumn(-1),
   m_levelOffset(0)
{ 
   m_colorBoxRenderer.m_width  =  50;
   m_colorBoxRenderer.m_height =  m_renderer.height();

   GCValues_t gval;
   gval.fMask = kGCForeground | kGCBackground | kGCStipple | kGCFillStyle  | kGCGraphicsExposures;
   gval.fForeground = gVirtualX->GetPixel(kGray);//gClient->GetResourcePool()->GetFrameHiliteColor();
   gval.fBackground = gVirtualX->GetPixel(kWhite);//gClient->GetResourcePool()->GetFrameBgndColor();
   gval.fFillStyle  = kFillOpaqueStippled; // kFillTiled;
   gval.fStipple    = gClient->GetResourcePool()->GetCheckeredBitmap();
   gval.fGraphicsExposures = kFALSE;
   m_highlightContext = gClient->GetGC(&gval, kTRUE);

   m_renderer.setHighlightContext( m_highlightContext);
}

FWGeometryTableManagerBase::~FWGeometryTableManagerBase()
{
}


int FWGeometryTableManagerBase::unsortedRowNumber(int unsorted) const
{
   return unsorted;
}

int FWGeometryTableManagerBase::numberOfRows() const 
{
   return m_row_to_index.size();
}

int FWGeometryTableManagerBase::numberOfColumns() const 
{
   return kNumCol;
}
   

std::vector<std::string> FWGeometryTableManagerBase::getTitles() const 
{
   std::vector<std::string> returnValue;
   returnValue.reserve(numberOfColumns());

   returnValue.push_back("Name");
   returnValue.push_back("Color");
   returnValue.push_back("RnrSelf");
   returnValue.push_back("RnrChildren");
   returnValue.push_back("Material");
   return returnValue;
}
  
void FWGeometryTableManagerBase::setSelection (int row, int column, int mask) 
{
   changeSelection(row, column);
}

const std::string FWGeometryTableManagerBase::title() const 
{
   return "Geometry";
}

int FWGeometryTableManagerBase::selectedRow() const 
{
   return m_selectedIdx;
}

int FWGeometryTableManagerBase::selectedColumn() const 
{
   return m_selectedColumn;
}
 
bool FWGeometryTableManagerBase::rowIsSelected(int row) const 
{
   return m_selectedIdx == row;
}

void FWGeometryTableManagerBase::changeSelection(int iRow, int iColumn)
{     
   if (iRow < 0) return; 

   //   m_selectedRow = iRow;
    m_selectedColumn = iColumn;

   visualPropertiesChanged();
}    

void  FWGeometryTableManagerBase::setBackgroundToWhite(bool iToWhite )
{
   if(iToWhite) {
      m_renderer.setGraphicsContext(&TGFrame::GetBlackGC());
   } else {
      m_renderer.setGraphicsContext(&TGFrame::GetWhiteGC());
   }
   m_renderer.setBlackIcon(iToWhite);
}


FWGeometryTableManagerBase::ESelectionState FWGeometryTableManagerBase::nodeSelectionState(int idx) const
{
   if (  m_selectedIdx == idx )
      return kSelected;
   if  (  m_highlightIdx == idx )
      return kHighlighted;

   return kNone;
}


//______________________________________________________________________________

FWTableCellRendererBase* FWGeometryTableManagerBase::cellRenderer(int iSortedRowNumber, int iCol) const
{
   FWTextTreeCellRenderer* renderer = &m_renderer;
   if (m_row_to_index.empty()) return renderer;


   int unsortedRow =  m_row_to_index[iSortedRowNumber];
   if (unsortedRow < 0) printf("!!!!!!!!!!!!!!!! error %d %d \n",unsortedRow,  iSortedRowNumber);


   const NodeInfo& data = m_entries[unsortedRow];
   TGeoNode& gn = *data.m_node;

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

   if (iCol == kName)
   {
      renderer->setData(cellName(data), isSelected); 


      renderer->setIsParent(nodeIsParent(data));

      renderer->setIsOpen( data.testBit(FWGeometryTableManagerBase::kExpanded));

      int level = data.m_level - m_levelOffset;
      if (nodeIsParent(data))
         renderer->setIndentation(20*level);
      else
         renderer->setIndentation(20*level + FWTextTreeCellRenderer::iconWidth());

      return renderer;
   }
   else
   {
      // printf("title %s \n",data.m_node->GetTitle() );
      renderer->setIsParent(false);
      renderer->setIndentation(0);
      if (iCol == kColor)
      {
         // m_colorBoxRenderer.setData(data.m_node->GetVolume()->GetLineColor(), isSelected);
         m_colorBoxRenderer.setData(data.m_color, isSelected);
         return  &m_colorBoxRenderer;
      }
      else if (iCol == kVisSelf )
      {
         renderer->setData(getVisibility(data)  ? "On" : "-",  isSelected );
         return renderer;
      }
      else if (iCol == kVisChild )
      {
         renderer->setData( getVisibilityChld(data) ? "On" : "-",  isSelected);
         return renderer;
      }
      else if (iCol == kMaterial )
      { 
         renderer->setData( gn.GetVolume()->GetMaterial()->GetName(),  isSelected);
         return renderer;
      }
      else
      {  renderer->setData("ERROR", false);
         return renderer;
      }

   }
}

//______________________________________________________________________________
bool FWGeometryTableManagerBase::firstColumnClicked(int row, int xPos)
{
   if (row == -1)
      return false;

   int idx = rowToIndex()[row];
   // printf("click %s \n", m_entries[idx].name());

   int off = 0;
   if (idx >= 0)
      off = (m_entries[idx].m_level - m_levelOffset)* 20;

   //   printf("compare %d %d level %d\n" , xPos, off, idx);
   if (xPos >  off &&  xPos < (off + 20))
   {
      m_entries[idx].switchBit(kExpanded);
 
      recalculateVisibility();
      dataChanged();
      visualPropertiesChanged();
      return false;
   }

   m_selectedIdx = idx;
   return true;
}


 

//______________________________________________________________________________

void FWGeometryTableManagerBase::getNodeMatrix(const NodeInfo& data, TGeoHMatrix& mtx) const
{
   // utility used by browser and FWGeoNode
   //   printf("================ FWGeometryTableManagerBase::getNodeMatri \n");
   int pIdx  = data.m_parent;

   while (pIdx > 0)
   {
      // printf("%s [%d]\n",m_entries.at(pIdx).name(), m_entries.at(pIdx).m_level );
      mtx.MultiplyLeft(m_entries.at(pIdx).m_node->GetMatrix());
      pIdx = m_entries.at(pIdx).m_parent;
   }

   //   printf("right %s [%d]\n",data.name(), data.m_level );
   mtx.Multiply(data.m_node->GetMatrix());
}

//______________________________________________________________________________
void FWGeometryTableManagerBase::redrawTable(bool setExpand) 
{
   std::cerr << "GeometryTableManagerBase::redrawTable ------------------------------------- \n";
   if (m_entries.empty()) return;

   if (setExpand) checkExpandLevel();


   changeSelection(0, 0);

   recalculateVisibility();


   dataChanged();
   visualPropertiesChanged();
}

//==============================================================================

FWGeometryTableManagerBase::Entries_i FWGeometryTableManagerBase::refSelected()
{
   Entries_i i = m_entries.begin();
   if (m_selectedIdx > 0)
      std::advance(i,m_selectedIdx );
   return  i; //m_entries[m_selectedIdx];
}

//______________________________________________________________________________

void FWGeometryTableManagerBase::getNodePath(int idx, std::string& path) const
{
   std::vector<std::string> relPath;
   while(idx >= 0)
   { 
      relPath.push_back( m_entries[idx].name());
      // printf("push %s \n",m_entries[idx].name() );
      idx  =  m_entries[idx].m_parent;
   }

   size_t ns = relPath.size();
   for (size_t i = 1; i < ns; ++i )
   {
      path +="/";
      path += relPath[ns-i -1];
      // printf("push_back add to path %s\n", path.c_str());
   }
}


//______________________________________________________________________________

void  FWGeometryTableManagerBase::topGeoNodeChanged(int idx)
{
   // cached 
   if (idx >= 0)
      m_levelOffset = m_entries[idx].m_level;
   else
      m_levelOffset = 0;

}

//______________________________________________________________________________

void FWGeometryTableManagerBase::setDaughtersSelfVisibility(bool v)
{
   int dOff = 0;
   TGeoNode* parentNode = m_entries[m_selectedIdx].m_node;
   int nD = parentNode->GetNdaughters();
   for (int n = 0; n != nD; ++n)
   {
      int idx = m_selectedIdx + 1 + n + dOff;
      NodeInfo& data = m_entries[idx];

      setVisibility(data, v);
      setVisibilityChld(data, v);

      FWGeometryTableManagerBase::getNNodesTotal(parentNode->GetDaughter(n), dOff);
   }
}


//______________________________________________________________________________
void FWGeometryTableManagerBase::setVisibility(NodeInfo& data, bool x)
{
   data.setBitVal(kVisNodeSelf, x);
}


void FWGeometryTableManagerBase::setVisibilityChld(NodeInfo& data, bool x)
{
   data.setBitVal(kVisNodeChld, x);
}

//______________________________________________________________________________

bool  FWGeometryTableManagerBase::getVisibility(const NodeInfo& data) const
{
   return  data.testBit(kVisNodeSelf);   
}

bool  FWGeometryTableManagerBase::getVisibilityChld(const NodeInfo& data) const
{

   return  data.testBit(kVisNodeChld);   
}

//______________________________________________________________________________

void  FWGeometryTableManagerBase::checkExpandLevel()
{
   // check expand state
   int ae = /*m_browser->getAutoExpand()*/ 1 +  m_levelOffset;
   for (Entries_i i = m_entries.begin(); i != m_entries.end(); ++i)
   {
      if (i->m_level  < ae)
         i->setBit(kExpanded);
      else
         i->resetBit(kExpanded);
   } 
}
