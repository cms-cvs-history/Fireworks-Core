#ifndef __CINT__
#include <iostream>
#include "TClass.h"
#include "TError.h"
#include "TEventList.h"
#include "TFile.h"
#include "TSystem.h"
#include "TTree.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "Fireworks/Core/interface/FWDisplayEvent.h"
#include "Fireworks/Core/interface/FWPhysicsObjectDesc.h"
#endif
//if this is commented out then 'ev' disappears from CINT after ed.draw(ev)
// #include "Fireworks/Core/interface/FWDisplayEvent.h"
// however, the minimal 'fix' is to just declare the class it already successful talked with!

void cosmics(const char* datafile = 0) {
   // get geometry files if they are missing
   TFile* ff = TFile::Open("cmsGeom10.root");
   if (! ff )
     gSystem->Exec("wget -O cmsGeom10.root https://twiki.cern.ch/twiki/bin/viewfile/CMS/PhysicsToolsDevFireworksDistribution?filename=cmsGeom10.root");
   else
     ff->Close();

   // load data file
   gErrorIgnoreLevel = 3000; // suppress warnings about missing dictionaries
   if ( datafile )
     ff = TFile::Open(datafile);
   else
     ff = TFile::Open("cosmics.root");
   gErrorIgnoreLevel = -1;
   
   if ( ! ff ) {
	std::cout << "Failed to load data file" << std::endl;
	return;
   }
   
   fwlite::Event ev(ff);
   
   Bool_t debugMode = kFALSE;
   if ( gSystem->Getenv("FireworksDebug") ) debugMode = kTRUE;
   std::string configFile = "cosmics.fwc";
   if ( gSystem->Exec(("ls "+configFile).c_str()) != 0) configFile = "src/Fireworks/Core/macros/cosmics.fwc";
   if ( gSystem->Getenv("FireworksConfig") ) configFile = gSystem->Getenv("FireworksConfig");

   FWDisplayEvent ed(configFile,debugMode);
   FWDisplayEvent::setMagneticField(0);
   FWDisplayEvent::setCaloScale(-1);

   //The following will be moved to a configuration file
   ed.registerProxyBuilder("ECal","ECalCaloTowerProxy3DLegoBuilder");
   ed.registerProxyBuilder("HCal","HCalCaloTowerProxy3DLegoBuilder");
   ed.registerProxyBuilder("Jets","CaloJetProxyEveLegoBuilder");
   ed.registerProxyBuilder("Jets","CaloJetProxyTH2LegoBuilder");
   ed.registerProxyBuilder("Jets","CaloJetSelectedProxyTH2LegoBuilder");
   
   ed.registerProxyBuilder("ECal","ECalCaloTowerProxyRhoPhiZ2DBuilder");
   ed.registerProxyBuilder("HCal","HCalCaloTowerProxyRhoPhiZ2DBuilder");
   // ed.registerProxyBuilder("Jets","CaloJetProxyRhoPhiZ2DBuilder");
   // ed.registerProxyBuilder("Tracks","TracksProxy3DBuilder");
   ed.registerProxyBuilder("Muons","MuonsProxyRhoPhiZ2DBuilder");
   ed.registerProxyBuilder("DTSegments","DTSegmentsProxyRhoPhiZ2DBuilder");
//       ed.registerProxyBuilder("MuonsPU","MuonsProxyPUBuilder");
   //   ed.registerProxyBuilder("ElectronTracks","ElectronsProxy3DBuilder");
   //   ed.registerProxyBuilder("Electrons","ElectronsProxyRhoPhiZ2DBuilder");
   //   ed.registerProxyBuilder("ElectronSC","ElectronsProxySCBuilder");
   // ed.registerProxyBuilder("ElectronTracks","ElectronsProxy3DBuilder");
   // ed.registerProxyBuilder("Electrons","ElectronsProxyRhoPhiZ2DBuilder");
//    ed.registerProxyBuilder("ElectronSC","ElectronsProxySCBuilder");
   //ed.registerProxyBuilder("Calo","CaloProxyLegoBuilder");
//    ed.registerProxyBuilder("TrackHits", "TracksRecHitsProxy3DBuilder");

   FWPhysicsObjectDesc ecal("ECal",
                            TClass::GetClass("CaloTowerCollection"),
                            FWDisplayProperties(kRed),
                            "towerMaker",
                            "",
                            "",
                            "",
                            2);

   FWPhysicsObjectDesc hcal("HCal",
		    TClass::GetClass("CaloTowerCollection"),
                            FWDisplayProperties(kBlue),
                            "towerMaker",
                            "",
                            "",
                            "",
                            2);
/*
   FWPhysicsObjectDesc jets("Jets",
                            TClass::GetClass("reco::CaloJetCollection"),
                            FWDisplayProperties(kYellow),
                            "iterativeCone5CaloJets",
                            "",
                            "",
                            "$.pt()>15",
                            3);
*/
   FWPhysicsObjectDesc tracks("Tracks",
                              TClass::GetClass("reco::TrackCollection"),
                              FWDisplayProperties(kGreen),
                              "generalTracks",
                              "",
                              "",
                              "$.pt()>0",
                              1);

   FWPhysicsObjectDesc muons("Muons",
                             TClass::GetClass("reco::MuonCollection"),
                             FWDisplayProperties(kRed),
                             "muons",
                             "",
                             "",
                             "$.pt()>0",
                             5);

   FWPhysicsObjectDesc dtSegments("DTSegments",
                             TClass::GetClass("DTRecSegment4DCollection"),
                             FWDisplayProperties(kBlue),
                             "dt4DSegments",
                             "",
                             "",
                             "",
                             1);
   
   FWPhysicsObjectDesc electrons("Electrons",
				 TClass::GetClass("reco::GsfElectronCollection"),
				 FWDisplayProperties(kCyan),
				 "pixelMatchGsfElectrons",
                                 "",
                                 "",
                                 "$.pt()>0",
                                 3);

   if (configFile.empty()) {
      ed.registerPhysicsObject(ecal);
      ed.registerPhysicsObject(hcal);
//      ed.registerPhysicsObject(jets);
//      ed.registerPhysicsObject(tracks);
      ed.registerPhysicsObject(muons);
      ed.registerPhysicsObject(dtSegments);
//      ed.registerPhysicsObject(electrons);
   }
   

   //Finished configuration

   TTree* events = (TTree*)ff->Get("Events");
   TEventList* list = new TEventList("list","");
   
//     const char* selection = "pixelMatchGsfElectrons.pt() > 0"; // " && pixelMatchGsfElectrons.phi() > 0";
   const char* selection = 0;
   if ( selection && events )
     {
	events->Draw(">>list",selection);
	events->SetEventList( list );
	printf("WANRING: looping over selected events!\n\tselection: \t%s\n",selection);
     }
   int nEntries = ev.size();
   if ( events && events->GetEventList() ) nEntries = list->GetN();
   
   for( unsigned int i = 0 /*1909*/; i < nEntries; ) {
      if ( events && events->GetEventList() ) 
	ev.to( events->GetEntryNumber(i) );
      else 
	ev.to(i);
      printf("starting to draw event %d...\n", i);
      int code = ed.draw(ev);
      printf("... event %d drawn\n", i);
      if ( code == 1 ) ++i;
      if ( code == -1 && i>0 ) --i;
      if ( code == -2 ) i=0;
      if ( code == -3 ) break;
   }
}
