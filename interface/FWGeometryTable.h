#ifndef Fireworks_Core_FWGeometryTable_h
#define Fireworks_Core_FWGeometryTable_h

#include "TGFrame.h"

class FWGUIManager;
class FWTableWidget;

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

private:
   FWGeometryTable(const FWGeometryTable&);
   const FWGeometryTable& operator=(const FWGeometryTable&);

  FWGUIManager  *m_guiManager;
  FWTableWidget *m_tableWidget;

  ClassDef(FWGeometryTable, 0);
};

#endif

