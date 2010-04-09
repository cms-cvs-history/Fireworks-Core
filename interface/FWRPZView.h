#ifndef Fireworks_Core_FWRPZView_h
#define Fireworks_Core_FWRPZView_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWRPZView
//
/**\class FWRPZView FWRPZView.h Fireworks/Core/interface/FWRPZView.h

   Description: <one line class summary>

   Usage:
    <usage>

 */
//
// Original Author:  Chris Jones
//         Created:  Tue Feb 19 10:33:21 EST 2008
// $Id: FWRPZView.h,v 1.1 2010/04/09 14:52:09 amraktad Exp $
//

// system include files
#include <string>

// user include files
#include "Fireworks/Core/interface/FWEveView.h"
#include "Fireworks/Core/interface/FWDoubleParameter.h"
#include "Fireworks/Core/interface/FWBoolParameter.h"
#include "Fireworks/Core/interface/FWEvePtr.h"

// forward declarations
class TEveProjectionManager;
class TGLMatrix;
class TEveCalo2D;
class TEveProjectionAxes;
class TEveWindowSlot;
class FWColorManager;

class FWRPZView : public FWEveView
{
public:
   FWRPZView(TEveWindowSlot* iParent, FWViewType::EType);
   virtual ~FWRPZView();

   // ---------- const member functions ---------------------

   virtual void addTo(FWConfiguration&) const;

   // ---------- member functions ---------------------------
   virtual void setGeometry(fireworks::Context&);
   virtual void setFrom(const FWConfiguration&);

   //returns the new element created from this import
   
   void eventEnd();
   void importElements(TEveElement* iProjectableChild, float iLayer, TEveElement* iProjectedParent=0);

private:
   FWRPZView(const FWRPZView&);    // stop default
   const FWRPZView& operator=(const FWRPZView&);    // stop default 

   void doDistortion();
   void doCompression(bool);
   // void doZoom(double);

   void updateCaloParameters();
   void updateScaleParameters();
   void updateCalo(TEveElement*, bool dataChanged = false);
   void updateCaloLines(TEveElement*);

   void showProjectionAxes( );
   // ---------- member data --------------------------------
   FWEvePtr<TEveProjectionManager> m_projMgr;

   double m_caloScale;
   FWEvePtr<TEveProjectionAxes> m_axes;

   // parameters
   FWLongParameter    m_overlayEventInfoLevel;
   FWBoolParameter    m_drawCMSLogo;
   FWDoubleParameter  m_caloDistortion;
   FWDoubleParameter  m_muonDistortion;
   FWBoolParameter    m_showProjectionAxes;
   FWBoolParameter    m_compressMuon;
   FWDoubleParameter  m_caloFixedScale;
   FWBoolParameter    m_caloAutoScale;
   FWBoolParameter*   m_showHF;
   FWBoolParameter*   m_showEndcaps;

};


#endif
