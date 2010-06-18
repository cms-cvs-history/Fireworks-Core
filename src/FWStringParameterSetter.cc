// -*- C++ -*-
//
// Package:     Core
// Class  :     FWStringParameterSetter
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Mon Mar 10 11:22:32 CDT 2008
// $Id: FWStringParameterSetter.cc,v 1.1 2009/10/07 12:46:47 dmytro Exp $
//

// system include files
#include "TGLabel.h"
#include "TGTextEntry.h"

#include <assert.h>
#include <iostream>

// user include files
#include "Fireworks/Core/src/FWStringParameterSetter.h"

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
FWStringParameterSetter::FWStringParameterSetter() :
   m_param(0),
   m_widget(0)
{
}

// FWStringParameterSetter::FWStringParameterSetter(const FWStringParameterSetter& rhs)
// {
//    // do actual copying here;
// }

FWStringParameterSetter::~FWStringParameterSetter()
{
}

//
// assignment operators
//
// const FWStringParameterSetter& FWStringParameterSetter::operator=(const FWStringParameterSetter& rhs)
// {
//   //An exception safe implementation is
//   FWStringParameterSetter temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//

void
FWStringParameterSetter::attach(FWParameterBase* iParam)
{
   m_param = dynamic_cast<FWStringParameter*>(iParam);
   assert(0!=m_param);
}

TGFrame*
FWStringParameterSetter::build(TGFrame* iParent)
{
  TGCompositeFrame* frame = new TGHorizontalFrame(iParent,180,20,kFixedWidth);

   m_widget = new TGTextEntry(frame, m_param->name().c_str(), 0);
   m_widget->SetText( m_param->value().c_str() );
   m_widget->Connect("ReturnPressed()", "FWStringParameterSetter", this, "doUpdate()");
   frame->AddFrame(m_widget, new TGLayoutHints(kLHintsExpandX|kLHintsCenterY,2,2,1,1));
   // label
   frame->AddFrame(new TGLabel(frame,m_param->name().c_str()),
                   new TGLayoutHints(kLHintsLeft|kLHintsCenterY,4,0,1,1) );

   return frame;
}

void
FWStringParameterSetter::doUpdate()
{
   assert(0!=m_param);
   assert(0!=m_widget);
   m_param->set(m_widget->GetText());
   update();
}
//
// const member functions
//

//
// static member functions
//
