// -*- C++ -*-
//
// Package:     Core
// Class  :     unittest_fwmodelid
// 
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Fri Jan 18 10:19:07 EST 2008
// $Id: unittest_fwmodelid.cc,v 1.1 2008/01/21 01:17:47 chrjones Exp $
//

// system include files
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <stdexcept>
#include <iostream>

// user include files
#include "Fireworks/Core/interface/FWConfiguration.h"


//
// constants, enums and typedefs
//
BOOST_AUTO_TEST_CASE( fwconfiguration )
{
   FWConfiguration config;
   BOOST_CHECK( 1 == config.version());
   BOOST_CHECK( 0 == config.stringValues());
   BOOST_CHECK( 0 == config.keyValues());
   
   const std::string kValue("1.0");
   config.addValue(kValue);
   BOOST_REQUIRE( 0 != config.stringValues() );
   BOOST_CHECK( 1 == config.stringValues()->size() );
   BOOST_CHECK( 0 == config.keyValues() );
   BOOST_CHECK( kValue == config.value() );
   BOOST_CHECK_THROW(config.addKeyValue("one",FWConfiguration("two")), std::runtime_error);

   //copy constructor
   FWConfiguration config2(config);
   BOOST_CHECK( 1 == config2.version());
   BOOST_REQUIRE( 0 != config2.stringValues() );
   BOOST_CHECK( 1 == config2.stringValues()->size() );
   BOOST_CHECK( 0 == config2.keyValues() );
   BOOST_CHECK( kValue == config2.value() );
   
   //operator=
   FWConfiguration config3;
   config3 = config;
   BOOST_CHECK( 1 == config3.version());
   BOOST_REQUIRE( 0 != config3.stringValues() );
   BOOST_CHECK( 1 == config3.stringValues()->size() );
   BOOST_CHECK( 0 == config3.keyValues() );
   BOOST_CHECK( kValue == config3.value() );
   
   FWConfiguration valueForConst(kValue);
   BOOST_CHECK( 0 == valueForConst.version());
   BOOST_REQUIRE( 0 != valueForConst.stringValues() );
   BOOST_CHECK( 1 == valueForConst.stringValues()->size() );
   BOOST_CHECK( 0 == valueForConst.keyValues() );
   BOOST_CHECK( kValue == valueForConst.value() );
   
   FWConfiguration topConfig;
   topConfig.addKeyValue("first",config);
   BOOST_REQUIRE( 0 != topConfig.keyValues() );
   BOOST_CHECK( 1 == topConfig.keyValues()->size() );
   BOOST_CHECK( 0 == topConfig.stringValues() );
   BOOST_CHECK( std::string("first") == topConfig.keyValues()->front().first );
   BOOST_CHECK( kValue == topConfig.keyValues()->front().second.value() );
   BOOST_CHECK_THROW(topConfig.addValue("one"), std::runtime_error);
   const FWConfiguration* found = topConfig.valueForKey("second");
   BOOST_CHECK(0 == found);
   found = topConfig.valueForKey("first");
   BOOST_REQUIRE(0!=found);
   BOOST_CHECK(found->value()==kValue);
   BOOST_CHECK_THROW(config.valueForKey("blah"), std::runtime_error);
   
   std::cout <<topConfig<<std::endl;
   
}
