#include "Fireworks/Core/interface/FWGeometryTable.h"
#include "Fireworks/Core/interface/FWGUIManager.h"
#include "Fireworks/TableWidget/interface/FWTableWidget.h"

#include "Fireworks/Core/interface/FWGeometryTableManager.h"


#include "TFile.h"
#include "TGFileDialog.h"
#include "TGeoNode.h"
#include "TGComboBox.h"
#include "TGLabel.h"

#include <iostream>

FWGeometryTable::FWGeometryTable(FWGUIManager *guiManager)
   : TGMainFrame(gClient->GetRoot(), 600, 500),
     m_mode(this, "Mode", 0l, 0l, 2l),
     m_searchExp(this,"Filter Materials",std::string()),
     m_guiManager(guiManager),
     m_tableManager(0),
     m_geometryFile(0),
     m_fileOpen(0),
     m_topNode(0),
     m_topVolume(0),
     m_level(-1)
{
   m_tableManager = new FWGeometryTableManager(this);

   this->Connect("CloseWindow()","FWGeometryTable",this,"windowIsClosing()");

   TGCompositeFrame* vf = this;
   
   {
      TGTextButton* m_fileOpen = new TGTextButton (vf, "Open Geometry File");
      vf->AddFrame(m_fileOpen,  new TGLayoutHints(kLHintsExpandX |kLHintsBottom , 2, 2, 2, 2));
      m_fileOpen->Connect("Clicked()","FWGeometryTable",this,"openFile()");
   }

   {
      TGCompositeFrame *frame = new TGHorizontalFrame(vf);
      vf->AddFrame(frame,  new TGLayoutHints(kLHintsExpandX, 2, 2, 4, 2));
      {
         frame->AddFrame(new TGLabel(frame, "Mode:"),  new TGLayoutHints(kLHintsLeft, 2, 4, 0, 0));
         m_combo = new 	TGComboBox(frame);
         m_combo->AddEntry("Node", 0);
         m_combo->AddEntry("Volume", 1);
         m_combo->Resize(100, 20);
         m_combo->Connect("Selected(Int_t)", "FWGeometryTable", this, "modeChanged(Int_t)");
         frame->AddFrame(m_combo);
         m_combo->Select(kNode, false);
      }
      {
         frame->AddFrame(new TGLabel(frame, "FilterMaterials:"),  new TGLayoutHints(kLHintsLeft, 12, 2, 0, 0));
         TGTextEntry* m_search = new TGTextEntry(frame/*,  "FilerMaterials"*/);
         m_search->Resize(200, 20);
         frame->AddFrame(m_search);
         m_search->Connect("TextChanged(const char *)", "FWGeometryTable", 
                           this, "updateFilterString(const char *)");

      }
   }


   m_tableWidget = new FWTableWidget(m_tableManager, vf); 
   vf->AddFrame(m_tableWidget,new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,2,2,2,2));
   m_tableWidget->SetBackgroundColor(0xffffff);
   m_tableWidget->SetLineSeparatorColor(0x000000);
   m_tableWidget->SetHeaderBackgroundColor(0xececec);
   m_tableWidget->Connect("cellClicked(Int_t,Int_t,Int_t,Int_t,Int_t,Int_t)",
                          "FWGeometryTable",this,
                          "cellClicked(Int_t,Int_t,Int_t,Int_t,Int_t,Int_t)");

   openFile();

   SetWindowName("Geometry Browser");
   MapSubwindows();
   Layout();
}

FWGeometryTable::~FWGeometryTable()
{}

void
FWGeometryTable::addTo(FWConfiguration& to) const
{
}

void
FWGeometryTable::setFrom(const FWConfiguration& from)
{
   m_combo->Select(m_mode.value(), false);
}

void 
FWGeometryTable::cellClicked(Int_t iRow, Int_t iColumn, Int_t iButton, Int_t iKeyMod, Int_t, Int_t)
{
   if (iButton != kButton1)
   {
      m_tableManager->setSelection(iRow, iColumn, iKeyMod);
      return;   
   }

   if (iColumn == 0)
   {
      m_tableManager->setExpanded(iRow);
   }
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

  m_tableManager->dataChanged();
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


   m_tableManager->fillNodeInfo(m_geoManager);
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

void
FWGeometryTable::modeChanged(Int_t x)
{
   printf("set mode %d \n", x);
   m_mode.set(x);
   m_tableManager->dataChanged();
}
 

void
FWGeometryTable::updateFilterString(const char *str)
{
   //   m_tableManager->sortWithFilter(str);
   m_searchExp.set(str);
   m_tableManager->dataChanged();
}
