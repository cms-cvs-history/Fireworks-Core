#include "TFile.h"
#include "DataFormats/FWLite/interface/Event.h"
//if this is commented out then 'ev' disappears from CINT after ed.draw(ev)
// #include "Fireworks/Core/interface/FWDisplayEvent.h"
// however, the minimal 'fix' is to just declare the class it already successful talked with!
class FWDisplayEvent;
void RUNME(const char* datafile = "data.root") {
   
   // get geometry files if they are missing
   TFile* ff = TFile::Open("cmsGeom10.root");
   if (! ff ) 
     gSystem->Exec("wget -O cmsGeom10.root https://twiki.cern.ch/twiki/bin/viewfile/CMS/PhysicsToolsDevFireworksDistribution?filename=cmsGeom10.root");
   else
     ff->Close();
   ff = TFile::Open("tracker.root");
   if (! ff ) 
     gSystem->Exec("wget -O tracker.root https://twiki.cern.ch/twiki/bin/viewfile/CMS/PhysicsToolsDevFireworksDistribution?filename=tracker.root");
   else
     ff->Close();
   
   // load data file
   gErrorIgnoreLevel = 3000; // suppress warnings about missing dictionaries
   ff = TFile::Open(datafile);
   gErrorIgnoreLevel = -1;
   if ( ! ff ) {
      cout << "Please provide data file to display: RUNME(\"myfile.root\")" <<endl;
      return;
   }
	
   fwlite::Event ev(ff);
   
   FWDisplayEvent ed;

   //The following will be moved to a configuration file
   ed.registerProxyBuilder("ECal","ECalCaloTowerProxy3DLegoBuilder");
   ed.registerProxyBuilder("HCal","HCalCaloTowerProxy3DLegoBuilder");
   ed.registerProxyBuilder("Jets","CaloJetProxy3DLegoBuilder");
   ed.registerProxyBuilder("Tracks","TracksProxy3DBuilder");
   ed.registerProxyBuilder("Muons","MuonsProxy3DBuilder");
   //ed.registerProxyBuilder("Calo","CaloProxyLegoBuilder");

   FWEventItem ecal("ECal",
		    TClass::GetClass("CaloTowerCollection"),
		    FWDisplayProperties(kBlue),
		    "towerMaker");
   ed.registerEventItem(ecal);

   FWEventItem hcal("HCal",
		    TClass::GetClass("CaloTowerCollection"),
		    FWDisplayProperties(kRed),
		    "towerMaker");
   ed.registerEventItem(hcal);

   FWEventItem jets("Jets",
		    TClass::GetClass("reco::CaloJetCollection"),
		    FWDisplayProperties(kYellow),
		    "iterativeCone5CaloJets");
   ed.registerEventItem(jets);

   FWEventItem tracks("Tracks",
		      TClass::GetClass("reco::TrackCollection"),
		      FWDisplayProperties(kGreen),
		      "ctfWithMaterialTracks");
   ed.registerEventItem(tracks);

   FWEventItem muons("Muons",
		     TClass::GetClass("reco::MuonCollection"),
		     FWDisplayProperties(kRed),
		     "trackerMuons");
   ed.registerEventItem(muons);
   //Finished configuration
   
   for( unsigned int i = 0; i < ev.size(); ) {
      ev.to(i);
      int code = ed.draw(ev);
      if ( code == 1 ) ++i;
      if ( code == -1 && i>0 ) --i;
      if ( code == -2 ) i=0;
      if ( code == -3 ) break;
   }
}
