// -*- C++ -*-
//
// Package:     Core
// Class  :     FWRhoPhiZView
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Tue Feb 19 10:33:25 EST 2008
// $Id: FWRhoPhiZView.cc,v 1.57 2010/03/12 20:07:52 amraktad Exp $
//

#define private public
#include "TGLOrthoCamera.h"
#undef private

#include <stdexcept>

// system include files
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/numeric/conversion/converter.hpp>
#include <iostream>
#include <sstream>
#include "TEveProjectionManager.h"
#include "TEveScene.h"
#include "TGLViewer.h"
#include "RVersion.h"
//EVIL, but only way I can avoid a double delete of TGLEmbeddedViewer::fFrame
#define private public
#include "TGLEmbeddedViewer.h"
#undef private
#include "TEveViewer.h"
#include "TEveManager.h"
#include "TEveWindow.h"
#include "TClass.h"
#include "TEveElement.h"
#include "TEveProjectionBases.h"
#include "TEvePolygonSetProjected.h"
#include "TEveProjections.h"
#include "TEveCalo.h"
#include "TEveProjectionAxes.h"
#include "TEveScalableStraightLineSet.h"
#include "TH2F.h"
#include "THStack.h"

// user include files
#include "Fireworks/Core/interface/FWRhoPhiZView.h"
#include "Fireworks/Core/interface/FWRhoPhiZViewManager.h"
#include "Fireworks/Core/interface/FWConfiguration.h"
#include "Fireworks/Core/interface/FWColorManager.h"
#include "Fireworks/Core/interface/TEveElementIter.h"

#include "Fireworks/Core/interface/FWEventAnnotation.h"
#include "Fireworks/Core/interface/CmsAnnotation.h"
#include "Fireworks/Core/interface/FWGLEventHandler.h"
#include "Fireworks/Core/interface/FWViewContextMenuHandlerGL.h"

//
// constants, enums and typedefs
//
static TEveElement* doReplication(TEveProjectionManager* iMgr, TEveElement* iFrom, TEveElement* iParent) {
   static const TEveException eh("FWRhoPhiZView::doReplication ");
   TEveElement  *new_re = 0;
   TEveProjected   *new_pr = 0;
   TEveProjected *pble   = dynamic_cast<TEveProjected*>(iFrom);
   //std::cout << typeid(*iFrom).name() <<std::endl;
   if (pble)
   {
      new_re = (TEveElement*) TClass::GetClass( typeid(*iFrom) )->New();
      assert(0!=new_re);
      new_pr = dynamic_cast<TEveProjected*>(new_re);
      assert(0!=new_pr);
      new_pr->SetProjection(iMgr, pble->GetProjectable());
      new_pr->SetDepth(iMgr->GetCurrentDepth());

      new_re->SetMainTransparency(iFrom->GetMainTransparency());
      new_re->SetMainColor(iFrom->GetMainColor());
      if ( TEvePolygonSetProjected* poly = dynamic_cast<TEvePolygonSetProjected*>(new_re) )
         poly->SetLineColor(dynamic_cast<TEvePolygonSetProjected*>(iFrom)->GetLineColor());

   }
   else
   {
      new_re = new TEveElementList;
   }
   new_re->SetElementNameTitle(iFrom->GetElementName(),
                               iFrom->GetElementTitle());
   new_re->SetRnrSelf     (iFrom->GetRnrSelf());
   new_re->SetRnrChildren(iFrom->GetRnrChildren());
   new_re->SetPickable(iFrom->IsPickable());
   iParent->AddElement(new_re);

   for (TEveElement::List_i i=iFrom->BeginChildren(); i!=iFrom->EndChildren(); ++i)
      doReplication(iMgr,*i, new_re);
   return new_re;
}

