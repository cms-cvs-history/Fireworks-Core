#ifndef Fireworks_Core_FWGeometryTable_h
#define Fireworks_Core_FWGeometryTable_h

#include "TGFrame.h"

class FWGUIManager;
class FWTableWidget;
class FWGeometryTableManager;
class TFile;
class TGTextButton;
class TGeoNode;
class TGeoVolume;

class FWGeometryTable : public TGMainFrame
{
public:
  FWGeometryTable(FWGUIManager*);
  virtual ~FWGeometryTable();
  
  void cellClicked(Int_t iRow, Int_t iColumn, 
                   Int_t iButton, Int_t iKeyMod, 
                   Int_t iGlobalX, Int_t iGlobalY);
  
  void newIndexSelected(int,int);
  void windowIsClosing();

  void openFile();
  void readFile();

private:
  FWGeometryTable(const FWGeometryTable&);
  const FWGeometryTable& operator=(const FWGeometryTable&);

  FWGUIManager           *m_guiManager;
  FWTableWidget          *m_tableWidget;
  FWGeometryTableManager *m_geometryTable;
  TFile                  *m_geometryFile;
  TGTextButton           *m_fileOpen;

  TGeoNode               *m_topNode;
  TGeoVolume             *m_topVolume;

  ClassDef(FWGeometryTable, 0);
};

#endif

