#include "Fireworks/Core/interface/FWGeometryTable.h"
#include "Fireworks/Core/interface/FWGUIManager.h"
#include "Fireworks/Core/src/FWDialogBuilder.h"

#include "Fireworks/TableWidget/interface/FWTableManagerBase.h"
#include "Fireworks/TableWidget/interface/FWTableWidget.h"
#include "Fireworks/TableWidget/src/FWTabularWidget.h"
#include "Fireworks/TableWidget/interface/FWTextTableCellRenderer.h"
#include "Fireworks/TableWidget/interface/FWTextTreeCellRenderer.h"
#include "Fireworks/TableWidget/interface/GlobalContexts.h"

#include "TFile.h"
#include "TGFileDialog.h"
#include "TGWindow.h"
#include "TGeoManager.h"

#include <iostream>

class FWGeometryTableManager : public FWTableManagerBase
{
public:
  FWGeometryTableManager()
    : m_selectedRow(-1)
    { 
      m_topVolumeRenderer.setGraphicsContext(&fireworks::boldGC());
      reset();
    }

  virtual void implSort(int, bool)
    {
      return;
    }

   virtual int unsortedRowNumber(int unsorted) const
    {
      return unsorted;
    }

   virtual int numberOfRows() const 
    {
      return m_row_to_index.size();
    }

   virtual int numberOfColumns() const 
    {
      return 2;
    }
   
  virtual std::vector<std::string> getTitles() const 
    {
      std::vector<std::string> returnValue;
      returnValue.reserve(numberOfColumns());
      returnValue.push_back("Label");
      returnValue.push_back("Value");
      return returnValue;
    }
  
  virtual FWTableCellRendererBase* cellRenderer(int iSortedRowNumber, int iCol) const
    {
      if (static_cast<int>(m_row_to_index.size()) <= iSortedRowNumber)
      {
        m_renderer.setData(std::string(), false);
        return &m_renderer;
      }         

      return &m_renderer;
   }

   void setSelection (int row, int column, int mask) 
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

  virtual const std::string title() const 
    {
      return "Geometry";
    }

   int selectedRow() const 
    {
      return m_selectedRow;
    }

   int selectedColumn() const 
    {
      return m_selectedColumn;
    }
 
   virtual bool rowIsSelected(int row) const 
    {
      return m_selectedRow == row;
    }

  void reset() 
    {
      changeSelection(-1, -1);
      //recalculateVisibility();
      dataChanged();
      visualPropertiesChanged();
    }

  std::vector<int> &rowToIndex() { return m_row_to_index; }
  sigc::signal<void,int,int> indexSelected_;

private:
  void changeSelection(int iRow, int iColumn)
    {      
      if (iRow == m_selectedRow && iColumn == m_selectedColumn)
        return;
      
      m_selectedRow = iRow;
      m_selectedColumn = iColumn;

      indexSelected_(iRow, iColumn);
      visualPropertiesChanged();
    }    

  std::vector<int>  m_row_to_index;
  int               m_selectedRow;
  int               m_selectedColumn;
  
  mutable FWTextTreeCellRenderer m_renderer;         
  mutable FWTextTreeCellRenderer m_topVolumeRenderer;  
};

FWGeometryTable::FWGeometryTable(FWGUIManager *guiManager)
  : TGMainFrame(gClient->GetRoot(), 400, 600),
    m_guiManager(guiManager),
    m_geometryTable(new FWGeometryTableManager()),
    m_geometryFile(0),
    m_fileOpen(0),
    m_topNode(0),
    m_topVolume(0)
{
  std::cout<<"FWGeometryTable::FWGeometryTable(FWGUIManager *guiManager) in"<<std::endl;

  gVirtualX->SelectInput(GetId(), kKeyPressMask | kKeyReleaseMask | kExposureMask |
                         kPointerMotionMask | kStructureNotifyMask | kFocusChangeMask |
                         kEnterWindowMask | kLeaveWindowMask);
  this->Connect("CloseWindow()","FWGeometryTable",this,"windowIsClosing()");

  FWDialogBuilder builder(this);
  builder.indent(4)
    .spaceDown(10)
    .addTable(m_geometryTable, &m_tableWidget).expand(true, true)
    .addTextButton("Open geometry file", &m_fileOpen);       
    
  m_tableWidget->SetBackgroundColor(0xffffff);
  m_tableWidget->SetLineSeparatorColor(0x000000);
  m_tableWidget->SetHeaderBackgroundColor(0xececec);
  m_tableWidget->Connect("cellClicked(Int_t,Int_t,Int_t,Int_t,Int_t,Int_t)",
                         "FWGeometryTable",this,
                         "cellClicked(Int_t,Int_t,Int_t,Int_t,Int_t,Int_t)");

  m_fileOpen->Connect("Clicked()", "FWGeometryTable", this, "openFile()");
  m_fileOpen->SetEnabled(true);

  MapSubwindows();
  Layout();

  std::cout<<"FWGeometryTable::FWGeometryTable(FWGUIManager *guiManager) out"<<std::endl;
}

FWGeometryTable::~FWGeometryTable()
{}

void 
FWGeometryTable::cellClicked(Int_t iRow, Int_t iColumn, Int_t iButton, Int_t iKeyMod, Int_t, Int_t)
{
   if (iButton != kButton1)
      return;   
}

void
FWGeometryTable::windowIsClosing()
{
  UnmapWindow();
  DontCallClose();
}

void
FWGeometryTable::newIndexSelected(int iSelectedRow, int iSelectedColumn)
{
   if (iSelectedRow == -1)
      return;
}

void 
FWGeometryTable::readFile()
{
  if ( ! m_geometryFile )
  {
    std::cout<<"FWGeometryTable::readFile() no geometry file!"<<std::endl;
    return;
  }
  
  m_geometryFile->ls();
      
  TGeoManager* m_geoManager = (TGeoManager*) m_geometryFile->Get("cmsGeo;1");

  /*
  TObjArray* nodes     = m_geoManager->GetListOfNodes();
  TObjArray* volumes   = m_geoManager->GetListOfVolumes();
  TObjArray* shapes    = m_geoManager->GetListOfShapes();

  nodes->Print();
  volumes->Print();
  shapes->Print();
  */

  m_topVolume = m_geoManager->GetTopVolume();
  m_topVolume->Print();

  //m_topNode   = m_geoManager->GetTopNode();
  //m_topNode->Print();

  int n_daughters = m_topVolume->GetNdaughters();
  std::cout<<"# daughters: "<< n_daughters <<std::endl;

  for ( size_t d = 0, de = n_daughters; d != de; ++d )
  {
    std::cout<< d <<" ";
    m_topVolume->GetNode(d)->Print();
  }
}

void
FWGeometryTable::openFile()
{
  std::cout<<"FWGeometryTable::openFile()"<<std::endl;
  
  const char* kRootType[] = {"ROOT files","*.root", 0, 0};
  TGFileInfo fi;
  fi.fFileTypes = kRootType;
 
  m_guiManager->updateStatus("opening geometry file...");

  new TGFileDialog(gClient->GetDefaultRoot(), 
                   (TGWindow*) m_guiManager->getMainFrame(), kFDOpen, &fi);

  m_guiManager->updateStatus("loading geometry file...");
  
  if ( fi.fFilename ) 
    m_geometryFile = new TFile(fi.fFilename, "READ");

  m_guiManager->clearStatus();

  readFile();
}
