#include "TFile.h"
#include "DataFormats/FWLite/interface/Event.h"
//if this is commented out then 'ev' disappears from CINT after ed.draw(ev)
//#include "PhysicsTools/Fireworks/interface/FWDisplayEvent.h"
// however, the minimal 'fix' is to just declare the class it already successful talked with!
class FWDisplayEvent;
void track_draw() {
  //TFile geomF("cmsGeom20.root");
  
  TFile f("/data/dmytro/samples/RelVal-RelVal151Higgs-ZZ-4Mu-1183228327-0000-860585E7-5227-DC11-8227-000423D662EE.root");
  fwlite::Event ev(&f);
  
  FWDisplayEvent ed;
  for( unsigned int i = 0; i < ev.size(); ) {
     ev.to(i);
     int code = ed.draw(ev);
     if ( code == 1 ) ++i;
     if ( code == -1 && i>0 ) --i;
     if ( code == -2 ) i=0;
     if ( code == -3 ) break;
  }
}
