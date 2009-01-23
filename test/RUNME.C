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

void RUNME(const char* datafile = 0) {
   // get geometry files if they are missing
   TFile* ff = TFile::Open("cmsGeom10.root");
   if (!ff )
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
   if ( !ff && !datafile ) {
      gSystem->Exec("wget -O data.root https://twiki.cern.ch/twiki/bin/viewfile/CMS/PhysicsToolsDevFireworksDistribution?filename=data.root");
      ff = TFile::Open("data.root");
   }

   if ( !ff ) {
      std::cout << "Failed to load data file" << std::endl;
      return;
   }

   fwlite::Event ev(ff);

   Bool_t debugMode = kFALSE;
   if ( gSystem->Getenv("FireworksDebug") ) debugMode = kTRUE;
   std::string configFile = "myconfig.fwc";
   if ( gSystem->Getenv("FireworksConfig") ) configFile = gSystem->Getenv("FireworksConfig");

   FWDisplayEvent ed(configFile,debugMode);


   FWPhysicsObjectDesc ecal("ECal",
                            TClass::GetClass("CaloTowerCollection"),
                            "ECal",
                            FWDisplayProperties(kRed),
                            "towerMaker",
                            "",
                            "",
                            "",
                            2);

   FWPhysicsObjectDesc hcal("HCal",
                            TClass::GetClass("CaloTowerCollection"),
                            "HCal",
                            FWDisplayProperties(kBlue),
                            "towerMaker",
                            "",
                            "",
                            "",
                            2);

   FWPhysicsObjectDesc jets("Jets",
                            TClass::GetClass("reco::CaloJetCollection"),
                            "Jets",
                            FWDisplayProperties(kYellow),
                            "iterativeCone5CaloJets",
                            "",
                            "",
                            "$.pt()>15",
                            3);

   FWPhysicsObjectDesc l1EmTrigs("L1EmTrig",
                                 TClass::GetClass("l1extra::L1EmParticleCollection"),
                                 "L1EmTrig",
                                 FWDisplayProperties(kOrange),
                                 "hltL1extraParticles",
                                 "Isolated",
                                 "",
                                 "",
                                 3);

   FWPhysicsObjectDesc l1MuonTrigs("L1MuonTrig",
                                   TClass::GetClass("l1extra::L1MuonParticleCollection"),
                                   "L1MuonTrig",
                                   FWDisplayProperties(kViolet),
                                   "hltL1extraParticles",
                                   "",
                                   "",
                                   "",
                                   3);

   FWPhysicsObjectDesc l1EtMissTrigs("L1EtMissTrig",
                                     TClass::GetClass("l1extra::L1EtMissParticleCollection"),
                                     "L1EtMissTrig",
                                     FWDisplayProperties(kTeal),
                                     "hltL1extraParticles",
                                     "",
                                     "",
                                     "",
                                     3);

   FWPhysicsObjectDesc l1JetTrigs("L1JetTrig",
                                  TClass::GetClass("l1extra::L1JetParticleCollection"),
                                  "L1JetTrig",
                                  FWDisplayProperties(kMagenta),
                                  "hltL1extraParticles",
                                  "Central",
                                  "",
                                  "",
                                  3);

   FWPhysicsObjectDesc tracks("Tracks",
                              TClass::GetClass("reco::TrackCollection"),
                              "Tracks",
                              FWDisplayProperties(kGreen),
                              "generalTracks",
                              "",
                              "",
                              "$.pt()>2",
                              1);

   FWPhysicsObjectDesc muons("Muons",
                             TClass::GetClass("reco::MuonCollection"),
                             "Muons",
                             FWDisplayProperties(kRed),
                             "muons",
                             "",
                             "",
                             "$.isGlobalMuon()",
                             5);

   FWPhysicsObjectDesc electrons("Electrons",
                                 TClass::GetClass("reco::GsfElectronCollection"),
                                 "Electrons",
                                 FWDisplayProperties(kCyan),
                                 "pixelMatchGsfElectrons",
                                 "",
                                 "",
                                 "$.hadronicOverEm()<0.05",
                                 3);

   FWPhysicsObjectDesc mets("METs",
                            TClass::GetClass("reco::CaloMETCollection"),
                            "METs",
                            FWDisplayProperties(kCyan),
                            "met",
                            "",
                            "",
                            "",
                            3);

   FWPhysicsObjectDesc genParticles("GenParticles",
                                    TClass::GetClass("reco::GenParticleCollection"),
                                    "GenParticles",
                                    FWDisplayProperties(kMagenta),
                                    "genParticles",
                                    "",
                                    "",
                                    "$.pt()>1 && $.status() == 3",
                                    6);

   // Vertices
   FWPhysicsObjectDesc vertices("Vertices",
                                TClass::GetClass("std::vector<reco::Vertex>"),
                                "Vertices",
                                FWDisplayProperties(kYellow),
                                "offlinePrimaryVertices",
                                "",
                                "",
                                "",
                                10);

   if (configFile.empty()) {
      ed.registerPhysicsObject(ecal);
      ed.registerPhysicsObject(hcal);
      ed.registerPhysicsObject(jets);
      ed.registerPhysicsObject(l1EmTrigs);
      ed.registerPhysicsObject(l1MuonTrigs);
      ed.registerPhysicsObject(l1EtMissTrigs);
      ed.registerPhysicsObject(l1JetTrigs);
      ed.registerPhysicsObject(tracks);
      ed.registerPhysicsObject(muons);
      ed.registerPhysicsObject(electrons);
      ed.registerPhysicsObject(genParticles);
      ed.registerPhysicsObject(vertices);
   }
   ed.registerPhysicsObject(mets);

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
