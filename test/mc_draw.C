#include "TFile.h"
#include "DataFormats/FWLite/interface/Event.h"
//if this is commented out then 'ev' disappears from CINT after ed.draw(ev)
//#include "PhysicsTools/Fireworks/interface/FWDisplayEvent.h"
// however, the minimal 'fix' is to just declare the class it already successful talked with!
class FWDisplayEvent;
void mc_draw() {
  //TFile geomF("cmsGeom20.root");
  
  TFile f("h_zz_4mu.root");
  fwlite::Event ev(&f);
  
  FWDisplayEvent ed;
  for(ev.toBegin();
      !ev.atEnd();
      ++ev)
  {
    ed.draw(ev);
  }
}
