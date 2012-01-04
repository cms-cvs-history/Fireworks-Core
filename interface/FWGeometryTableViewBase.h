#ifndef Fireworks_Core_FWGeometryTableViewBase_h
#define Fireworks_Core_FWGeometryTableViewBase_h

#ifndef __CINT__
#include <boost/shared_ptr.hpp>
#endif


#include "Rtypes.h"
#include "TGFrame.h"

#include "Fireworks/Core/interface/FWViewType.h"
#ifndef __CINT__
#include "Fireworks/Core/interface/FWViewBase.h"
#include "Fireworks/Core/interface/FWViewBase.h"
#include "Fireworks/Core/interface/FWConfigurableParameterizable.h"
#include "Fireworks/Core/interface/FWStringParameter.h"
#include "Fireworks/Core/interface/FWEnumParameter.h"
#include "Fireworks/Core/interface/FWLongParameter.h"
#include "Fireworks/Core/interface/FWParameterSetterBase.h"
#include "Fireworks/Core/interface/FWParameterSetterEditorBase.h"
#endif


class TGTextButton;
class TGeoNode;
class TGeoVolume;
class TGTextEntry;
class TGComboBox;
class TGStatusBar;
class TGeoManager;
class TEveWindowSlot;
class TEveWindowFrame;

class FWTableWidget;
class FWGeometryTableManager;
class FWConfiguration;
class FWColorPopup;
class FWColorManager;
class FWGeoTopNode;
class FWParameterBase;
class FWViewCombo;
class FWGUIValidatingTextEntry;
class FWGeoMaterialValidator;

class FWGeometryTableViewBase
#ifndef __CINT__
   : public  FWViewBase,
     public  FWParameterSetterEditorBase
#endif
{

public:
   enum EMode { kNode, kVolume};

   FWGeometryTableViewBase(TEveWindowSlot*, FWViewType::EType, FWColorManager*, TGeoNode*, TObjArray*);
   virtual ~FWGeometryTableViewBase();
  
   void cellClicked(Int_t iRow, Int_t iColumn, 
                    Int_t iButton, Int_t iKeyMod, 
                    Int_t iGlobalX, Int_t iGlobalY);
  
   void chosenItem(int);
   void chosenItemFrom3DView(int);
   void selectView(int);
   void filterListCallback();
   void filterTextEntryCallback();
   void updateFilter(std::string&);

   void printTable();

   void cdNode(int);
   void cdTop();
   void cdUp();
   void setPath(int, std::string&);

   bool getVolumeMode()      const { return m_mode.value() == kVolume; }
   int getMode() const  { return m_mode.value() ;}
   std::string getFilter ()  const { return m_filter.value(); }
   int getAutoExpand()       const { return m_autoExpand.value(); }
   int getVisLevel()         const  {return m_visLevel.value(); }
   bool getIgnoreVisLevelWhenFilter() const  {return m_visLevelFilter.value(); }

   int getTopNodeIdx() const { return m_topNodeIdx.value(); }
   bool getEnableHighlight() { return m_enableHighlight.value(); } 
   FWGeometryTableManager*  getTableManager() { return m_tableManager;} 
   virtual void setFrom(const FWConfiguration&);

   // ---------- const member functions --------------------- 

   virtual void addTo(FWConfiguration&) const;
   virtual void saveImageTo( const std::string& iName ) const {}
   void nodeColorChangeRequested(Color_t);

   void setBackgroundColor();
   virtual void populate3DViewsFromConfig();


   bool drawTopNode() const { return !m_disableTopNode.value(); }
  
   bool rnrOvl() const { return typeId() == FWViewType::kOverlapTable; }
   //   TEvePointSet* overlapPnts() { return m_overlapPnts; }

   void refreshTable3D();
protected:
   virtual void initGeometry(TGeoNode* iGeoTopNode, TObjArray* iVolumes) = 0;

#ifndef __CINT__     
   FWLongParameter         m_topNodeIdx;   
   FWBoolParameter         m_enableHighlight;  
   FWEnumParameter         m_mode;
   FWStringParameter       m_filter; 
   FWBoolParameter         m_disableTopNode;
   FWLongParameter         m_autoExpand;
   FWLongParameter         m_visLevel;
   FWBoolParameter         m_visLevelFilter;   
#endif

   FWColorManager         *m_colorManager;
   FWTableWidget          *m_tableWidget;
   FWGeometryTableManager *m_tableManager;

   TGCompositeFrame       *m_settersFrame;
   FWGeoTopNode           *m_eveTopNode;

   FWColorPopup           *m_colorPopup;

   TEveWindowFrame*        m_eveWindow;
   TGCompositeFrame*       m_frame;

   FWViewCombo*            m_viewBox;

   FWGUIValidatingTextEntry* m_filterEntry;
   FWGeoMaterialValidator*   m_filterValidator;

   const FWConfiguration*  m_viewersConfig;

#ifndef __CINT__
   std::vector<boost::shared_ptr<FWParameterSetterBase> > m_setters;
#endif
   void resetSetters();
   void makeSetter(TGCompositeFrame* frame, FWParameterBase* param);

   void enableHighlight();

private:
   FWGeometryTableViewBase(const FWGeometryTableViewBase&); // stop default

   const FWGeometryTableViewBase& operator=(const FWGeometryTableViewBase&); // stop default

   ClassDef(FWGeometryTableViewBase, 0);
};

#endif

