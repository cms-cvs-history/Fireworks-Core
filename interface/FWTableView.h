// -*- C++ -*-
#ifndef Fireworks_Core_FWTableView_h
#define Fireworks_Core_FWTableView_h
//
// Package:     Core
// Class  :     FWTableView
//
/**\class FWTableView FWTableView.h Fireworks/Core/interface/FWTableView.h

   Description: <one line class summary>

   Usage:
   <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Thu Feb 21 11:22:37 EST 2008
// $Id: FWTableView.h,v 1.4.2.2 2009/04/10 14:23:56 jmuelmen Exp $
//

// system include files
#include "Rtypes.h"

// user include files
#include "Fireworks/Core/interface/FWViewBase.h"
#include "Fireworks/Core/interface/FWTableViewTableManager.h"
#include "Fireworks/Core/interface/FWExpressionEvaluator.h"

// forward declarations
class TGFrame;
class TGLEmbeddedViewer;
class TGCompositeFrame;
class TGComboBox;
class TEvePad;
class TEveViewer;
class TEveScene;
class TEveElementList;
class TEveGeoShape;
class TGLMatrix;
class FWEventItem;
class FWTableViewManager;
class FWTableWidget;
class FWEveValueScaler;
class TEveWindowFrame;
class TEveWindowSlot;
class FWTableViewManager;

class FWTableView : public FWViewBase {
     friend class FWTableViewTableManager;

public:
     FWTableView(TEveWindowSlot *, const FWTableViewManager *);
     virtual ~FWTableView();

     // ---------- const member functions ---------------------
     TGFrame* frame() const;
     const std::string& typeName() const;
     virtual void addTo(FWConfiguration&) const;

     virtual void saveImageTo(const std::string& iName) const;

     // ---------- static member functions --------------------
     static const std::string& staticTypeName();

     // ---------- member functions ---------------------------
     virtual void setFrom(const FWConfiguration&);
     void setBackgroundColor(Color_t);
     void updateItems ();
     void updateEvaluators ();
     void selectCollection (Int_t);
     void display ();
     const FWEventItem *item () const;
     void modelSelected(Int_t iRow,Int_t iButton,Int_t iKeyMod);

private:
     FWTableView(const FWTableView&);    // stop default
     const FWTableView& operator=(const FWTableView&);    // stop default

protected:
     // ---------- member data --------------------------------
     TEveWindowFrame *m_frame;
     TGComboBox *m_collection;
     TGCompositeFrame *m_vert, *m_column_control;
     int m_iColl;
     const FWTableViewManager *m_manager;
     FWTableViewTableManager m_tableManager;
     FWTableWidget *m_tableWidget;
     std::vector<FWExpressionEvaluator> m_evaluators;
};


#endif
