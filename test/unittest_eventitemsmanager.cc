// -*- C++ -*-
//
// Package:     Core
// Class  :     unittest_changemanager
// 
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Fri Jan 18 10:19:07 EST 2008
// $Id: unittest_eventitemsmanager.cc,v 1.1 2008/02/25 21:32:18 chrjones Exp $
//

// system include files
#include <boost/test/auto_unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/test/test_tools.hpp>
#include "TClass.h"
#include "Cintex/Cintex.h"

// user include files
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#define private public
#include "Fireworks/Core/interface/FWEventItem.h"
#undef private

#include "Fireworks/Core/interface/FWModelChangeManager.h"
#include "Fireworks/Core/interface/FWSelectionManager.h"

#include "Fireworks/Core/interface/FWEventItemsManager.h"
#include "Fireworks/Core/interface/FWConfiguration.h"

//
// constants, enums and typedefs
//
namespace {
   struct Listener {
      Listener(): nMessages_(0), item_(0) {}
      int nMessages_;
      const FWEventItem* item_;
      
      void reset() {
         nMessages_=0;
         item_=0;
      }
      void newItem(const FWEventItem* iItem) {
         ++nMessages_;
         item_=iItem;
      }
   };
}

BOOST_AUTO_TEST_CASE( eventitemmanager )
{
   ROOT::Cintex::Cintex::Enable();
   FWModelChangeManager cm;
   
   FWSelectionManager sm(&cm);
   FWEventItemsManager eim(&cm,&sm);

   Listener listener;
   //NOTE: have to pass a pointer to the listener else the bind will
   // create a copy of the listener and the original one will never
   // 'hear' any signal
   eim.newItem_.connect(boost::bind(&Listener::newItem,&listener,_1));

   TClass* cls=TClass::GetClass("reco::TrackCollection");
   assert(0!=cls);
   
   FWPhysicsObjectDesc tracks("Tracks",
                              cls,
                              FWDisplayProperties(1),
                              "label",
                              "instance",
                              "proc");

   BOOST_REQUIRE(listener.nMessages_==0);
   BOOST_REQUIRE(eim.begin()==eim.end());

   eim.add(tracks);
   BOOST_CHECK(listener.nMessages_==1);
   BOOST_CHECK(eim.end()-eim.begin() == 1);
   const FWEventItem* item= *(eim.begin());
   BOOST_REQUIRE(item!=0);
   BOOST_CHECK(item == listener.item_);
   BOOST_CHECK(item->name() == "Tracks");
   BOOST_CHECK(item->type() == cls);
   BOOST_CHECK(item->defaultDisplayProperties().color() == 1);
   BOOST_CHECK(item->defaultDisplayProperties().isVisible());
   BOOST_CHECK(item->moduleLabel() == "label");
   BOOST_CHECK(item->productInstanceLabel() == "instance");
   BOOST_CHECK(item->processName() == "proc");

   FWConfiguration config;
   eim.addTo(config);
   
   eim.clearItems();   
   listener.reset();
   

   BOOST_REQUIRE(listener.nMessages_==0);
   BOOST_REQUIRE(eim.begin()==eim.end());

   eim.setFrom(config);
   
   BOOST_CHECK(listener.nMessages_==1);
   BOOST_CHECK(eim.end()-eim.begin() == 1);
   item= *(eim.begin());
   BOOST_REQUIRE(item!=0);
   BOOST_CHECK(item == listener.item_);
   BOOST_CHECK(item->name() == "Tracks");
   BOOST_CHECK(item->type() == cls);
   BOOST_CHECK(item->defaultDisplayProperties().color() == 1);
   BOOST_CHECK(item->defaultDisplayProperties().isVisible());
   BOOST_CHECK(item->moduleLabel() == "label");
   BOOST_CHECK(item->productInstanceLabel() == "instance");
   BOOST_CHECK(item->processName() == "proc");
}