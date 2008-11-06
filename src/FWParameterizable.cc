// -*- C++ -*-
//
// Package:     Core
// Class  :     FWParameterizable
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Sat Feb 23 13:36:27 EST 2008
// $Id: FWParameterizable.cc,v 1.1 2008/03/11 02:43:55 chrjones Exp $
//

// system include files

// user include files
#include "Fireworks/Core/interface/FWParameterizable.h"
#include "Fireworks/Core/interface/FWParameterBase.h"


//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
FWParameterizable::FWParameterizable()
{
}

// FWParameterizable::FWParameterizable(const FWParameterizable& rhs)
// {
//    // do actual copying here;
// }

FWParameterizable::~FWParameterizable()
{
}

//
// assignment operators
//
// const FWParameterizable& FWParameterizable::operator=(const FWParameterizable& rhs)
// {
//   //An exception safe implementation is
//   FWParameterizable temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void
FWParameterizable::add(FWParameterBase* iParam)
{
   m_parameters.push_back(iParam);
}

//
// const member functions
//

//
// static member functions
//
