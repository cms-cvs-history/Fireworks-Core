#ifndef Fireworks_Core_FWGeometryTable_h
#define Fireworks_Core_FWGeometryTable_h

#include "TGFrame.h"

#include "Fireworks/Core/interface/FWConfigurableParameterizable.h"
#include "Fireworks/Core/interface/FWStringParameter.h"
#include "Fireworks/Core/interface/FWEnumParameter.h"
#include "Fireworks/Core/interface/FWParameterSetterEditorBase.h"

class FWGUIManager;
class FWTableWidget;
class FWGeometryTableManager;
class TFile;
class TGTextButton;
class TGeoNode;
class TGeoVolume;
class TGTextEntry;

class FWGeometryTable : public TGMainFrame, public FWConfigurableParameterizable, public FWParameterSetterEditorBase
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

   virtual void setFrom(const FWConfiguration&) {}

   // ---------- const member functions --------------------- 

   virtual void addTo(FWConfiguration&) const {}
private:
   FWGeometryTable(const FWGeometryTable&);
   const FWGeometryTable& operator=(const FWGeometryTable&);

   FWEnumParameter         m_mode;
   FWStringParameter       m_filter; 

   FWGUIManager           *m_guiManager;
   FWTableWidget          *m_tableWidget;
   FWGeometryTableManager *m_geometryTable;
   TFile                  *m_geometryFile;
   TGTextButton           *m_fileOpen;
   TGTextEntry            *m_search;

   TGeoNode               *m_topNode;
   TGeoVolume             *m_topVolume;

   int m_level;


   void handleNode(const TGeoNode*);
 
   ClassDef(FWGeometryTable, 0);
};

#endif

