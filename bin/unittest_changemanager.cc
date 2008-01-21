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
// $Id$
//

// system include files
#include <boost/test/auto_unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/test/test_tools.hpp>

// user include files
#include "Fireworks/Core/interface/FWModelChangeManager.h"


//
// constants, enums and typedefs
//
namespace {
   struct Listener {
      Listener(): nHeard_(0) {}
      int nHeard_;
      
      void listen(const std::set<FWModelId>& iIds) {
         nHeard_  += iIds.size();
      }
   };
}

BOOST_AUTO_TEST_CASE( changemanager )
{
   FWModelChangeManager cm;
   
   Listener listener;
   //NOTE: have to pass a pointer to the listener else the bind will
   // create a copy of the listener and the original one will never
   // 'hear' any signal
   cm.changes_.connect(boost::bind(&Listener::listen,&listener,_1));
   
   BOOST_CHECK(listener.nHeard_ ==0 );
   cm.changed(FWModelId());
   BOOST_CHECK(listener.nHeard_ ==1 );

   listener.nHeard_ =0;
   cm.beginChanges();
   cm.changed(FWModelId());
   BOOST_CHECK(listener.nHeard_ ==0 );
   cm.endChanges();
   BOOST_CHECK(listener.nHeard_ ==1 );
   
   //sending same ID twice should give only 1 message
   listener.nHeard_ =0;
   cm.beginChanges();
   cm.changed(FWModelId());
   BOOST_CHECK(listener.nHeard_ ==0 );
   cm.changed(FWModelId());
   BOOST_CHECK(listener.nHeard_ ==0 );
   cm.endChanges();
   BOOST_CHECK(listener.nHeard_ ==1 );

   listener.nHeard_ =0;
   cm.beginChanges();
   cm.changed(FWModelId(0,1));
   BOOST_CHECK(listener.nHeard_ ==0 );
   cm.changed(FWModelId(0,2));
   BOOST_CHECK(listener.nHeard_ ==0 );
   cm.endChanges();
   BOOST_CHECK(listener.nHeard_ ==2 );

   listener.nHeard_ =0;
   {
      FWChangeSentry sentry(cm);
      cm.changed(FWModelId(0,1));
      BOOST_CHECK(listener.nHeard_ ==0 );
   }
   BOOST_CHECK(listener.nHeard_ ==1 );
}
