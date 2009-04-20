// $Id: FWTableViewTableManager.cc,v 1.1.2.2 2009/04/20 16:33:36 jmuelmen Exp $

#include <math.h>
#include "TClass.h"
#include "TGClient.h"
#include "Fireworks/Core/interface/FWTableViewTableManager.h"
#include "Fireworks/Core/interface/FWTableViewManager.h"
#include "Fireworks/Core/interface/FWTableView.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/TableWidget/interface/FWTableWidget.h"

FWTableViewTableManager::FWTableViewTableManager (const FWTableView *view)
     : m_view(view),
       m_graphicsContext(0),
       m_renderer(0)
{
     GCValues_t gc = *(m_view->m_tableWidget->GetWhiteGC().GetAttributes());
     m_graphicsContext = gClient->GetResourcePool()->GetGCPool()->GetGC(&gc,kTRUE);
//      m_graphicsContext = (TGGC *)&FWTextTableCellRenderer::getDefaultGC();
     m_renderer = new FWTextTableCellRenderer(m_graphicsContext,
					      &FWTextTableCellRenderer::getHighlightGC(),
					      FWTextTableCellRenderer::kJustifyRight);
}

FWTableViewTableManager::~FWTableViewTableManager ()
{

}

int FWTableViewTableManager::numberOfRows() const
{
     if (m_view->item() != 0)
	  return m_view->item()->size();
     else return 0;
}

int FWTableViewTableManager::numberOfColumns() const
{
     return m_view->m_evaluators.size();
}

std::vector<std::string> FWTableViewTableManager::getTitles () const
{
     unsigned int n = numberOfColumns();
     std::vector<std::string> ret;
     ret.reserve(n);
     for (unsigned int i = 0; i < n; ++i) {
	  ret.push_back((*m_view->m_manager->tableFormats(m_view->item()->modelType()->GetName())).second[i].name);
// 	  printf("%s\n", ret.back().c_str());
     }
     return ret;
}

int FWTableViewTableManager::unsortedRowNumber(int iSortedRowNumber) const
{
     // no sort for now
     return iSortedRowNumber;
}

FWTableCellRendererBase *FWTableViewTableManager::cellRenderer(int iSortedRowNumber, int iCol) const
{
     if (m_view->item()->modelData(iSortedRowNumber) != 0 &&
	 iCol < m_view->m_evaluators.size()) {
	  double ret;
	  try {
// 	       printf("iCol %d, size %d\n", iCol, m_view->m_evaluators.size());
	       ret = m_view->m_evaluators[iCol].evalExpression(m_view->item()->modelData(iSortedRowNumber));
	  } catch (...) {
	       printf("something bad happened\n");
	       ret = -999;
	  }
	  int precision = (*m_view->m_manager->tableFormats(m_view->item()->modelType()->GetName())).second[iCol].precision;
	  char s[100];
	  char fs[100];
	  switch (precision) {
	  case FWTableViewManager::TableEntry::INT:
	       snprintf(s, sizeof(s), "%d", int(rint(ret)));
	       break;
	  case FWTableViewManager::TableEntry::INT_HEX:
	       snprintf(s, sizeof(s), "0x%x", int(rint(ret)));
	       break;
	  case FWTableViewManager::TableEntry::BOOL:
	       snprintf(s, sizeof(s), int(rint(ret)) != 0 ? "true" : "false");
	       break;
	  default: 
	       snprintf(fs, sizeof(fs), "%%.%df", precision);
	       snprintf(s, sizeof(s), fs, ret);
	       break;
	  }
 	  m_graphicsContext->
 	       SetForeground(gVirtualX->GetPixel(m_view->item()->modelInfo(iSortedRowNumber).
 						 displayProperties().color()));
 	  m_renderer->setGraphicsContext(m_graphicsContext);
	  m_renderer->setData(s, m_view->item()->modelInfo(iSortedRowNumber).isSelected());
// 			      not m_view->item()->modelInfo(iSortedRowNumber).
// 			      displayProperties().isVisible());
     } else { 
	  m_renderer->setData("invalid", false);
     }
     return m_renderer;
}

void FWTableViewTableManager::implSort(int iCol, bool iSortOrder)
{
     static const bool sort_up = true;
     printf("sorting %s\n", iSortOrder ? "up" : "down");
//      if (iSortOrder == sort_up) {
// 	  std::map<double,int, std::greater<double> > s;
// 	  doSort(*m_collection, *(m_valueGetters[iCol]), s, m_sortedToUnsortedIndicies);
//      } else {
// 	  std::map<double,int, std::less<double> > s;
// 	  doSort(*m_collection, *(m_valueGetters[iCol]), s, m_sortedToUnsortedIndicies);
//      }
}
