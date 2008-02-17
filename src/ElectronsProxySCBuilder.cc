// -*- C++ -*-
//
// Package:     Calo
// Class  :     ElectronsProxySCBuilder
// 
// Implementation:
//     <Notes on implementation>
//
// Original Author:  
//         Created:  Sun Jan  6 23:57:00 EST 2008
// $Id: ElectronsProxySCBuilder.cc,v 1.1.2.1 2008/02/15 21:48:18 jmuelmen Exp $
//

// system include files
#include "TEveGeoNode.h"
#include "TEveGeoShapeExtract.h"
#include "TGeoBBox.h"
#include "TGeoArb8.h"
#include "TGeoTube.h"
#include "TEveManager.h"
#include "TH1F.h"
#include "TColor.h"
#include "TROOT.h"
#include "TEveTrack.h"
#include "TEveTrackPropagator.h"

// user include files
#include "Fireworks/Electrons/interface/ElectronsProxy3DBuilder.h"
#include "Fireworks/Core/interface/ElectronsProxySCBuilder.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/DetIdToMatrix.h"
#include "DataFormats/EgammaCandidates/interface/PixelMatchGsfElectron.h"
#include "DataFormats/EgammaReco/interface/BasicClusterShapeAssociation.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
ElectronsProxySCBuilder::ElectronsProxySCBuilder()
{
}

// ElectronsProxySCBuilder::ElectronsProxySCBuilder(const ElectronsProxySCBuilder& rhs)
// {
//    // do actual copying here;
// }

ElectronsProxySCBuilder::~ElectronsProxySCBuilder()
{
}

//
// member functions
//
void ElectronsProxySCBuilder::build (TEveElementList **product) 
{
     printf("calling ElectronsProxySCBuilder::buildRhoZ\n");
     TEveElementList* tList = *product;
     if(0 == tList) {
	  tList =  new TEveElementList(m_item->name().c_str(),"Supercluster RhoZ",true);
	  *product = tList;
	  tList->SetMainColor(m_item->defaultDisplayProperties().color());
	  gEve->AddElement(tList);
     } else {
	  tList->DestroyElements();
     }
     // get electrons
#if 0
     using reco::PixelMatchGsfElectronCollection;
     const PixelMatchGsfElectronCollection *electrons = 0;
     printf("getting electrons\n");
     m_item->get(electrons);
     printf("got electrons\n");
     if (electrons == 0) {
	  std::cout <<"failed to get GSF electrons" << std::endl;
	  return;
     }
     using reco::PixelMatchGsfElectronCollection;
     const PixelMatchGsfElectronCollection *electrons = 
	  ElectronsProxy3DBuilder::electrons;
     printf("%d GSF electrons\n", electrons->size());
#endif
     // get rechits
     const EcalRecHitCollection *hits = 0;
     printf("getting rechits\n");
     m_item->get(hits);
     printf("got rechits\n");
     if (hits == 0) {
	  std::cout <<"failed to get Ecal RecHits" << std::endl;
	  return;
     }
#if 0
     TEveTrackPropagator *propagator = new TEveTrackPropagator();
     propagator->SetMagField( -4.0);
     propagator->SetMaxR( 120 );
     propagator->SetMaxZ( 300 );
     int index=0;
     TEveRecTrack t;
//      t.fBeta = 1.;
     for(PixelMatchGsfElectronCollection::const_iterator i = electrons->begin();
	 i != electrons->end(); ++i, ++index) {
	  assert(i->gsfTrack().isNonnull());
	  t.fP = TEveVector(i->gsfTrack()->px(),
			    i->gsfTrack()->py(),
			    i->gsfTrack()->pz());
	  t.fV = TEveVector(i->gsfTrack()->vx(),
			    i->gsfTrack()->vy(),
			    i->gsfTrack()->vz());
	  t.fSign = i->gsfTrack()->charge();
	  TEveTrack* trk = new TEveTrack(&t, propagator);
	  trk->SetMainColor(m_item->defaultDisplayProperties().color());
	  tList->AddElement(trk);
	  //cout << it->px()<<" "
	  //   <<it->py()<<" "
	  //   <<it->pz()<<endl;
	  //cout <<" *";
	  assert(i->superCluster().isNonnull());
     }
#endif
     printf("%d RecHits\n", hits->size());
     for (EcalRecHitCollection::const_iterator i = hits->begin();
	  i != hits->end(); ++i) {
	  TEveGeoShapeExtract* extract = m_item->getGeom()->getExtract(i->id().rawId() );
	  printf("extract is 0x%x\n", extract);
	  if(0!=extract) {
	       TEveTrans t = extract->GetTrans();
	       t.MoveLF(3, -i->energy() / 2);
	       TGeoBBox *sc_box = new TGeoBBox(1.1, 1.1, i->energy() / 2, 0);
	       TEveGeoShapeExtract *extract2 = new 
		    TEveGeoShapeExtract("SC");
	       extract2->SetTrans(t.Array());
	       TColor* c = gROOT->GetColor(tList->GetMainColor());
	       Float_t rgba[4] = { 1, 0, 0, 1 };
	       if (c) {
		    rgba[0] = c->GetRed();
		    rgba[1] = c->GetGreen();
		    rgba[2] = c->GetBlue();
	       }
	       extract2->SetRGBA(rgba);
	       extract2->SetRnrSelf(true);
	       extract2->SetRnrElements(true);
	       extract2->SetShape(sc_box);
	       tList->AddElement(TEveGeoShape::ImportShapeExtract(extract2,0));
#if 0
	       TGeoTrap *crystal = dynamic_cast<TGeoTrap *>(extract->GetShape());
	       assert(crystal != 0);
// 	       printf("%d\n", (char *)(&crystal->fH1) - (char *)crystal);
	       double *H1 = (double *)crystal + 30; // this is a kluge
	       printf("%f\n", *H1);
// 	       *H1++ = i->energy() / 10;
// 	       *H1++ = i->energy() / 10;
// 	       *H1++ = i->energy() / 10;
// 	       H1++;
// 	       *H1++ = i->energy() / 10;
// 	       *H1++ = i->energy() / 10;
// 	       *H1++ = i->energy() / 10;
	       TEveElement* shape = TEveGeoShape::ImportShapeExtract(extract,0);
	       shape->SetMainTransparency(50);
	       shape->SetMainColor(Color_t(kBlack + (int)floor(i->energy() + 10))); // tList->GetMainColor());
	       gEve->AddElement(shape);
	       tList->AddElement(shape);
#endif
	  }
     }
}
