#ifndef Fireworks_Core_FWGeometryTableManager_h
#define Fireworks_Core_FWGeometryTableManager_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWGeometryTableManager
// 
/**\class FWGeometryTableManager FWGeometryTableManager.h Fireworks/Core/interface/FWGeometryTableManager.h

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Thomas McCauley, Alja Mrak-Tadel
//         Created:  Thu Jan 27 14:50:40 CET 2011
// $Id: FWGeometryTableManager.h,v 1.1.2.1 2011/02/01 19:00:37 amraktad Exp $
//

#include <sigc++/sigc++.h>

#include "Fireworks/TableWidget/interface/FWTableManagerBase.h"
#include "Fireworks/TableWidget/interface/FWTextTreeCellRenderer.h"

#include "Fireworks/TableWidget/interface/FWTextTableCellRenderer.h"
#include "Fireworks/TableWidget/interface/FWTableCellRendererBase.h"

class FWTableCellRendererBase;
class TGeoManager;
class TGeoNode;
class FWColorBoxIcon;

class FWGeometryTableManager : public FWTableManagerBase
{
   friend class FWGeometryTable;

   struct NodeInfo
   {
      NodeInfo():m_node(0), m_parent(-1), m_level(-1), m_imported(false), m_visible(false), m_expanded(false)
      {}  

      TGeoNode* m_node;
      int       m_parent;
      int       m_level;
      bool      m_imported;
      bool      m_visible;
      bool      m_expanded;

      const char* name() const;
   };

   // AMT could be a common base class with FWCollectionSummaryModelCellRenderer ..
   class ColorBoxRenderer : public FWTableCellRendererBase
   { 
   public:
      ColorBoxRenderer();
      virtual ~ColorBoxRenderer();
  
      virtual UInt_t width() const { return m_width; }
      virtual UInt_t height() const { return m_height; }
      void setData(Color_t c, bool);
      virtual void draw(Drawable_t iID, int iX, int iY, unsigned int iWidth, unsigned int iHeight);

      UInt_t  m_width;
      UInt_t  m_height;
      Pixel_t m_color;      
      bool    m_isSelected;
      TGGC*   m_colorContext;

   };

public:
   FWGeometryTableManager();
   virtual ~FWGeometryTableManager() {}

   // const functions
   virtual int unsortedRowNumber(int unsorted) const;
   virtual int numberOfRows() const;
   virtual int numberOfColumns() const;
   virtual std::vector<std::string> getTitles() const;
   virtual FWTableCellRendererBase* cellRenderer(int iSortedRowNumber, int iCol) const;

   virtual const std::string title() const;

   int selectedRow() const;
   int selectedColumn() const;
   virtual bool rowIsSelected(int row) const;

   std::vector<int> rowToIndex() { return m_row_to_index; }


protected:
   void setExpanded(int row);
   void setSelection(int row, int column, int mask); 

   virtual void implSort(int, bool); 

private:
   enum   ECol {kName, kColor,  kVisSelf, kVisChild, kMaterial, kPosition, kBBoxSize, kNumCol };

   typedef std::vector<NodeInfo> Entries_v;
   typedef Entries_v::iterator Entries_i;

   FWGeometryTableManager(const FWGeometryTableManager&); // stop default
   const FWGeometryTableManager& operator=(const FWGeometryTableManager&); // stop default

   void reset();
   void recalculateVisibility();
   void changeSelection(int iRow, int iColumn);
   void fillNodeInfo(TGeoManager* geoManager);
   void importChildren(int row, int level = -1);
   void checkHierarchy();

   // ---------- member data --------------------------------
   std::vector<int>  m_row_to_index;
   int               m_selectedRow;
   int               m_selectedColumn;
  
   Entries_v         m_entries;

   mutable FWTextTreeCellRenderer m_renderer;  
   mutable ColorBoxRenderer       m_colorBoxRenderer;         

   sigc::signal<void,int,int> indexSelected_;
};

#endif
