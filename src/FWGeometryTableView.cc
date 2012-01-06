// -*- C++ -*-
//
// Package:     Core
// Class  :     FWGeometryTableView
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  
//         Created:  Wed Jan  4 00:05:34 CET 2012
// $Id: FWGeometryTableView.cc,v 1.22.2.9 2012/01/04 02:39:46 amraktad Exp $
//

// system include files
#include <boost/bind.hpp>

// user include files
#include "Fireworks/Core/src/FWGeometryTableView.h"
#include "Fireworks/Core/interface/FWViewType.h"
#include "Fireworks/Core/interface/FWGeometryTableManagerBase.h"
#include "Fireworks/Core/interface/CmsShowViewPopup.h"
#include "Fireworks/Core/src/FWGeometryTableManager.h"
#include "Fireworks/Core/interface/fwLog.h"

#include "Fireworks/Core/src/FWGUIValidatingTextEntry.h"
#include "Fireworks/Core/src/FWValidatorBase.h"
#include "Fireworks/Core/interface/FWGeoTopNode.h"

#include "TGButton.h"
#include "TGLabel.h"
#include "KeySymbols.h"
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

class FWGeoMaterialValidator : public FWValidatorBase 
{
public:
   struct Material
   {
      TGeoMaterial* g;
      std::string n;
      bool operator< (const Material& x) const { return n < x.n ;}
      Material( TGeoMaterial* x) {  g= x; n = x ? x->GetName() : "<show-all>";}
   };

   FWGeometryTableManagerBase* m_table;
   mutable std::vector<Material> m_list;

   FWGeoMaterialValidator( FWGeometryTableManagerBase* t) { m_table = t;}
   virtual ~FWGeoMaterialValidator() {}


   virtual void fillOptions(const char* iBegin, const char* iEnd, std::vector<std::pair<boost::shared_ptr<std::string>, std::string> >& oOptions) const 
   {
      oOptions.clear();
      std::string part(iBegin,iEnd);
      unsigned int part_size = part.size();

      m_list.clear();
      m_list.push_back(0);

      FWGeometryTableManagerBase::Entries_i it = m_table->refSelected();
      int nLevel = it->m_level;
      it++;
      while (it->m_level > nLevel)
      {
         TGeoMaterial* g = it->m_node->GetVolume()->GetMaterial();
         bool duplicate = false;
         for (std::vector<Material>::iterator j = m_list.begin(); j!=m_list.end(); ++j) {
            if (j->g == g) {
               duplicate = true;
               break;
            }
         }
         if (!duplicate)
            m_list.push_back(g);

         ++it;
      }
      std::vector<Material>::iterator startIt = m_list.begin();
      startIt++;
      std::sort(startIt, m_list.end());

      std::string h = "";
      oOptions.push_back(std::make_pair(boost::shared_ptr<std::string>(new std::string(m_list.begin()->n)), h));
      for (std::vector<Material>::iterator i = startIt; i!=m_list.end(); ++i) {
         if(part == (*i).n.substr(0,part_size) )
         {
            //  std::cout << i->n <<std::endl;
            oOptions.push_back(std::make_pair(boost::shared_ptr<std::string>(new std::string((*i).n)), (*i).n.substr(part_size, (*i).n.size()-part_size)));
         }
      }

   }

   bool isStringValid(std::string& exp) 
   {
      if (exp.empty()) return true;

      for (std::vector<Material>::iterator i = m_list.begin(); i!=m_list.end(); ++i) {
         if (exp == (*i).n) 
            return true;
    
      }
      return false;
   }
};


//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

