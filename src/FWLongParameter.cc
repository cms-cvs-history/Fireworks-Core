// -*- C++ -*-
//
// Package:     Core
// Class  :     FWLongParameter
// 
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Fri Mar  7 14:36:41 EST 2008
// $Id: FWLongParameter.cc,v 1.1 2008/03/11 02:43:55 chrjones Exp $
//

// system include files
#include <assert.h>
#include <sstream>

// user include files
#include "Fireworks/Core/interface/FWLongParameter.h"
#include "Fireworks/Core/interface/FWConfiguration.h"


//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
FWLongParameter::FWLongParameter(FWParameterizable* iParent,
                                     const std::string& iName,
                                     long iDefault,
                                     long iMin,
                                     long iMax):
FWParameterBase(iParent,iName),
m_value(iDefault),
m_min(iMin),
m_max(iMax)
{
}

// FWLongParameter::FWLongParameter(const FWLongParameter& rhs)
// {
//    // do actual copying here;
// }
/*
FWLongParameter::~FWLongParameter()
{
}
*/
//
// assignment operators
//
// const FWLongParameter& FWLongParameter::operator=(const FWLongParameter& rhs)
// {
//   //An exception safe implementation is
//   FWLongParameter temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void 
FWLongParameter::setFrom(const FWConfiguration& iFrom)
{
   assert(0!=iFrom.valueForKey(name()));
   std::istringstream s(iFrom.valueForKey(name())->value());
   s>>m_value;
   changed_(m_value);
}

void 
FWLongParameter::set(long iValue)
{
   m_value = iValue;
   changed_(iValue);
}

//
// const member functions
//
void 
FWLongParameter::addTo(FWConfiguration& iTo) const
{
   std::ostringstream s;
   s<<m_value;
   iTo.addKeyValue(name(),FWConfiguration(s.str()));
}

//
// static member functions
//
