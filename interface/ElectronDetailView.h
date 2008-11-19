// -*- C++ -*-
#ifndef Fireworks_Core_ElectronDetailView_h
#define Fireworks_Core_ElectronDetailView_h
//
// Package:     Calo
// Class  :     ElectronDetailView
// 
/**\class ElectronDetailView ElectronDetailView.h Fireworks/Calo/interface/ElectronDetailView.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Original Author:  
//         Created:  Sun Jan  6 23:42:33 EST 2008
// $Id: ElectronDetailView.h,v 1.4 2008/07/01 05:03:25 jmuelmen Exp $
//

// system include files

#include "Rtypes.h"

// user include files
#include "TEveBoxSet.h"
#include "Fireworks/Core/interface/FWDetailView.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

// forward declarations

class FWEventItem;
class TEveElementList;
namespace reco {
     class GsfElectron;
}

class ElectronDetailView : public FWDetailView {
     
public:
     ElectronDetailView();
     virtual ~ElectronDetailView();
     
     virtual void build (TEveElementList **product, const FWModelId &id);

protected:
     void setItem (const FWEventItem *iItem) { m_item = iItem; }
     void build_3d (TEveElementList **product, const FWModelId &id);
     void build_projected (TEveElementList **product, const FWModelId &id);
     void getCenter( Double_t* vars )
     {
	vars[0] = rotation_center[0];
	vars[1] = rotation_center[1];
	vars[2] = rotation_center[2];
     }
     TEveElementList *makeLabels (const reco::GsfElectron &);
//      TEveElementList *getEcalCrystalsBarrel (const class DetIdToMatrix &,
// 					     const std::vector<class DetId> &);
//      TEveElementList *getEcalCrystalsBarrel (const class DetIdToMatrix &,
// 					     double eta, double phi,
// 					     int n_eta = 5, int n_phi = 10);
//      TEveElementList *getEcalCrystalsEndcap (const class DetIdToMatrix &,
// 					     const std::vector<class DetId> &);
//      TEveElementList *getEcalCrystalsEndcap (const class DetIdToMatrix &,
// 					     double x, double y, int iz,
// 					     int n_x = 5, int n_y = 5);
     void fillData (const std::vector<DetId> &, class TEveCaloDataVec *);

private:
     ElectronDetailView(const ElectronDetailView&); // stop default
     const ElectronDetailView& operator=(const ElectronDetailView&); // stop default
     
     // ---------- member data --------------------------------
     const FWEventItem* m_item;
     void resetCenter() { 
	rotation_center[0] = 0;
	rotation_center[1] = 0;
	rotation_center[2] = 0;
     }
     const EcalRecHitCollection *barrel_hits;
     const EcalRecHitCollection *endcap_hits;
     std::vector<DetId> seed_detids;
     double x_min;
     double x_max;
     double y_min;
     double y_max;
};

#endif