//
// constructors and destructor
//
FWRhoPhiZView::FWRhoPhiZView(TEveWindowSlot* iParent,const std::string& iName, const TEveProjection::EPType_e& iProjType) :
   m_projType(iProjType),
   m_typeName(iName),
   m_caloScale(1),
   m_axes(),
   m_overlayEventInfo(0),
   m_overlayLogo(0),
   m_overlayEventInfoLevel(this, "Overlay Event Info", 1l, 0l, 3l),
   m_drawCMSLogo(this,"Show Logo",false),
   m_caloDistortion(this,"Calo compression",1.0,0.01,10.),
   m_muonDistortion(this,"Muon compression",0.2,0.01,10.),
   m_showProjectionAxes(this,"Show projection axes", false),
   m_compressMuon(this,"Compress detectors",false),
   m_caloFixedScale(this,"Calo scale (GeV/meter)",2.,0.001,100.),
   m_caloAutoScale(this,"Calo auto scale",false),
   m_lineWidth(this,"Line width",1.0,1.0,10.0),
   m_smoothLine(this,"Smooth line",false),
   m_showHF(0),
   m_showEndcaps(0),
   m_cameraZoom(0),
   m_cameraMatrix(0)
{
   TEveViewer* nv = new TEveViewer(iName.c_str());
#if ROOT_VERSION_CODE >= ROOT_VERSION(5,25,4)
   m_embeddedViewer =  nv->SpawnGLEmbeddedViewer(0);
#else
   m_embeddedViewer =  nv->SpawnGLEmbeddedViewer();
#endif
   iParent->ReplaceWindow(nv);
   gEve->GetViewers()->AddElement(nv);

   TEveScene* ns = gEve->SpawnNewScene(iName.c_str());
   m_scene.reset(ns);
   nv->AddScene(ns);
   m_viewer.reset(nv);

   m_viewContextMenu.reset(new FWViewContextMenuHandlerGL(nv));

   m_projMgr.reset(new TEveProjectionManager(iProjType));
   m_projMgr->SetImportEmpty(kTRUE);
   if ( iProjType == TEveProjection::kPT_RPhi ) {
      // compression
      m_projMgr->GetProjection()->AddPreScaleEntry(0, 130, 1.0);
      m_projMgr->GetProjection()->AddPreScaleEntry(0, 300, 0.2);
      // projection specific parameters
      m_showEndcaps = new FWBoolParameter(this,"Show calo endcaps", true);
      m_showEndcaps->changed_.connect(  boost::bind(&FWRhoPhiZView::updateCaloParameters, this) );
      m_showHF = new FWBoolParameter(this,"Show HF", true);
      m_showHF->changed_.connect(  boost::bind(&FWRhoPhiZView::updateCaloParameters, this) );
   } else {
      m_projMgr->GetProjection()->AddPreScaleEntry(0, 130, 1.0);
      m_projMgr->GetProjection()->AddPreScaleEntry(1, 310, 1.0);
      m_projMgr->GetProjection()->AddPreScaleEntry(0, 370, 0.2);
      m_projMgr->GetProjection()->AddPreScaleEntry(1, 580, 0.2);
   }
   gEve->AddToListTree(m_projMgr.get(),kTRUE); // debug

   FWGLEventHandler* eh = new FWGLEventHandler((TGWindow*)m_embeddedViewer->GetGLWidget(), (TObject*)m_embeddedViewer);
   m_embeddedViewer->SetEventHandler(eh);
   eh->openSelectedModelContextMenu_.connect(openSelectedModelContextMenu_);

   m_embeddedViewer->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
   if ( TGLOrthoCamera* camera = dynamic_cast<TGLOrthoCamera*>( &(m_embeddedViewer->CurrentCamera()) ) ) {
      m_cameraZoom = &(camera->fZoom);
      m_cameraMatrix = const_cast<TGLMatrix*>(&(camera->GetCamTrans()));
      camera->SetZoomMax(1e6);
   }

   m_axes.reset(new TEveProjectionAxes(m_projMgr.get()));
   m_showProjectionAxes.changed_.connect(boost::bind(&FWRhoPhiZView::showProjectionAxes,this));
   ns->AddElement(m_axes.get());

   gEve->AddElement(m_projMgr.get(), ns);
   
   m_overlayEventInfo = new FWEventAnnotation(m_embeddedViewer);
   m_overlayEventInfoLevel.changed_.connect(boost::bind(&FWEventAnnotation::setLevel,m_overlayEventInfo, _1));
   m_overlayLogo = new CmsAnnotation(m_embeddedViewer, 0.02, 0.98);
   m_drawCMSLogo.changed_.connect(boost::bind(&CmsAnnotation::setVisible,m_overlayLogo, _1));
   m_caloDistortion.changed_.connect(boost::bind(&FWRhoPhiZView::doDistortion,this));
   m_muonDistortion.changed_.connect(boost::bind(&FWRhoPhiZView::doDistortion,this));
   m_compressMuon.changed_.connect(boost::bind(&FWRhoPhiZView::doCompression,this,_1));
   m_caloFixedScale.changed_.connect( boost::bind(&FWRhoPhiZView::updateScaleParameters, this) );
   m_caloAutoScale.changed_.connect(  boost::bind(&FWRhoPhiZView::updateScaleParameters, this) );
   m_lineWidth.changed_.connect(boost::bind(&FWRhoPhiZView::lineWidthChanged,this));
   m_smoothLine.changed_.connect(boost::bind(&FWRhoPhiZView::lineSmoothnessChanged,this));

}

