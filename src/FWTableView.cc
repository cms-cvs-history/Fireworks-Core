// -*- C++ -*-
//
// Package:     Core
// Class  :     FWTableView
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Thu Feb 21 11:22:41 EST 2008
// $Id: FWTableView.cc,v 1.4.2.3 2009/04/20 16:33:36 jmuelmen Exp $
//

// system include files
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/numeric/conversion/converter.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>

// FIXME
// need camera parameters
#define private public
#include "TGLPerspectiveCamera.h"
#undef private


#include "TRootEmbeddedCanvas.h"
#include "THStack.h"
#include "TCanvas.h"
#include "TClass.h"
#include "TH2F.h"
#include "TView.h"
#include "TColor.h"
#include "TEveScene.h"
#include "TGLViewer.h"
//EVIL, but only way I can avoid a double delete of TGLEmbeddedViewer::fFrame
#define private public
#include "TGLEmbeddedViewer.h"
#undef private
#include "TGComboBox.h"
#include "TGLabel.h"
#include "TGTextView.h"
#include "TGTextEntry.h"
#include "TEveViewer.h"
#include "TEveManager.h"
#include "TEveWindow.h"
#include "TEveElement.h"
#include "TEveCalo.h"
#include "TEveElement.h"
#include "TEveRGBAPalette.h"
#include "TEveLegoEventHandler.h"
#include "TGLWidget.h"
#include "TGLScenePad.h"
#include "TGLFontManager.h"
#include "TEveTrans.h"
#include "TGeoTube.h"
#include "TEveGeoNode.h"
#include "TEveStraightLineSet.h"
#include "TEveText.h"
#include "TGeoArb8.h"

// user include files
#include "Fireworks/Core/interface/FWModelChangeManager.h"
#include "Fireworks/Core/interface/FWSelectionManager.h"
#include "Fireworks/Core/interface/FWTableView.h"
#include "Fireworks/Core/interface/FWTableViewManager.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/FWEveValueScaler.h"
#include "Fireworks/Core/interface/FWConfiguration.h"
#include "Fireworks/Core/interface/BuilderUtils.h"
#include "Fireworks/Core/interface/FWExpressionEvaluator.h"
#include "Fireworks/TableWidget/interface/FWTableWidget.h"

//
// constants, enums and typedefs
//

//
// static data member definitions
//
//double FWTableView::m_scale = 1;

