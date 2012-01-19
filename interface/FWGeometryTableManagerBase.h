#ifndef Fireworks_Core_FWGeometryTableManagerBase_h
#define Fireworks_Core_FWGeometryTableManagerBase_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWGeometryTableManagerBase
// 
/**\class FWGeometryTableManagerBase FWGeometryTableManagerBase.h Fireworks/Core/interface/FWGeometryTableManagerBase.h

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Alja Mrak-Tadel, Matevz Tadel
//         Created:  Thu Jan 27 14:50:40 CET 2011
// $Id: FWGeometryTableManagerBase.h,v 1.1.2.3 2012/01/18 02:38:36 amraktad Exp $
//

#include <sigc++/sigc++.h>
#include <boost/tr1/unordered_map.hpp>

#include "Fireworks/Core/interface/FWGeometryTableViewBase.h"

#include "Fireworks/TableWidget/interface/FWTableManagerBase.h"
#include "Fireworks/TableWidget/interface/FWTextTreeCellRenderer.h"
#include "Fireworks/TableWidget/interface/FWTextTableCellRenderer.h"
#include "Fireworks/TableWidget/interface/FWTableCellRendererBase.h"

#include "TGeoNode.h"
#include "TGeoVolume.h"

class FWTableCellRendererBase;
// class FWGeometryTableViewBase;
//class TGeoManager;
class TGeoNode;
class TEvePointSet;

class FWGeometryTableManagerBase : public FWTableManagerBase
{
   friend class FWGeometryTableViewBase;

public:
   enum   ECol   { kName, kColor,  kVisSelf, kVisChild, kMaterial, kNumCol };
   enum   ESelectionState { kNone, kSelected, kHighlighted, kFiltered };

   enum Bits
   {
      kExpanded        =  BIT(0),
      kMatches         =  BIT(1),
      kChildMatches    =  BIT(2),
      kFilterCached    =  BIT(3),

      kVisNodeSelf     =  BIT(4),
      kVisNodeChld     =  BIT(5),

      kFlag1            =  BIT(6),
      kFlag2            =  BIT(7)
   };

   struct NodeInfo
   {
      NodeInfo():m_node(0), m_parent(-1), m_color(0), m_level(-1), 
                 m_flags(kVisNodeSelf|kVisNodeChld) {}  

      NodeInfo(TGeoNode* n, Int_t p, Color_t col, Char_t l, UChar_t f = kVisNodeSelf|kVisNodeChld ):m_node(n), m_parent(p), m_color(col), m_level(l), 
                 m_flags(f) {}  

      TGeoNode*   m_node;
      Int_t       m_parent;
      Color_t     m_color;
      UChar_t     m_level;
      UChar_t     m_flags;

      const char* name() const;
      //  const char* nameIndent() const;

      void setBit(UChar_t f)    { m_flags  |= f;}
      void resetBit(UChar_t f)  { m_flags &= ~f; }
      void setBitVal(UChar_t f, bool x) { x ? setBit(f) : resetBit(f);}
 
      bool testBit(UChar_t f) const  { return (m_flags & f) == f; }
      bool testBitAny(UChar_t f) const  { return (m_flags & f) != 0; }

      void switchBit(UChar_t f) { testBit(f) ? resetBit(f) : setBit(f); }
   };

   struct Match
   {
      bool m_matches;
      bool m_childMatches;
      Match() : m_matches(false), m_childMatches(false) {}
   
      bool accepted() { return m_matches || m_childMatches; }
   };

   typedef std::vector<NodeInfo> Entries_v;
   typedef Entries_v::iterator Entries_i;
   
   typedef boost::unordered_map<TGeoVolume*, Match>  Volumes_t;
   typedef Volumes_t::iterator               Volumes_i; 

   int m_highlightIdx;

   //private: 
   // AMT: this could be a common base class with FWCollectionSummaryModelCellRenderer ..
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

protected:
   virtual bool nodeIsParent(const NodeInfo&) const { return false; }
   virtual ESelectionState nodeSelectionState(int idx) const;

public:
   FWGeometryTableManagerBase();
   virtual ~FWGeometryTableManagerBase();
   //   virtual std::string& cellName(const NodeInfo& ) const { return &std::string("ddd");} 
   virtual const char* cellName(const NodeInfo& ) const { return 0;} 

   // virtual functions of FWTableManagerBase
   
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

   void setSelection(int row, int column, int mask); 
   virtual void implSort(int, bool) {}

   bool nodeImported(int idx) const;
   // geo stuff

   NodeInfo* getSelected();

   Entries_v& refEntries() {return m_entries;}

   void loadGeometry( TGeoNode* , TObjArray*);

   void setBackgroundToWhite(bool);
   void getNodePath(int, std::string&) const;

   int getLevelOffset() const { return m_levelOffset; }

   void setDaughtersSelfVisibility(bool);

   void getNodeMatrix(const NodeInfo& nodeInfo, TGeoHMatrix& mat) const;

   virtual void setVisibility(NodeInfo& nodeInfo, bool );
   virtual void setVisibilityChld(NodeInfo& nodeInfo, bool);

   virtual bool getVisibilityChld(const NodeInfo& nodeInfo) const;
   virtual bool getVisibility (const NodeInfo& nodeInfo) const;


   static  void getNNodesTotal(TGeoNode* geoNode, int& off);

   // private:
   FWGeometryTableManagerBase(const FWGeometryTableManagerBase&); // stop default
   const FWGeometryTableManagerBase& operator=(const FWGeometryTableManagerBase&); // stop default

   
   bool firstColumnClicked(int row, int xPos);
   void changeSelection(int iRow, int iColumn);
   void redrawTable(bool setExpand = false);

   virtual void recalculateVisibility() = 0;

   // signal callbacks
   void topGeoNodeChanged(int);

   void checkExpandLevel();
   // ---------- member data --------------------------------
   
   
   // table stuff
   mutable TGGC* m_highlightContext; 
   mutable FWTextTreeCellRenderer m_renderer;  
   mutable ColorBoxRenderer       m_colorBoxRenderer;  

   std::vector<int>  m_row_to_index;
   int               m_selectedIdx;
   int               m_selectedColumn;
   
   Entries_v          m_entries;


   int m_levelOffset;
};



inline void FWGeometryTableManagerBase::getNNodesTotal(TGeoNode* geoNode, int& off)
{   
   int nD =  geoNode->GetNdaughters();
   off += nD;
   for (int i = 0; i < nD; ++i )
   {
      getNNodesTotal(geoNode->GetDaughter(i), off);
   }
}

#endif
