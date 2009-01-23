#include "DataFormats/FWLite/interface/Handle.h"
#include "TEveManager.h"
void cint_draw() {
   //TFile geomF("cmsGeom20.root");

   TEveManager::Create();
   gGeoManager = gEve->GetGeometry("cmsGeom10.root");

   TEveElementList* gL =
      new TEveElementList("CMS");
   gEve->AddGlobalElement(gL);

   TGeoNode* node = gGeoManager->GetTopVolume()->GetNode(0);
   TEveGeoTopNode* re = new TEveGeoTopNode(gGeoManager,
                   node);
   re->UseNodeTrans();
   gEve->AddGlobalElement(re,gL);
/*
   TFile f("h_zz_4mu.root");
   fwlite::Event ev(&f);

   ev.toBegin();
   {
    fwlite::Handle<edm::HepMCProduct> mc;
    mc.getByLabel(ev,"source");

    HepMC::GenEvent* mce = mc.ref().GetEvent();

    gEve->DisableRedraw();
    TEveTrackList* cont = new TEveTrackList("MC Tracks");
    cont->SetMainColor(Color_t(3));
    TEveTrackPropagator* rnrStyle = cont->GetPropagator();
    //units are kG
    rnrStyle->SetMagField( 4.0*10.);

    gEve->AddElement(cont);

    int index=0;
    cout <<"----"<<endl;
    TEveRecTrack t;
    t.beta = 1.;
    for(HepMC::GenEvent::particle_iterator it = mce->particles_begin();
        it != mce->particles_end();++it,++index) {
      HepMC::GenParticle* particle = *it;
      t.P = TEveVector(particle->momentum().x(),
                         particle->momentum().y(),
                         particle->momentum().z());
      HepMC::GenVertex* vertex = particle->production_vertex();
      if(0!=vertex) {
        t.V = TEveVector(vertex->position().x(),
                           vertex->position().y(),
                           vertex->position().z());
      } else {
        t.V = TEveVector();
      }
      t.sign = particle->pdg_id()>0?+1:-1;
      cout <<"px "<<t.P.x<<" py "<<t.P.y<<" pz "<<t.P.z
           <<" vx "<<t.V.x<<" vy "<<t.V.y<<" vz "<<t.V.z
           <<" sign "<<t.sign<<std::endl;
      gEve->AddElement(new TEveTrack(&t,rnrStyle),cont);
      //cout << particle->momentum().x()<<" "
      //   <<particle->momentum().y()<<" "
      //   <<particle->momentum().z()<<endl;
      //cout <<" *";
    }
    cout <<"finished"<<endl;
    cont->UpdateItems();
    cont->MakeTracks();
    gEve->EnableRedraw();
    gEve->Redraw3D();
   }
 */
}