//
// constructors and destructor
//
FWTableView::FWTableView (TEveWindowSlot* iParent, const FWTableViewManager *manager)
     : m_iColl(-1),
       m_manager(manager),
       m_tableManager(this),
       m_tableWidget(0)
{
//      TGLayoutHints *tFrameHints = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);
     const int width = 100, height = 100;
//      TGVerticalFrame *topframe = new TGVerticalFrame(iParent->GetEveFrame(), 100, 100);
     m_frame = iParent->MakeFrame(0);
     TGCompositeFrame *frame = m_frame->GetGUICompositeFrame();
     m_vert = new TGVerticalFrame(frame);
     frame->AddFrame(m_vert, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
     TGHorizontalFrame *header = new TGHorizontalFrame(m_vert);
     m_vert->AddFrame(header, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
     TGLabel *label = new TGLabel(header, "Collection");
     header->AddFrame(label, new TGLayoutHints(kLHintsLeft));
     m_collection = new TGComboBox(header);
     updateItems();
     header->AddFrame(m_collection, new TGLayoutHints(kLHintsLeft | kLHintsExpandX | kLHintsExpandY));
     m_collection->Connect("Selected(Int_t)", "FWTableView", this, "selectCollection(Int_t)");
     m_collection->Select(2, true);
     m_column_control = new TGVerticalFrame(m_vert);
     m_vert->AddFrame(m_column_control, new TGLayoutHints(kLHintsExpandX));
     TGLabel *column_control_label = new TGLabel(m_column_control, "Column editor");
     m_column_control->AddFrame(column_control_label, new TGLayoutHints(kLHintsExpandX));
     TGHorizontalFrame *column_control_fields = new TGHorizontalFrame(m_column_control);
     m_column_control->AddFrame(column_control_fields, new TGLayoutHints(kLHintsExpandX));
     TGTextEntry *column_name_field = new TGTextEntry(column_control_fields);
     TGTextEntry *column_expr_field = new TGTextEntry(column_control_fields);
     TGTextEntry *column_prec_field = new TGTextEntry(column_control_fields);
     column_control_fields->AddFrame(column_name_field, new TGLayoutHints(kLHintsExpandX));
     column_control_fields->AddFrame(column_expr_field, new TGLayoutHints(kLHintsExpandX));
     column_control_fields->AddFrame(column_prec_field, new TGLayoutHints(kLHintsExpandX));
     m_tableWidget = new FWTableWidget(&m_tableManager, m_vert);
     m_tableWidget->SetBackgroundColor(gVirtualX->GetPixel(kBlack));
     m_tableWidget->SetHeaderBackgroundColor(gVirtualX->GetPixel(kWhite));
     m_tableWidget->Connect("rowClicked(Int_t,Int_t,Int_t)", "FWTableView",
			    this, "modelSelected(Int_t,Int_t,Int_t)");
     m_vert->AddFrame(m_tableWidget, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
     frame->MapSubwindows();
     m_vert->HideFrame(m_column_control);
     frame->Layout();
     frame->MapWindow();
//      frame->Resize(0,0);

//      iParent->GetEveFrame()->Layout();
//      iParent->GetEveFrame()->MapSubwindows();
// #if 1
//      TGHorizontalFrame *buttons = new TGHorizontalFrame(topframe, width, 25, kFixedHeight);
//      topframe->AddFrame(buttons, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY));
//      TGTextEntry *text = new TGTextEntry(buttons, "Collection");
//      buttons->AddFrame(text, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY));
// #else
//      TGTextView *text = new TGTextView(topframe, width, height, "Blah blah blah blah blah");
//      topframe->AddFrame(text, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY));
//      topframe->Layout();
//      topframe->MapSubwindows();
// //      topframe->MapWindow();
// #endif
//      iParent->GetEveFrame()->AddFrame(frame,tFrameHints);
// //      parent->HideFrame(frame);
//      iParent->GetEveFrame()->Layout();
//      iParent->GetEveFrame()->MapSubwindows();
//      iParent->GetEveFrame()->MapWindow();
#if 0
   TEveViewer* nv = new TEveViewer(staticTypeName().c_str());
   m_embeddedViewer =  nv->SpawnGLEmbeddedViewer();
   iParent->ReplaceWindow(nv);

   TGLEmbeddedViewer* ev = m_embeddedViewer;
   ev->SetCurrentCamera(TGLViewer::kCameraPerspXOZ);
   //? ev->SetEventHandler(new TTableEventHandler("Lego", ev->GetGLWidget(), ev));
   m_cameraMatrix = const_cast<TGLMatrix*>(&(ev->CurrentCamera().GetCamTrans()));
   m_cameraMatrixBase = const_cast<TGLMatrix*>(&(ev->CurrentCamera().GetCamBase()));
   if ( TGLPerspectiveCamera* camera =
        dynamic_cast<TGLPerspectiveCamera*>(&(ev->CurrentCamera())) )
      m_cameraFOV = &(camera->fFOV);

   TEveScene* ns = gEve->SpawnNewScene(staticTypeName().c_str());
   m_scene = ns;
   nv->AddScene(ns);
   m_viewer=nv;
   gEve->AddElement(nv, gEve->GetViewers());
   gEve->AddElement(list,ns);
   gEve->AddToListTree(list, kTRUE);
#endif
}

FWTableView::~FWTableView()
{
     m_frame->DestroyWindowAndSlot();
}

void
FWTableView::setFrom(const FWConfiguration& iFrom)
{
   // take care of parameters
   FWConfigurableParameterizable::setFrom(iFrom);
}

void
FWTableView::setBackgroundColor(Color_t iColor) 
{
     m_tableWidget->SetBackgroundColor(iColor);
//    m_viewer->GetGLViewer()->SetClearColor(iColor);
}

//
// const member functions
//
TGFrame*
FWTableView::frame() const
{
     return 0;
//    return m_embeddedViewer->GetFrame();
}

const std::string&
FWTableView::typeName() const
{
   return staticTypeName();
}

void
FWTableView::addTo(FWConfiguration& iTo) const
{
   // take care of parameters
   FWConfigurableParameterizable::addTo(iTo);
}

void
FWTableView::saveImageTo(const std::string& iName) const
{
//    bool succeeded = m_viewer->GetGLViewer()->SavePicture(iName.c_str());
//    if(!succeeded) {
//       throw std::runtime_error("Unable to save picture");
//    }
   static int hide_view = true;
   printf("blah\n");
   if (hide_view) {
	m_vert->HideFrame(m_column_control);
	m_vert->Layout();
   } else {
	m_vert->ShowFrame(m_column_control);
	m_vert->Layout();
   }
   hide_view = not hide_view;
}

void FWTableView::updateItems ()
{
     m_collection->RemoveAll();
     for (std::vector<const FWEventItem *>::const_iterator it = m_manager->items().begin(), 
	       itEnd = m_manager->items().end();
	  it != itEnd; ++it) {
	  m_collection->AddEntry((*it)->name().c_str(), it - m_manager->items().begin());
     }
}

void FWTableView::updateEvaluators ()
{
     if (m_iColl == -1) {
	  printf("what should I do with collection -1?\n");
	  return;
     }
     const FWEventItem *item = m_manager->items()[m_iColl];
     std::vector<FWExpressionEvaluator> &ev = m_evaluators;
     ev.clear();
     if (m_manager->tableFormats(item->modelType()->GetName()) == 
	 m_manager->m_tableFormats.end()) {
	  printf("No table format for objects of this type\n");
	  return;
     }
     for (std::vector<FWTableViewManager::TableEntry>::const_iterator 
	       i = m_manager->tableFormats(item->modelType()->GetName())->second.begin(),
	       end = m_manager->tableFormats(item->modelType()->GetName())->second.end();
	  i != end; ++i) {
	  try {
	       ev.push_back(FWExpressionEvaluator(i->expression, item->modelType()->GetName()));
	  } catch (...) {
	       printf("expression %s is not valid, skipping\n", i->expression.c_str());
	       ev.push_back(FWExpressionEvaluator("0", item->modelType()->GetName()));
	  }
     }
     printf("Got evaluators\n");
}

const FWEventItem *FWTableView::item () const
{
     if (m_iColl == -1)
	  return 0;
     return m_manager->items()[m_iColl];
}

void FWTableView::display ()
{
     if (m_iColl == -1) {
	  printf("what should I do with collection -1?\n");
	  return;
     }
     const FWEventItem *item = m_manager->items()[m_iColl];
     updateEvaluators();
     m_tableManager.dataChanged();
//      std::vector<FWExpressionEvaluator> &ev = m_evaluators;
//      for (unsigned int i = 0; i < item->size(); ++i) {
// 	  for (unsigned int j = 0; j < ev.size(); ++j) {
// 	       printf("%s = %f\t", (*m_manager->tableFormats(item->modelType()->GetName())).second[j].name.c_str(),
// 		      ev[j].evalExpression(item->modelData(i)));
// 	  }
// 	  printf("\n");
//      }
//      fflush(stdout);
}


void FWTableView::selectCollection (Int_t i_coll)
{
     printf("selected collection %d, ", i_coll);
     const FWEventItem *item = m_manager->items()[i_coll];
     printf("%s\n", item->modelType()->GetName());
     m_iColl = i_coll;
     display();
}

void FWTableView::modelSelected(Int_t iRow,Int_t iButton,Int_t iKeyMod)
{
     if(iKeyMod & kKeyControlMask) {      
	  item()->toggleSelect(iRow);
     } else {
	  FWChangeSentry sentry(*(item()->changeManager()));
	  item()->selectionManager()->clearSelection();
	  item()->select(iRow);
     }
}

//
// static member functions
//
const std::string&
FWTableView::staticTypeName()
{
   static std::string s_name("Table");
   return s_name;
}
