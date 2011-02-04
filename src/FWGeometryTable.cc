
#include <boost/shared_ptr.hpp>
#include "Fireworks/Core/interface/FWGeometryTable.h"
#include "Fireworks/Core/interface/FWGUIManager.h"
#include "Fireworks/Core/src/FWDialogBuilder.h"
#include "Fireworks/TableWidget/interface/FWTableWidget.h"
#include "Fireworks/Core/interface/FWParameterSetterBase.h"

#include "Fireworks/Core/interface/FWGeometryTableManager.h"


#include "TFile.h"
#include "TGFileDialog.h"
#include "TGeoNode.h"
#include "TGWindow.h"

#include <iostream>
 std::vector<boost::shared_ptr<FWParameterSetterBase> > m_setters;
FWGeometryTable::FWGeometryTable(FWGUIManager *guiManager)
   : TGMainFrame(gClient->GetRoot(), 600, 500),
     m_mode(this, "Mode", 0l, 0l, 2l),
     m_filter(this,"Filter Materials",std::string()),
     m_guiManager(guiManager),
     m_geometryTable(new FWGeometryTableManager()),
     m_geometryFile(0),
     m_fileOpen(0),
     m_topNode(0),
     m_topVolume(0),
     m_level(-1)
{
   m_mode.addEntry(0, "Node");
   m_mode.addEntry(1, "Volume");
  
 




   gVirtualX->SelectInput(GetId(), kKeyPressMask | kKeyReleaseMask | kExposureMask |
                          kPointerMotionMask | kStructureNotifyMask | kFocusChangeMask |
                          kEnterWindowMask | kLeaveWindowMask);
   this->Connect("CloseWindow()","FWGeometryTable",this,"windowIsClosing()");

   boost::shared_ptr<FWParameterSetterBase> ptr( FWParameterSetterBase::makeSetterFor((FWParameterBase*)&m_mode) );
   ptr->attach((FWParameterBase*)&m_mode, this);
   TGFrame* pframe = ptr->build(this);
   AddFrame(pframe, new TGLayoutHints(kLHintsExpandX));
   m_setters.push_back(ptr);

   FWDialogBuilder builder(this);
   builder.indent(4)
      .spaceDown(10)
      .addTextButton("Open geometry file", &m_fileOpen) 

   
      .addLabel("Material:").floatLeft(4).expand(false, false)
      .addTextEntry("", &m_search).expand(true, false)
      .spaceDown(10)
      .addTable(m_geometryTable, &m_tableWidget).expand(true, true);


  

   openFile();
    
   m_tableWidget->SetBackgroundColor(0xffffff);
   m_tableWidget->SetLineSeparatorColor(0x000000);
   m_tableWidget->SetHeaderBackgroundColor(0xececec);
   m_tableWidget->Connect("cellClicked(Int_t,Int_t,Int_t,Int_t,Int_t,Int_t)",
                          "FWGeometryTable",this,
                          "cellClicked(Int_t,Int_t,Int_t,Int_t,Int_t,Int_t)");

   m_fileOpen->Connect("Clicked()","FWGeometryTable",this,"openFile()");
   m_fileOpen->SetEnabled();

   SetWindowName("Geometry browser");
   MapSubwindows();
   Layout();
}

FWGeometryTable::~FWGeometryTable()
{}

void 
FWGeometryTable::cellClicked(Int_t iRow, Int_t iColumn, Int_t iButton, Int_t iKeyMod, Int_t, Int_t)
{
   if (iButton != kButton1)
      return;   

   if (iColumn == 0)
      m_geometryTable->setExpanded(iRow);

   m_geometryTable->setSelection(iRow, iColumn, iKeyMod);
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

  m_geometryTable->dataChanged();
}

void 
FWGeometryTable::handleNode(const TGeoNode* node)
{
  for ( size_t d = 0, de = node->GetNdaughters(); d != de; ++d )
  {
    handleNode(node->GetDaughter(d));
  }
}

void 
FWGeometryTable::readFile()
{
   if ( ! m_geometryFile )
   {
      std::cout<<"FWGeometryTable::readFile() no geometry file!"<< std::endl;
      return;
   }
  
   m_geometryFile->ls();
      
   TGeoManager* m_geoManager = (TGeoManager*) m_geometryFile->Get("cmsGeo;1");


   m_geometryTable->fillNodeInfo(m_geoManager);
}

void
FWGeometryTable::openFile()
{
   std::cout<<"FWGeometryTable::openFile()"<<std::endl;

   if (0)
   {  
      const char* kRootType[] = {"ROOT files","*.root", 0, 0};
      TGFileInfo fi;
      fi.fFileTypes = kRootType;
 
      m_guiManager->updateStatus("opening geometry file...");

      new TGFileDialog(gClient->GetDefaultRoot(), 
                       (TGWindow*) m_guiManager->getMainFrame(), kFDOpen, &fi);

      m_guiManager->updateStatus("loading geometry file...");
      m_geometryFile = new TFile(fi.fFilename, "READ");
   }
   else
   {
      // AMT developing environment
      m_geometryFile = new TFile("cmsSimGeom-14.root", "READ");
   }
   m_guiManager->clearStatus();

   readFile();
}