FWRhoPhiZView::~FWRhoPhiZView()
{
   m_axes.destroyElement();
   m_projMgr.destroyElement();

   m_scene->Destroy();
   m_viewer->DestroyWindowAndSlot();
}

//
// assignment operators
//
// const FWRhoPhiZView& FWRhoPhiZView::operator=(const FWRhoPhiZView& rhs)
// {
//   //An exception safe implementation is
//   FWRhoPhiZView temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//

void
FWRhoPhiZView::doDistortion()
{
   if ( m_projType == TEveProjection::kPT_RPhi ) {
      m_projMgr->GetProjection()->ChangePreScaleEntry(0,1,m_caloDistortion.value());
      m_projMgr->GetProjection()->ChangePreScaleEntry(0,2,m_muonDistortion.value());
   } else {
      m_projMgr->GetProjection()->ChangePreScaleEntry(0,1,m_caloDistortion.value());
      m_projMgr->GetProjection()->ChangePreScaleEntry(0,2,m_muonDistortion.value());
      m_projMgr->GetProjection()->ChangePreScaleEntry(1,1,m_caloDistortion.value());
      m_projMgr->GetProjection()->ChangePreScaleEntry(1,2,m_muonDistortion.value());
   }
   m_projMgr->UpdateName();
   m_projMgr->ProjectChildren();
   gEve->Redraw3D();
}

void
FWRhoPhiZView::doCompression(bool flag)
{
   m_projMgr->GetProjection()->SetUsePreScale(flag);
   m_projMgr->UpdateName();
   m_projMgr->ProjectChildren();
   gEve->Redraw3D();
}

void
FWRhoPhiZView::setBackgroundColor(Color_t iColor)
{
   FWColorManager::setColorSetViewer(m_viewer->GetGLViewer(), iColor);
}

void
FWRhoPhiZView::resetCamera()
{
   //this is needed to get EVE to transfer the TEveElements to GL so the camera reset will work
   m_scene->Repaint();
   m_viewer->Redraw(kTRUE);
   m_embeddedViewer->ResetCurrentCamera();
}

