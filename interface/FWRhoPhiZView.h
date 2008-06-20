#ifndef Fireworks_Core_FWRhoPhiZView_h
#define Fireworks_Core_FWRhoPhiZView_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWRhoPhiZView
// 
/**\class FWRhoPhiZView FWRhoPhiZView.h Fireworks/Core/interface/FWRhoPhiZView.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Tue Feb 19 10:33:21 EST 2008
// $Id: FWRhoPhiZView.h,v 1.7 2008/06/09 18:50:04 chrjones Exp $
//

// system include files
#include <string>
#include "TGLViewer.h"
#include "TEveProjections.h"

// user include files
#include "Fireworks/Core/interface/FWViewBase.h"
#include "Fireworks/Core/interface/FWDoubleParameter.h"
#include "Fireworks/Core/interface/FWBoolParameter.h"

// forward declarations
class TEvePad;
class TEveViewer;
class TGLEmbeddedViewer;
class TEveProjectionManager;
class TGFrame;
class TGLMatrix;

class FWRhoPhiZView : public FWViewBase
{

   public:
      FWRhoPhiZView(TGFrame* iParent,
                    const std::string& iTypeName,
                    const TEveProjection::EPType_e& iProjType);
      virtual ~FWRhoPhiZView();

      // ---------- const member functions ---------------------
      TGFrame* frame() const;
      const std::string& typeName() const;
      virtual void addTo(FWConfiguration&) const;

      virtual void saveImageTo(const std::string& iName) const;

      // ---------- static member functions --------------------

      // ---------- member functions ---------------------------
      void resetCamera();
      void destroyElements();
      void replicateGeomElement(TEveElement*);
      virtual void setFrom(const FWConfiguration&);

      //returns the new element created from this import
      TEveElement* importElements(TEveElement*, float iLayer);
   private:
      void doDistortion(double);
      void doCompression(bool);
      void doZoom(double);
      FWRhoPhiZView(const FWRhoPhiZView&); // stop default

      const FWRhoPhiZView& operator=(const FWRhoPhiZView&); // stop default

      // ---------- member data --------------------------------
      TEvePad* m_pad;
      TEveViewer* m_viewer;
      TGLEmbeddedViewer* m_embeddedViewer;
      TEveProjectionManager* m_projMgr;
      std::vector<TEveElement*> m_geom;
      std::string m_typeName;
      TEveScene* m_scene;

      FWDoubleParameter m_distortion;
      FWBoolParameter m_compressMuon;
      // camera parameters
      double* m_cameraZoom;
      TGLMatrix* m_cameraMatrix;
};


#endif
