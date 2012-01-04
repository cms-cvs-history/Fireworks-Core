#ifndef Fireworks_Core_FWGeometryTableView_h
#define Fireworks_Core_FWGeometryTableView_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWGeometryTableView
// 
/**\class FWGeometryTableView FWGeometryTableView.h Fireworks/Core/interface/FWGeometryTableView.h

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  
//         Created:  Wed Jan  4 00:05:38 CET 2012
// $Id$
//

// system include files

// user include files

// forward declarations

#include "Fireworks/Core/interface/FWGeometryTableViewBase.h"

class FWGeometryTableView : public FWGeometryTableViewBase
{

public:
   FWGeometryTableView(TEveWindowSlot* iParent, FWColorManager* colMng, TGeoNode* tn, TObjArray* volumes);
   virtual ~FWGeometryTableView();
   virtual void populateController(ViewerParameterGUI&) const;

protected:

   virtual void initGeometry(TGeoNode* iGeoTopNode, TObjArray* iVolumes);

private:
   FWGeometryTableView(const FWGeometryTableView&); // stop default

   const FWGeometryTableView& operator=(const FWGeometryTableView&); // stop default

   // ---------- member data --------------------------------



   void autoExpandChanged();
   void modeChanged();
};


#endif
