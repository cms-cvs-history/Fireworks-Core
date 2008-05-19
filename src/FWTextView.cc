#define private public
#include "DataFormats/FWLite/interface/Event.h"
#undef private
#include "DataFormats/FWLite/interface/Handle.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectronFwd.h"
#include "DataFormats/JetReco/interface/CaloJetCollection.h"
#include "DataFormats/METReco/interface/CaloMETCollection.h"
#include "DataFormats/METReco/interface/CaloMET.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "Fireworks/Core/interface/FWTextView.h"

#include <iostream>

void FWTextView::newEvent (const fwlite::Event &ev)
{
     //------------------------------------------------------------
     // get electrons
     //------------------------------------------------------------
     fwlite::Handle<reco::GsfElectronCollection> h_els;
     const reco::GsfElectronCollection *els = 0;
     int n_els = 0;
     try {
	  h_els.getByLabel(ev, "pixelMatchGsfElectrons");
	  els = h_els.ptr();
     }
     catch (...) 
     {
	  std::cout << "no electrons :-(" << std::endl;
     }
     if (els != 0)
	  n_els = els->size();
     //------------------------------------------------------------
     // get muons
     //------------------------------------------------------------
     fwlite::Handle<reco::MuonCollection> h_mus;
     const reco::MuonCollection *mus = 0;
     int n_mus = 0;
     try {
	  h_mus.getByLabel(ev, "muons");
	  mus = h_mus.ptr();
     }
     catch (...) 
     {
	  std::cout << "no muons :-(" << std::endl;
     }
     if (mus != 0)
	  n_mus = mus->size();
     //------------------------------------------------------------
     // get jets
     //------------------------------------------------------------
     fwlite::Handle<reco::CaloJetCollection> h_jets;
     const reco::CaloJetCollection *jets = 0;
     int n_jets = 0;
     try {
	  h_jets.getByLabel(ev, "iterativeCone5CaloJets");
	  jets = h_jets.ptr();
     }
     catch (...) 
     {
	  std::cout << "no jets :-(" << std::endl;
     }
     if (jets != 0)
	  n_jets = jets->size();
     //------------------------------------------------------------
     // get MET
     //------------------------------------------------------------
     fwlite::Handle<reco::CaloMETCollection> h_mets;
     const reco::CaloMETCollection *mets = 0;
     int n_mets = 0;
     try {
	  h_mets.getByLabel(ev, "met");
	  mets = h_mets.ptr();
     }
     catch (...) 
     {
	  std::cout << "no MET :-(" << std::endl;
     }
     if (mets != 0)
	  n_mets = mets->size();

     //------------------------------------------------------------
     // print header
     //------------------------------------------------------------
     printf("run %d event %d:\n", ev.aux_.run(), ev.aux_.event());
     std::cout << ev.aux_;
     assert(n_mets == 1);
     const CaloMET &met = *mets->begin();
     printf("MET %5.1f\t MET phi %.2f\t Sum Et %5.1f\t sig(MET) %.2f\n",
	    met.p4().Et(), met.phi(), met.sumEt(), met.mEtSig());
     printf("%d electrons\t%d muons\t%d jets\t\n", 
	    n_els, n_mus, n_jets);
     //------------------------------------------------------------
     // print muons
     //------------------------------------------------------------
     printf("Muons\n");
     printf("pt\t global\t tk\t SA\t calo\t iso(3)\t iso(5)\t tr pt\t eta\t"
	    " phi\t chi^2/ndof\t matches\t d0\t sig(d0)\t"
	    " loose(match)\t tight(match)\t loose(depth)\t tight(depth)\n");
     for (int i = 0; i < n_mus; ++i) {
	  const reco::Muon &muon = mus->at(i);
	  if (i == 5 || muon.pt() < 1) {
	       printf("skipping %d muons\n", n_mus - i);
	       break;
	  }
	  double trpt = -1; // need a better way to flag invalid track
	  double trd0 = -1;
	  double trsd0 = 1;
	  if (muon.track().isNonnull()) {
	       trpt = muon.track()->pt();
	       trd0 = muon.track()->d0();
	       trsd0 = muon.track()->d0Error();
	  }
	  printf("%5.1f\t %c\t %c\t %c\t %c\t %.3f\t %.3f\t %.3f\t %.3f\t %.3f\t"
		 "%.3f\t\t %d\t\t %.3f\t %7.3f\t %c\t\t %c\t\t %c\t\t %c\n",
		 muon.pt(), // is this right?
		 muon.isGlobalMuon() 		? 'y' : 'n', 
		 muon.isTrackerMuon()		? 'y' : 'n', 
		 muon.isStandAloneMuon()	? 'y' : 'n', 
		 muon.isCaloMuon()		? 'y' : 'n',
		 0., 0., // what iso? 
		 trpt, muon.eta(), muon.phi(),
		 0., // how to get chi^2?
		 muon.numberOfMatches(Muon::SegmentArbitration), // is this the right arbitration?
		 trd0, trd0 / trsd0,
		 'm', 'm', 'm', 'm' // get these flags!
	       );
     }
     //------------------------------------------------------------
     // print electrons
     //------------------------------------------------------------
     printf("Electrons\n");
     printf("Et\t eta\t phi\t E/p\t H/E\t fbrem\t dei\t dpi\t see\t spp\t"
	    " iso\t robust\t loose\t tight\n");
     for (int i = 0; i < n_els; ++i) {
	  const reco::GsfElectron &electron = els->at(i);
	  const double et = electron.caloEnergy() / cosh(electron.eta()); // is this the right way to get ET?
	  if (i == 5 || et < 1) {
	       printf("skipping %d electrons\n", n_els - i);
	       break;
	  }
	  const double pin  = electron.trackMomentumAtVtx().R();
	  const double pout = electron.trackMomentumOut().R();
	  printf("%5.1f\t %.3f\t %.3f\t %.3f\t %.3f\t %.3f\t %.3f\t %.3f\t"
		 "%.3f\t %.3f\t %.3f\t %c\t %c\t %c\n",
		 et, electron.eta(), electron.phi(),
		 electron.eSuperClusterOverP(), electron.hadronicOverEm(),
		 (pin - pout) / pin,
		 electron.deltaEtaSuperClusterTrackAtVtx(),
		 electron.deltaPhiSuperClusterTrackAtVtx(),
		 0., 0., // can we get the shape?
		 0., // can we get the iso?
		 'm', 'm', 'm' // can we get these flags?
	       );
     }
     //------------------------------------------------------------
     // print jets
     //------------------------------------------------------------
     printf("Jets\n");
     printf("Et\t eta\t phi\t ECAL\t HCAL\t emf\t chf\n");
     for (int i = 0; i < n_jets; ++i) {
	  const reco::CaloJet &jet = jets->at(i);
	  const double et = jet.p4().Et();
	  if (i == 20 || et < 1) {
	       printf("skipping %d jets\n", n_jets - i);
	       break;
	  }
	  printf("%5.1f\t %.3f\t %.3f\t %5.1f\t %5.1f\t %.3f\t %.3f\n",
		 et, jet.eta(), jet.phi(),
		 jet.p4().E() * jet.emEnergyFraction(),		// this has got
		 jet.p4().E() * jet.energyFractionHadronic(),	// to be a joke
		 jet.emEnergyFraction(), 
		 0.	// how do we get the charge fraction?
	       );
     }
}
