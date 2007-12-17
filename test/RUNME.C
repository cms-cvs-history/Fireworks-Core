#include "TFile.h"
#include "DataFormats/FWLite/interface/Event.h"
//if this is commented out then 'ev' disappears from CINT after ed.draw(ev)
// #include "Fireworks/Core/interface/FWDisplayEvent.h"
// however, the minimal 'fix' is to just declare the class it already successful talked with!
class FWDisplayEvent;
void RUNME() {
   //TFile geomF("cmsGeom20.root");
  
   // TFile f("/data/dmytro/samples/RelVal-RelVal151Higgs-ZZ-4Mu-1183228327-0000-860585E7-5227-DC11-8227-000423D662EE.root");
   TFile f("/data/dmytro/samples/RelVal-RelVal151TTbar-1183231184-0000-0CC1FC7F-A127-DC11-88B4-001617C3B6D2.root");
   fwlite::Event ev(&f);
   
   FWDisplayEvent ed;
   ed.registerProxyBuilder("Tracks","TracksProxy3DBuilder");
   ed.registerProxyBuilder("Muons","MuonsProxy3DBuilder");
   ed.registerProxyBuilder("Calo","CaloProxyLegoBuilder");
   
   for( unsigned int i = 0; i < ev.size(); ) {
      ev.to(i);
      int code = ed.draw(ev);
      if ( code == 1 ) ++i;
      if ( code == -1 && i>0 ) --i;
      if ( code == -2 ) i=0;
      if ( code == -3 ) break;
   }
}
