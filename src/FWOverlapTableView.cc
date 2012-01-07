// -*- C++ -*-
//
// Package:     Core
// Class  :     FWOverlapTableView
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  
//         Created:  Wed Jan  4 00:06:35 CET 2012
// $Id: FWOverlapTableView.cc,v 1.1.2.3 2012/01/06 23:19:40 amraktad Exp $
//

// system include files

// user include files
#include "Fireworks/Core/src/FWOverlapTableView.h"
#include "Fireworks/Core/src/FWOverlapTableManager.h"
#include "Fireworks/Core/interface/FWGeoTopNode.h"

#include "Fireworks/Core/src/FWGUIValidatingTextEntry.h"
#include "Fireworks/Core/src/FWValidatorBase.h"

#include "TEveScene.h"
#include "TEveSceneInfo.h"
#include "TEveWindow.h"

#include "TEvePointSet.h"
#include "TEveManager.h"


#include "TGeoVolume.h"
#include "TGeoMatrix.h"
#include "TGeoShape.h"
#include "TGeoBBox.h"
#include "TGeoMatrix.h"

#include "KeySymbols.h"
#include "TGLabel.h"
#include "TGListBox.h"
#include "TGButton.h"

class FWGeoPathValidator : public FWValidatorBase 
{
public:
   FWOverlapTableView* m_browser;
   mutable std::vector<std::string> m_list;

   FWGeoPathValidator( FWOverlapTableView* v) { m_browser = v;}
   virtual ~FWGeoPathValidator() {}


   virtual void fillOptions(const char* iBegin, const char* iEnd, std::vector<std::pair<boost::shared_ptr<std::string>, std::string> >& oOptions) const 
   {
      
   }

   bool isStringValid(std::string& exp) 
   {
     
      return false;
   }
};

//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

FWOverlapTableView::FWOverlapTableView(TEveWindowSlot* iParent, FWColorManager* colMng) : 
   FWGeometryTableViewBase(iParent, FWViewType::kOverlapTable, colMng),
   m_tableManager(0),
   m_eveTopNode(0),
   m_marker(0),
   m_pathEntry(0),
   m_pathValidator(0),
   m_path(this,"Path:", std::string("/cms:World_1/cms:CMSE_1/muonBase:MUON_1/muonBase:MB_1/muonBase:MBWheel_1N_2")),
   m_precision(this, "Precision", 0.001, 1., 10.)
{

   // top row
   TGHorizontalFrame* hp =  new TGHorizontalFrame(m_frame);

   {
      m_viewBox = new FWViewCombo(hp, this);
      hp->AddFrame( m_viewBox,new TGLayoutHints(kLHintsExpandY, 2, 2, 0, 0));
   }
   { 
      hp->AddFrame(new TGLabel(hp, "Path:"), new TGLayoutHints(kLHintsBottom, 10, 0, 0, 2));
      m_pathEntry = new FWGUIValidatingTextEntry(hp);
      m_pathEntry->SetHeight(20);
      m_pathValidator = new FWGeoPathValidator(this);
      m_pathEntry->setValidator(m_pathValidator);
      hp->AddFrame(m_pathEntry, new TGLayoutHints(kLHintsExpandX,  1, 2, 1, 0));
      m_pathEntry->setMaxListBoxHeight(150);
      m_pathEntry->getListBox()->Connect("Selected(int)", "FWOverlapTableView",  this, "pathListCallback()");
      m_pathEntry->Connect("ReturnPressed()", "FWOverlapTableView",  this, "pathTextEntryCallback()");

      gVirtualX->GrabKey( m_pathEntry->GetId(),gVirtualX->KeysymToKeycode((int)kKey_A),  kKeyControlMask, true);
   }

   {
      TGTextButton* rb = new TGTextButton (hp, "recalculate");
      hp->AddFrame(rb, new TGLayoutHints(kLHintsNormal, 2, 2, 0, 0));
      rb->Connect("Clicked()","FWOverlapTableView",this,"recalculate()");
   }
   m_frame->AddFrame(hp,new TGLayoutHints(kLHintsLeft|kLHintsExpandX, 4, 2, 2, 0));
   m_tableManager = new FWOverlapTableManager(this);

 std::cerr << " FWOverlapTableView::initGeometry \n";
   m_marker = new TEvePointSet();
   m_marker->SetMarkerSize(5);
   m_marker->SetMainColor(kRed);
   m_marker->IncDenyDestroy();
   m_tableManager->importOverlaps();


   postConst();
}
//______________________________________________________________________________


FWOverlapTableView::~FWOverlapTableView()
{
   if (m_marker) m_marker->DecDenyDestroy();
}

//______________________________________________________________________________
FWGeometryTableManagerBase* FWOverlapTableView::getTableManager()
{
   return m_tableManager;
}

//______________________________________________________________________________
void FWOverlapTableView::assertEveGeoElement()
{
   if (!m_eveTopNode )
   {
      m_eveTopNode = new  FWEveOverlap(this);
      m_eveTopNode->SetElementNameTitle("overlapNode", "opverlapNodetitle");
      m_eveTopNode->IncDenyDestroy();
      m_eveTopNode->AddElement( m_marker);
      gEve->AddElement(m_eveTopNode);
   }
}
//______________________________________________________________________________


TEveElement* FWOverlapTableView::getEveGeoElement() const
{
   return m_eveTopNode;
}

//______________________________________________________________________________
void FWOverlapTableView::pathTextEntryCallback()
{
   std::cout << "text entry click ed " <<m_pathEntry->GetText() << std::endl ;
   recalculate();
}

//______________________________________________________________________________

void FWOverlapTableView::pathListCallback()
{ 
    std::cout << "list click ed \n" ;
}
//______________________________________________________________________________

void FWOverlapTableView::recalculate()
{
   m_path.set(m_pathEntry->GetText());
   m_tableManager->importOverlaps();
   refreshTable3D();
}

//______________________________________________________________________________
void FWOverlapTableView::setFrom(const FWConfiguration& iFrom)
{
   FWGeometryTableViewBase::setFrom(iFrom);
   m_pathEntry->SetText(m_path.value().c_str());
}
