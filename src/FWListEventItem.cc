// -*- C++ -*-
//
// Package:     Core
// Class  :     FWListEventItem
// 
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Thu Feb 28 11:13:37 PST 2008
// $Id: FWListEventItem.cc,v 1.8 2008/03/11 23:30:04 chrjones Exp $
//

// system include files
#include <boost/bind.hpp>
#include <iostream>
#include "TEveManager.h"
#include "TEveSelection.h"

// user include files
#include "Fireworks/Core/src/FWListEventItem.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/src/FWListModel.h"

#include "Fireworks/Core/interface/FWModelId.h"
#include "Fireworks/Core/interface/FWDetailViewManager.h"
#include "Fireworks/Core/interface/FWModelChangeManager.h"


//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
FWListEventItem::FWListEventItem(FWEventItem* iItem,
                                 FWDetailViewManager* iDV):
TEveElementList(iItem->name().c_str(),"",kTRUE),
m_item(iItem),
m_detailViewManager(iDV)
{
   m_item->itemChanged_.connect(boost::bind(&FWListEventItem::itemChanged,this,_1));
   m_item->changed_.connect(boost::bind(&FWListEventItem::modelsChanged,this,_1));
   TEveElementList::SetMainColor(iItem->defaultDisplayProperties().color());
}

// FWListEventItem::FWListEventItem(const FWListEventItem& rhs)
// {
//    // do actual copying here;
// }

FWListEventItem::~FWListEventItem()
{
}

//
// assignment operators
//
// const FWListEventItem& FWListEventItem::operator=(const FWListEventItem& rhs)
// {
//   //An exception safe implementation is
//   FWListEventItem temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
bool 
FWListEventItem::doSelection(bool iToggleSelection)
{
   return true;
}

void 
FWListEventItem::SetMainColor(Color_t iColor)
{
   FWChangeSentry sentry(*(m_item->changeManager()));
   FWDisplayProperties prop(iColor,m_item->defaultDisplayProperties().isVisible());
   m_item->setDefaultDisplayProperties(prop);
   TEveElementList::SetMainColor(iColor);
   
   for(int index=0; index <static_cast<int>(m_item->size()); ++index) {
      FWDisplayProperties prop=m_item->modelInfo(index).displayProperties();
      if(iColor !=prop.color()) {
          prop.setColor(iColor);
         m_item->setDisplayProperties(index,prop);
      }
   }
}


void 
FWListEventItem::SetRnrState(Bool_t rnr)
{
   FWChangeSentry sentry(*(m_item->changeManager()));
   FWDisplayProperties prop(m_item->defaultDisplayProperties().color(),rnr);
   m_item->setDefaultDisplayProperties(prop);
   TEveElementList::SetRnrState(rnr);

   for(int index=0; index <static_cast<int>(m_item->size()); ++index) {
      FWDisplayProperties prop=m_item->modelInfo(index).displayProperties();
      if(rnr !=prop.isVisible()) {
          prop.setIsVisible(rnr);
         m_item->setDisplayProperties(index,prop);
      }
   }
   
}
Bool_t 
FWListEventItem::SingleRnrState() const
{
   return kTRUE;
}


void 
FWListEventItem::itemChanged(const FWEventItem* iItem)
{
   //std::cout <<"item changed "<<eventItem()->size()<<std::endl;
   this->DestroyElements();
   for(unsigned int index = 0; index < eventItem()->size(); ++index) {
      FWListModel* model = new FWListModel(FWModelId(eventItem(),index));
      this->AddElement( model );
      model->SetMainColor(m_item->defaultDisplayProperties().color());
   }
}

void 
FWListEventItem::modelsChanged( const std::set<FWModelId>& iModels )
{
   //std::cout <<"modelsChanged "<<std::endl;
   bool aChildChanged = false;
   TEveElement::List_i itElement = this->BeginChildren();
   int index = 0;
   for(FWModelIds::const_iterator it = iModels.begin(), itEnd = iModels.end();
       it != itEnd;
       ++it,++itElement,++index) {
      assert(itElement != this->EndChildren());         
      while(index < it->index()) {
         ++itElement;
         ++index;
         assert(itElement != this->EndChildren());         
      }
      //std::cout <<"   "<<index<<std::endl;
      bool modelChanged = false;
      const FWEventItem::ModelInfo& info = it->item()->modelInfo(index);
      FWListModel* model = static_cast<FWListModel*>(*itElement);
      modelChanged = model->update(info.displayProperties());
      if(info.isSelected() xor (*itElement)->GetSelectedLevel()==1) {
         modelChanged = true;
         if(info.isSelected()) {         
            gEve->GetSelection()->AddElement(*itElement);
         } else {
            gEve->GetSelection()->RemoveElement(*itElement);
         }
      }      
      if(modelChanged) {
         (*itElement)->ElementChanged();
         aChildChanged=true;
         //(*itElement)->UpdateItems();  //needed to force list tree to update immediately
      }
   }
   if(aChildChanged) {
      this->UpdateItems();
   }
   //std::cout <<"modelsChanged done"<<std::endl;

}

//
// const member functions
//
FWEventItem* 
FWListEventItem::eventItem() const
{
   return m_item;
}

void 
FWListEventItem::openDetailViewFor(int index) const
{
   m_detailViewManager->openDetailViewFor( FWModelId(m_item,index));
}

//
// static member functions
//

ClassImp(FWListEventItem)
