#include "TFile.h"
#include "DataFormats/FWLite/interface/Event.h"
//if this is commented out then 'ev' disappears from CINT after ed.draw(ev)
// #include "Fireworks/Core/interface/FWDisplayEvent.h"
// however, the minimal 'fix' is to just declare the class it already successful talked with!
class FWDisplayEvent;
void RUNME(const char* datafile = 0) {
   
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
   if ( datafile )
     ff = TFile::Open(datafile);
   else
     ff = TFile::Open("data.root");
   gErrorIgnoreLevel = -1;
   if ( ! ff && ! datafile ) {
      gSystem->Exec("wget -O data.root https://twiki.cern.ch/twiki/bin/viewfile/CMS/PhysicsToolsDevFireworksDistribution?filename=data.root");
      ff = TFile::Open("data.root");
   }
   
   if ( ! ff ) {
      cout << "Failed to load data file" <<endl;
      return;
   }
   
   fwlite::Event ev(ff);
   
   FWDisplayEvent ed;

   //The following will be moved to a configuration file
   ed.registerProxyBuilder("ECal","ECalCaloTowerProxy3DLegoBuilder");
   ed.registerProxyBuilder("ECal","ECalCaloTowerProxyRhoPhiZ2DBuilder");
   ed.registerProxyBuilder("HCal","HCalCaloTowerProxy3DLegoBuilder");
   ed.registerProxyBuilder("HCal","HCalCaloTowerProxyRhoPhiZ2DBuilder");
   ed.registerProxyBuilder("Jets","CaloJetProxy3DLegoBuilder");
   ed.registerProxyBuilder("Tracks","TracksProxy3DBuilder");
   ed.registerProxyBuilder("Muons","MuonsProxy3DBuilder");
   ed.registerProxyBuilder("ElectronTracks","ElectronsProxy3DBuilder");
   ed.registerProxyBuilder("Electrons","ElectronsProxyRhoPhiZ2DBuilder");
   //ed.registerProxyBuilder("Calo","CaloProxyLegoBuilder");

   FWPhysicsObjectDesc ecal("ECal",
		    TClass::GetClass("CaloTowerCollection"),
		    FWDisplayProperties(kRed),
		    "towerMaker");
   ed.registerPhysicsObject(ecal);

   FWPhysicsObjectDesc hcal("HCal",
		    TClass::GetClass("CaloTowerCollection"),
		    FWDisplayProperties(kBlue),
		    "towerMaker");
   ed.registerPhysicsObject(hcal);

   FWPhysicsObjectDesc jets("Jets",
		    TClass::GetClass("reco::CaloJetCollection"),
		    FWDisplayProperties(kYellow),
		    "iterativeCone5CaloJets");
   ed.registerPhysicsObject(jets);

   FWPhysicsObjectDesc tracks("Tracks",
		      TClass::GetClass("reco::TrackCollection"),
		      FWDisplayProperties(kGreen),
		      "ctfWithMaterialTracks");
   ed.registerPhysicsObject(tracks);

   FWPhysicsObjectDesc muons("Muons",
		     TClass::GetClass("reco::MuonCollection"),
		     FWDisplayProperties(kRed),
		     "trackerMuons");
   ed.registerPhysicsObject(muons);

   FWPhysicsObjectDesc electrons("Electrons",
				 TClass::GetClass("reco::PixelMatchGsfElectronCollection"),
				 FWDisplayProperties(kYellow),
				 "randompixelMatchGsfElectrons");
   ed.registerPhysicsObject(electrons);

   FWPhysicsObjectDesc electronTracks("ElectronTracks",
				      TClass::GetClass("reco::PixelMatchGsfElectronCollection"),
				      FWDisplayProperties(kYellow),
				      "pixelMatchGsfElectrons");
   ed.registerPhysicsObject(electronTracks);

//    FWPhysicsObjectDesc hybridSuperclusters("BasicClusterShapeAssociation",
//  					   TClass::GetClass("reco::BasicClusterShapeAssociationCollection"),
//  					   FWDisplayProperties(kYellow),
//  					   "hybridSuperClusters");
//    ed.registerPhysicsObject(hybridSuperclusters);
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
