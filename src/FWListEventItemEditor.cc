// -*- C++ -*-
//
// Package:     Core
// Class  :     FWListEventItemEditor
// 
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Mon Mar  3 09:36:01 EST 2008
// $Id$
//

// system include files
#include "TGTextEntry.h"
#include "TGButton.h"

// user include files
#include "Fireworks/Core/src/FWListEventItemEditor.h"
#include "Fireworks/Core/src/FWListEventItem.h"
#include "Fireworks/Core/interface/FWEventItem.h"


//
// constants, enums and typedefs
//
ClassImp(FWListEventItemEditor)

//
// static data member definitions
//

//
// constructors and destructor
//
FWListEventItemEditor::FWListEventItemEditor(const TGWindow* p,
                                             Int_t width,
                                             Int_t height,
                                             UInt_t options,
                                             Pixel_t back):
TGedFrame(p, width, height, options | kVerticalFrame, back)
{
   MakeTitle("FWListEventItem");
   TGGroupFrame* vf = new TGGroupFrame(this,"Object Filter",kVerticalFrame);

   m_filterExpression = new TGTextEntry(vf);
   vf->AddFrame(m_filterExpression, new TGLayoutHints(kLHintsExpandX,0,5,5,5));

   m_filterExpression->Connect("ReturnPressed()","FWListEventItemEditor",this,"runFilter()");
   m_filterRunExpressionButton = new TGTextButton(vf,"Run Filter");
   vf->AddFrame(m_filterRunExpressionButton);
   m_filterRunExpressionButton->Connect("Clicked()","FWListEventItemEditor",this,"runFilter()");
   AddFrame(vf, new TGLayoutHints(kLHintsTop, 0, 0, 0, 0));
}

// FWListEventItemEditor::FWListEventItemEditor(const FWListEventItemEditor& rhs)
// {
//    // do actual copying here;
// }

FWListEventItemEditor::~FWListEventItemEditor()
{
}

//
// assignment operators
//
// const FWListEventItemEditor& FWListEventItemEditor::operator=(const FWListEventItemEditor& rhs)
// {
//   //An exception safe implementation is
//   FWListEventItemEditor temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void
FWListEventItemEditor::SetModel(TObject* iObj)
{
   m_item = dynamic_cast<FWListEventItem*>(iObj);
   m_filterExpression->SetText(m_item->eventItem()->filterExpression().c_str());
   assert(0!=m_item);
   
}

void
FWListEventItemEditor::runFilter()
{
   m_item->eventItem()->setFilterExpression(m_filterExpression->GetText());
}
//
// const member functions
//

//
// static member functions
//
