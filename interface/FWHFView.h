#ifndef Fireworks_Core_FWHFView_h
#define Fireworks_Core_FWHFView_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWHFView
// 
/**\class FWHFView FWHFView.h Fireworks/Core/interface/FWHFView.h

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Yanjun
//         Created:  Mon May 31 13:42:21 CEST 2010
// $Id: FWHFView.h,v 1.1 2010/05/31 13:01:24 amraktad Exp $
//

// system include files

// user include files
#include "Fireworks/Core/interface/FWLegoViewBase.h"

// forward declarations

class FWHFView : public FWLegoViewBase
{
public:
   FWHFView(TEveWindowSlot*, FWViewType::EType);
   virtual ~FWHFView();

   virtual void setContext(fireworks::Context&);
   // ---------- const member functions ---------------------

   virtual TEveCaloData* getCaloData(fireworks::Context&) const;
   // ---------- static member functions --------------------

   // ---------- member functions ---------------------------

private:
   FWHFView(const FWHFView&); // stop default

   const FWHFView& operator=(const FWHFView&); // stop default

   // ---------- member data --------------------------------

};


#endif
