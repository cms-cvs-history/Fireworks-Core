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
#include "Fireworks/Core/interface/ElectronDetailView.h"
#include "Fireworks/Core/interface/TrackDetailView.h"
#include "Fireworks/Core/interface/MuonDetailView.h"
#endif
//if this is commented out then 'ev' disappears from CINT after ed.draw(ev)
// #include "Fireworks/Core/interface/FWDisplayEvent.h"
// however, the minimal 'fix' is to just declare the class it already successful talked with!

void RUNME(const char* datafile = 0) {
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
     ff = TFile::Open("data.root");
     //     ff = TFile::Open("CSA07-CSA07Muon-Chowder-A3-PDMuon-ReReco-100pb-Skims4_0000_08A0A10B-25C5-DC11-BE27-001617C3B6B4.root");
   gErrorIgnoreLevel = -1;
   if ( ! ff && ! datafile ) {
      gSystem->Exec("wget -O data.root https://twiki.cern.ch/twiki/bin/viewfile/CMS/PhysicsToolsDevFireworksDistribution?filename=data.root");
      ff = TFile::Open("data.root");
   }
   
   if ( ! ff ) {
	std::cout << "Failed to load data file" << std::endl;
	return;
   }
   
   fwlite::Event ev(ff);
   
   Bool_t debugMode = kFALSE;
   if ( gSystem->Getenv("FireworksDebug") ) debugMode = kTRUE;
   Bool_t newLegoMode = kFALSE;
   if ( gSystem->Getenv("FireworksNewLego") ) newLegoMode = kTRUE;
   std::string configFile = "";
   if ( gSystem->Getenv("FireworksConfig") ) configFile = gSystem->Getenv("FireworksConfig");
   
   FWDisplayEvent ed(configFile,debugMode,newLegoMode);
   
   //uncomment this to read the configuration
   //FWDisplayEvent ed("myconfig.fwc",debugMode);

   //The following will be moved to a configuration file
   ed.registerProxyBuilder("ECal","ECalCaloTowerProxy3DLegoBuilder");
   ed.registerProxyBuilder("HCal","HCalCaloTowerProxy3DLegoBuilder");
   ed.registerProxyBuilder("Jets","CaloJetProxyEveLegoBuilder");
   ed.registerProxyBuilder("Jets","CaloJetProxyTH2LegoBuilder");
   ed.registerProxyBuilder("Jets","CaloJetSelectedProxyTH2LegoBuilder");
   
   ed.registerProxyBuilder("ECal","ECalCaloTowerProxyRhoPhiZ2DBuilder");
   ed.registerProxyBuilder("HCal","HCalCaloTowerProxyRhoPhiZ2DBuilder");
   ed.registerProxyBuilder("Jets","CaloJetProxyRhoPhiZ2DBuilder");
   ed.registerProxyBuilder("Tracks","TracksProxy3DBuilder");
   ed.registerProxyBuilder("Muons","MuonsProxyRhoPhiZ2DBuilder");
//       ed.registerProxyBuilder("MuonsPU","MuonsProxyPUBuilder");
   //   ed.registerProxyBuilder("ElectronTracks","ElectronsProxy3DBuilder");
   //   ed.registerProxyBuilder("Electrons","ElectronsProxyRhoPhiZ2DBuilder");
   //   ed.registerProxyBuilder("ElectronSC","ElectronsProxySCBuilder");
   // ed.registerProxyBuilder("ElectronTracks","ElectronsProxy3DBuilder");
   ed.registerProxyBuilder("Electrons","ElectronsProxyRhoPhiZ2DBuilder");
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
   ed.registerPhysicsObject(ecal);

   FWPhysicsObjectDesc hcal("HCal",
		    TClass::GetClass("CaloTowerCollection"),
                            FWDisplayProperties(kBlue),
                            "towerMaker",
                            "",
                            "",
                            "",
                            2);
   ed.registerPhysicsObject(hcal);

   FWPhysicsObjectDesc jets("Jets",
                            TClass::GetClass("reco::CaloJetCollection"),
                            FWDisplayProperties(kYellow),
                            "iterativeCone5CaloJets",
                            "",
                            "",
                            "$.pt()>15",
                            3);
   ed.registerPhysicsObject(jets);

   FWPhysicsObjectDesc tracks("Tracks",
                              TClass::GetClass("reco::TrackCollection"),
                              FWDisplayProperties(kGreen),
                              "ctfWithMaterialTracks",
                              "",
                              "",
                              "$.pt()>0",
                              1);
   ed.registerPhysicsObject(tracks);

//    FWPhysicsObjectDesc trackhits("TrackHits",
// 		      TClass::GetClass("reco::TrackCollection"),
// 		      FWDisplayProperties(kWhite),
// 		      "ctfWithMaterialTracks");
//    ed.registerPhysicsObject(trackhits);

   FWPhysicsObjectDesc muons("Muons",
                             TClass::GetClass("reco::MuonCollection"),
                             FWDisplayProperties(kRed),
                             "trackerMuons",
                             "",
                             "",
                             "$.pt()>0",
                             5);
   ed.registerPhysicsObject(muons);

//       FWPhysicsObjectDesc muonsPU("MuonsPU", 
// 	TClass::GetClass("reco::MuonCollection"), 
// 	FWDisplayProperties(kRed), 
// 	"trackerMuons"); 
// 	ed.registerPhysicsObject(muonsPU); 
   

   FWPhysicsObjectDesc electrons("Electrons",
				 TClass::GetClass("reco::PixelMatchGsfElectronCollection"),
				 FWDisplayProperties(kCyan),
				 "pixelMatchGsfElectrons",
                                 "",
                                 "",
                                 "$.pt()>0",
                                 3);
   ed.registerPhysicsObject(electrons);

//    FWPhysicsObjectDesc electronTracks("ElectronTracks",
// 				      TClass::GetClass("reco::PixelMatchGsfElectronCollection"),
// 				      FWDisplayProperties(kPink),
// 				      "pixelMatchGsfElectrons");
//    ed.registerPhysicsObject(electronTracks);


//    FWPhysicsObjectDesc electronSC("ElectronSC",
// 				     TClass::GetClass("reco::PixelMatchGsfElectronCollection"),
// 				     FWDisplayProperties(kYellow),
// 				     "pixelMatchGsfElectrons");
//    ed.registerPhysicsObject(electronSC);

   // register detail viewers
   ed.registerDetailView("Electrons", new ElectronDetailView);
   ed.registerDetailView("Muons", new MuonDetailView);
   ed.registerDetailView("Tracks", new TrackDetailView);

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
   ed.writeConfigurationFile("myconfig.fwc");
}
