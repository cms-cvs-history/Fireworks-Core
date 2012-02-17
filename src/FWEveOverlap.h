#ifndef Fireworks_Core_FWGeoEveOverlap_h
#define Fireworks_Core_FWGeoEveOverlap_h

#include "Fireworks/Core/interface/FWGeoTopNode.h"
#include "TString.h"
#include <Rtypes.h>

class FWGeometryTableManagerBase;

class FWEveOverlap : public FWGeoTopNode
{
public:

   enum MenuOptions {
      kOvlVisOff,
      kOvlVisOnOvl,
      kOvlVisOnAllMother,
      kOvlVisMother,
      kOvlSwitchVis,
      kOvlCamera,
      kOvlPrintOvl,
      kOvlPrintPath
   };

   FWEveOverlap(FWOverlapTableView* v);
   virtual ~FWEveOverlap(){}

   virtual void Paint(Option_t* option="");
   virtual TString     GetHighlightTooltip();

   virtual FWGeometryTableManagerBase* tableManager();
   virtual void popupMenu(int x, int y);
private:
   FWOverlapTableView       *m_browser;

   ClassDef(FWEveOverlap, 0);
};

#endif