void
FWRhoPhiZView::destroyElements()
{
   m_projMgr->DestroyElements();
   std::for_each(m_geom.begin(),m_geom.end(),
                 boost::bind(&TEveProjectionManager::AddElement,
                             m_projMgr.get(),
                             _1));
}
void
FWRhoPhiZView::replicateGeomElement(TEveElement* iChild)
{ 
   m_geom.push_back(doReplication(m_projMgr.get(),iChild,m_projMgr.get()));
   m_projMgr->AssertBBox();
   m_projMgr->ProjectChildrenRecurse(m_geom.back());
}

//returns the new element created from this import
TEveElement*
FWRhoPhiZView::importElements(TEveElement* iChildren, float iLayer, TEveElement* iProjectedParent)
{
   float oldLayer = m_projMgr->GetCurrentDepth();
   m_projMgr->SetCurrentDepth(iLayer);
   //make sure current depth is reset even if an exception is thrown
   boost::shared_ptr<TEveProjectionManager> sentry(m_projMgr.get(),
                                                   boost::bind(&TEveProjectionManager::SetCurrentDepth,
                                                               _1,oldLayer));
   m_projMgr->ImportElements(iChildren,iProjectedParent);
   TEveElement* lastChild = m_projMgr->LastChild();
   updateCalo( lastChild, true );
   updateCaloLines( lastChild );

   return lastChild;
}

void
FWRhoPhiZView::updateCalo(TEveElement* iParent, bool dataChanged)
{
   TEveElementIter child(iParent);
   while ( TEveElement* element = child.current() )
   {
      if ( TEveCalo2D* calo2d = dynamic_cast<TEveCalo2D*>(element) ) {
         calo2d->SetValueIsColor(kFALSE);
         calo2d->SetMaxTowerH( 150 );
         calo2d->SetMaxValAbs( 150/m_caloFixedScale.value() );
         calo2d->SetScaleAbs( !m_caloAutoScale.value() );
         if ( dataChanged ) calo2d->GetData()->DataChanged();
         double eta_range = 5.191;
         if ( m_showHF && !m_showHF->value() ) eta_range = 3.0;
         if ( m_showEndcaps && !m_showEndcaps->value() ) eta_range = 1.479;
         calo2d->SetEta(-eta_range,eta_range);
         calo2d->ElementChanged();
         m_caloScale = calo2d->GetValToHeight();
         if ( m_axes ) {
            if ( m_showProjectionAxes.value() )
               m_axes->SetRnrState(kTRUE);
            else
               m_axes->SetRnrState(kFALSE);
         }
      }
      child.next();
   }
}

void
FWRhoPhiZView::updateCaloLines(TEveElement* iParent)
{
   TEveElementIter child(iParent);
   while ( TEveElement* element = child.current() )
   {
      if ( TEveStraightLineSetProjected* projected = dynamic_cast<TEveStraightLineSetProjected*>(element) )
         if ( TEveScalableStraightLineSet* line = dynamic_cast<TEveScalableStraightLineSet*>(projected->GetProjectable()) )
         {
            line->SetScale( m_caloScale );
            line->ElementChanged();
            projected->UpdateProjection();
            projected->ElementChanged();
         }
      child.next();
   }
}


void
FWRhoPhiZView::setFrom(const FWConfiguration& iFrom)
{
   // take care of parameters
   FWConfigurableParameterizable::setFrom(iFrom);

   // retrieve camera parameters
   // zoom
   assert(m_cameraZoom);
   std::string zoomName("cameraZoom"); zoomName += typeName();
   assert( 0!=iFrom.valueForKey(zoomName) );
   std::istringstream s(iFrom.valueForKey(zoomName)->value());
   s>>(*m_cameraZoom);

   // transformation matrix
   assert(m_cameraMatrix);
   std::string matrixName("cameraMatrix");
   for ( unsigned int i = 0; i < 16; ++i ) {
      std::ostringstream os;
      os << i;
      const FWConfiguration* value = iFrom.valueForKey( matrixName + os.str() + typeName() );
      assert( value );
      std::istringstream s(value->value());
      s>>((*m_cameraMatrix)[i]);
   }
   {
      assert( m_overlayEventInfo);
      m_overlayEventInfo->setFrom(iFrom);
   }
   {
      assert( m_overlayLogo);
      m_overlayLogo->setFrom(iFrom);
   }
   m_viewer->GetGLViewer()->RequestDraw();
}


