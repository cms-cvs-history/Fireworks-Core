#ifndef Fireworks_Core_FWGeometryTable_h
#define Fireworks_Core_FWGeometryTable_h

#include "TGFrame.h"

#include "Fireworks/Core/interface/FWConfigurableParameterizable.h"
#include "Fireworks/Core/interface/FWStringParameter.h"
#include "Fireworks/Core/interface/FWEnumParameter.h"


class FWGUIManager;
class FWTableWidget;
class FWGeometryTableManager;
class TFile;
class TGTextButton;
class TGeoNode;
class TGeoVolume;
class TGTextEntry;
class TGComboBox;

class FWGeometryTable : public TGMainFrame, public FWConfigurableParameterizable
{
   friend class FWGeometryTableManager;

public:
   enum EMode { kNode, kVolume };

   FWGeometryTable(FWGUIManager*);
   virtual ~FWGeometryTable();
  
   void cellClicked(Int_t iRow, Int_t iColumn, 
                    Int_t iButton, Int_t iKeyMod, 
                    Int_t iGlobalX, Int_t iGlobalY);
  
   void newIndexSelected(int,int);
   void windowIsClosing();
   void updateFilterString(const char *str);   

   void openFile();
   void readFile();
   void modeChanged(Int_t);

   virtual void setFrom(const FWConfiguration&);

   // ---------- const member functions --------------------- 

   virtual void addTo(FWConfiguration&) const;

protected:
   FWEnumParameter         m_mode;
   FWStringParameter       m_searchExp; 

private:
   FWGeometryTable(const FWGeometryTable&);
   const FWGeometryTable& operator=(const FWGeometryTable&);
   FWGUIManager           *m_guiManager;

   FWTableWidget          *m_tableWidget;
   FWGeometryTableManager *m_tableManager;

   TFile                  *m_geometryFile;
   TGTextButton           *m_fileOpen;
   TGTextEntry            *m_search;
   TGComboBox             *m_combo;

   TGeoNode               *m_topNode;
   TGeoVolume             *m_topVolume;

   int m_level;


   void handleNode(const TGeoNode*);
 
   ClassDef(FWGeometryTable, 0);
};

#endif