FWGeometryTableView::FWGeometryTableView(TEveWindowSlot* iParent, FWColorManager* colMng, TGeoNode* tn, TObjArray* volumes)
   : FWGeometryTableViewBase(iParent, FWViewType::kGeometryTable, colMng, tn, volumes),
     m_tableManager(0),
     m_filterEntry(0),
     m_filterValidator(0),
     m_eveTopNode(0),
     m_mode(this, "Mode:", 0l, 0l, 1l),
     m_filter(this,"Materials:",std::string()),
     m_disableTopNode(this,"HideTopNode", true),
     m_autoExpand(this,"ExpandList:", 1l, 0l, 100l),
     m_visLevel(this,"VisLevel:", 3l, 1l, 100l),
     m_visLevelFilter(this,"IgnoreVisLevelOnFilter", true)
{


   // top row
   TGHorizontalFrame* hp =  new TGHorizontalFrame(m_frame);

   {
      TGTextButton* rb = new TGTextButton (hp, "CdTop");
      hp->AddFrame(rb, new TGLayoutHints(kLHintsNormal, 2, 2, 0, 0) );
      //rb->Connect("Clicked()","FWGeometryTableViewBase",this,"cdTop()");
   } 
   {
      TGTextButton* rb = new TGTextButton (hp, "CdUp");
      hp->AddFrame(rb, new TGLayoutHints(kLHintsNormal, 2, 2, 0, 0));
      // rb->Connect("Clicked()","FWGeometryTableViewBase",this,"cdUp()");
   }
  
   {
      m_viewBox = new FWViewCombo(hp, this);
      hp->AddFrame( m_viewBox,new TGLayoutHints(kLHintsExpandY, 2, 2, 0, 0));
   }
   {
      hp->AddFrame(new TGLabel(hp, "Filter:"), new TGLayoutHints(kLHintsBottom, 10, 0, 0, 2));
      m_filterEntry = new FWGUIValidatingTextEntry(hp);
      m_filterEntry->SetHeight(20);
      m_filterValidator = new FWGeoMaterialValidator(getTableManager());
      m_filterEntry->setValidator(m_filterValidator);
      hp->AddFrame(m_filterEntry, new TGLayoutHints(kLHintsExpandX,  1, 2, 1, 0));
      m_filterEntry->setMaxListBoxHeight(150);
      //   m_filterEntry->getListBox()->Connect("Selected(int)", "FWGeometryTableViewBase",  this, "filterListCallback()");
      //  m_filterEntry->Connect("ReturnPressed()", "FWGeometryTableViewBase",  this, "filterTextEntryCallback()");

      gVirtualX->GrabKey( m_filterEntry->GetId(),gVirtualX->KeysymToKeycode((int)kKey_A),  kKeyControlMask, true);
   }
   m_frame->AddFrame(hp,new TGLayoutHints(kLHintsLeft|kLHintsExpandX, 4, 2, 2, 0));

   m_tableManager = new FWGeometryTableManager(this);
   initGeometry(tn, volumes);



   m_mode.addEntry(kNode, "Node");
   m_mode.addEntry(kVolume, "Volume");
   
   m_mode.changed_.connect(boost::bind(&FWGeometryTableView::modeChanged,this));
   m_autoExpand.changed_.connect(boost::bind(&FWGeometryTableView::autoExpandChanged, this));
   m_visLevel.changed_.connect(boost::bind(&FWGeometryTableView::refreshTable3D,this));
   m_visLevelFilter.changed_.connect(boost::bind(&FWGeometryTableView::refreshTable3D,this));


   m_disableTopNode.changed_.connect(boost::bind(&FWGeometryTableViewBase::refreshTable3D,this));
   postConst();
}


FWGeometryTableView::~FWGeometryTableView()
{
}



void FWGeometryTableView::initGeometry(TGeoNode* iGeoTopNode, TObjArray* iVolumes)
{ 
   m_tableManager->loadGeometry(iGeoTopNode, iVolumes);
   cdTop();
}


FWGeometryTableManagerBase* FWGeometryTableView::getTableManager()
{
   return m_tableManager;
}


void FWGeometryTableView::modeChanged()
{
   // reset filter when change mode
   std::cout << "chage mode \n";

   m_tableManager->updateFilter();
   refreshTable3D();
}



void FWGeometryTableView::autoExpandChanged()
{
   m_tableManager->checkExpandLevel();
   m_tableManager->redrawTable();
}




   //______________________________________________________________________________
void FWGeometryTableView::filterTextEntryCallback()
{
   // std::cout << "text entry click ed \n" ;
   std::string exp = m_filterEntry->GetText();
   if ( m_filterValidator->isStringValid(exp)) 
   {
      updateFilter(exp);
   }
   else
   {
      fwLog(fwlog::kError) << "filter expression not valid." << std::endl;
      return;
   }
}

void FWGeometryTableView::filterListCallback()
{ 
   //   std::cout << "list click ed \n" ;

   std::string exp = m_filterEntry->GetText();
   updateFilter(exp);
}



void FWGeometryTableView::updateFilter(std::string& exp)
{
   // std::cout << "=FWGeometryTableViewBase::updateFilter()" << m_filterEntry->GetText() <<std::endl;
  
   if (exp == m_filterValidator->m_list.begin()->n) 
      exp.clear();

   if (exp == m_filter.value()) return;

   if (exp.empty())
   {
      // std::cout << "FITLER OFF \n";
      for (FWGeometryTableManagerBase::Entries_i i = m_tableManager->refEntries().begin(); i !=  m_tableManager->refEntries().end(); ++i)
      {
         m_tableManager->setVisibility(*i, true);
         m_tableManager->setVisibilityChld(*i, true);
      }

      // NOTE: entry should be cleared automatically
      m_filterEntry->Clear();

      m_tableManager->checkExpandLevel();
   }
  
   m_filter.set(exp);
   m_tableManager->updateFilter();
   refreshTable3D();

}


//==============================================================================

void FWGeometryTableView::populateController(ViewerParameterGUI& gui) const
{
   gui.requestTab("Style").
      addParam(&m_disableTopNode).
      addParam(&m_mode).
      addParam(&m_autoExpand).
      addParam(&m_visLevel).
      addParam(&m_visLevelFilter).
      separator().
      addParam(&m_enableHighlight);

}

//______________________________________________________________________________
void  FWGeometryTableView::assertEveGeoElement()
{
 if (!m_eveTopNode )
   {
      printf("NEWFWEveDetectorGeo -------------------------------\n ");
      m_eveTopNode = new  FWEveDetectorGeo(this);
      m_eveTopNode->IncDenyDestroy();
   }
}

TEveElement* FWGeometryTableView::getEveGeoElement() const
{
  
   return m_eveTopNode;
}