//
// const member functions
//
TGFrame*
FWRhoPhiZView::frame() const
{
   return m_embeddedViewer->GetFrame();
}

const std::string&
FWRhoPhiZView::typeName() const
{
   return m_typeName;
}

void
FWRhoPhiZView::addTo(FWConfiguration& iTo) const
{
   // take care of parameters
   FWConfigurableParameterizable::addTo(iTo);

   // store camera parameters
   // zoom
   assert(m_cameraZoom);
   std::ostringstream s;
   s<<(*m_cameraZoom);
   std::string name("cameraZoom");
   iTo.addKeyValue(name+typeName(),FWConfiguration(s.str()));

   // transformation matrix
   assert(m_cameraMatrix);
   std::string matrixName("cameraMatrix");
   for ( unsigned int i = 0; i < 16; ++i ) {
      std::ostringstream osIndex;
      osIndex << i;
      std::ostringstream osValue;
      osValue << (*m_cameraMatrix)[i];
      iTo.addKeyValue(matrixName+osIndex.str()+typeName(),FWConfiguration(osValue.str()));
   }
   { 
      assert ( m_overlayEventInfo );
      m_overlayEventInfo->addTo(iTo);
   }
   { 
      assert ( m_overlayLogo );
      m_overlayLogo->addTo(iTo);
   }   
}

void
FWRhoPhiZView::saveImageTo(const std::string& iName) const
{
   bool succeeded = m_viewer->GetGLViewer()->SavePicture(iName.c_str());
   if(!succeeded) {
      throw std::runtime_error("Unable to save picture to file");
   }
}

FWViewContextMenuHandlerBase*
FWRhoPhiZView::contextMenuHandler() const {
   return (FWViewContextMenuHandlerBase*)m_viewContextMenu.get();
}


void
FWRhoPhiZView::updateScaleParameters()
{
   updateCalo(m_projMgr.get());
   updateCaloLines(m_projMgr.get());
   gEve->Redraw3D();
}

void
FWRhoPhiZView::updateCaloParameters()
{
   updateCalo(m_projMgr.get());
   gEve->Redraw3D();
}
/*
   void
   FWRhoPhiZView::setMinEnergy( TEveCalo2D* calo, double value, std::string name )
   {
   if ( !calo->GetData() ) return;

   TEveCaloDataHist* data = (TEveCaloDataHist*)data;
   TList* hists = data->GetStack()->GetHists();
   for (Int_t i =0; i < hists->GetSize(); ++i)
   {
      std::string histName(hists->At(i)->GetName());
      if ( histName.find(name,0) != std::string::npos)
      {
         data->SetSliceThreshold(i, value);
         break;
      }
   }
   }
 */
void FWRhoPhiZView::showProjectionAxes( )
{
   if ( !m_axes ) return; // just in case
   if ( m_showProjectionAxes.value() )
      m_axes->SetRnrState(kTRUE);
   else
      m_axes->SetRnrState(kFALSE);
   gEve->Redraw3D();
}

void
FWRhoPhiZView::lineWidthChanged()
{
   m_embeddedViewer->SetLineScale(m_lineWidth.value());
   m_embeddedViewer->RequestDraw();
}

void
FWRhoPhiZView::lineSmoothnessChanged()
{
   m_embeddedViewer->SetSmoothLines(m_smoothLine.value());
   m_embeddedViewer->RequestDraw();
}

void
FWRhoPhiZView::eventEnd()
{
   if (not m_caloAutoScale.value()) return;
   updateScaleParameters();

   m_overlayEventInfo->setEvent();
}

