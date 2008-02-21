// -*- C++ -*-
//
// Package:     Core
// Class  :     FWRPZ2DDataProxyBuilder
// 
// Implementation:
//     <Notes on implementation>
//
// Original Author:  
//         Created:  Thu Dec  6 17:49:54 PST 2007
// $Id: FWRPZ2DDataProxyBuilder.cc,v 1.2 2008/01/28 14:02:28 chrjones Exp $
//

// system include files
#include <iostream>
#include <boost/bind.hpp>
#include <algorithm>
#include "TEveElement.h"

// user include files
#include "Fireworks/Core/interface/FWRPZ2DDataProxyBuilder.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/FWModelId.h"


//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
FWRPZ2DDataProxyBuilder::FWRPZ2DDataProxyBuilder():
  m_item(0)
{
}

// FWRPZ2DDataProxyBuilder::FWRPZ2DDataProxyBuilder(const FWRPZ2DDataProxyBuilder& rhs)
// {
//    // do actual copying here;
// }

FWRPZ2DDataProxyBuilder::~FWRPZ2DDataProxyBuilder()
{
}

//
// assignment operators
//
// const FWRPZ2DDataProxyBuilder& FWRPZ2DDataProxyBuilder::operator=(const FWRPZ2DDataProxyBuilder& rhs)
// {
//   //An exception safe implementation is
//   FWRPZ2DDataProxyBuilder temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void
FWRPZ2DDataProxyBuilder::setItem(const FWEventItem* iItem)
{
  m_item = iItem;
   if(0 != m_item) {
      m_item->changed_.connect(boost::bind(&FWRPZ2DDataProxyBuilder::modelChangesRhoPhi,this,_1));
      m_item->changed_.connect(boost::bind(&FWRPZ2DDataProxyBuilder::modelChangesRhoZ,this,_1));
   }
}

void
FWRPZ2DDataProxyBuilder::buildRhoPhi(TEveElementList** iObject)
{
  if(0!= m_item) {
    buildRhoPhi(m_item, iObject);
  }
}

void
FWRPZ2DDataProxyBuilder::buildRhoZ(TEveElementList** iObject)
{
  if(0!= m_item) {
    buildRhoZ(m_item, iObject);
  }
}

void 
FWRPZ2DDataProxyBuilder::modelChangesRhoPhi(const FWModelIds& iIds)
{
   std::for_each(m_rhoPhiProjs.begin(),
                 m_rhoPhiProjs.end(),
                 boost::bind(&FWRPZ2DDataProxyBuilder::modelChangesRhoPhi,
                             this,
                             iIds,
                             _1));
}

void 
FWRPZ2DDataProxyBuilder::modelChangesRhoZ(const FWModelIds& iIds)
{
   std::for_each(m_rhoZProjs.begin(),
                 m_rhoZProjs.end(),
                 boost::bind(&FWRPZ2DDataProxyBuilder::modelChangesRhoZ,
                             this,
                             iIds,
                             _1));
}

static void
changeElementAndChildren(TEveElement* iElement, 
                         const FWEventItem::ModelInfo& iInfo)
{
   iElement->SetMainColor(iInfo.displayProperties().color());
   //for now, if selected make the item white
   if(iInfo.isSelected()) {
      iElement->SetMainColor(static_cast<Color_t>(kWhite));
   }
   
   for(TEveElement::List_i itElement = iElement->BeginChildren(),
       itEnd = iElement->EndChildren();
       itElement != itEnd;
       ++itElement) {
      changeElementAndChildren(*itElement, iInfo);
   }
}

static 
void 
modelChanges(const FWEventItem* iItem, 
             const FWModelIds& iIds,
             TEveElement* iElements )
{
   //std::cout <<"modelChanged "<<m_item->size()<<" "<<iElements->GetNChildren()<<std::endl;
   assert(iItem && iItem->size() == iElements->GetNChildren() && "can not use default modelChanges implementation");
   TEveElement::List_i itElement = iElements->BeginChildren();
   int index = 0;
   for(FWModelIds::const_iterator it = iIds.begin(), itEnd = iIds.end();
       it != itEnd;
       ++it,++itElement,++index) {
      assert(itElement != iElements->EndChildren());         
      while(index < it->index()) {
         ++itElement;
         ++index;
         assert(itElement != iElements->EndChildren());         
      }
      const FWEventItem::ModelInfo& info = it->item()->modelInfo(index);
      changeElementAndChildren(*itElement, info);
      (*itElement)->SetRnrSelf(info.displayProperties().isVisible());
      (*itElement)->SetRnrChildren(info.displayProperties().isVisible());
      (*itElement)->ElementChanged();
   }
}

void 
FWRPZ2DDataProxyBuilder::modelChangesRhoPhi(const FWModelIds& iIds, TEveElement* iElements)
{
   modelChanges(m_item,iIds,iElements);
}
void 
FWRPZ2DDataProxyBuilder::modelChangesRhoZ(const FWModelIds& iIds, TEveElement* iElements)
{
   modelChanges(m_item,iIds,iElements);
}

void 
FWRPZ2DDataProxyBuilder::addRhoPhiProj(TEveElement* iElement)
{
   
   //std::cout <<"setRhoPhiProj "<<m_item->name()<<" "<<iElement->GetRnrElName()<<" "<<iElement->GetNChildren()<<" "<<m_item->size()<<std::endl;
   assert(0!=iElement);
   m_rhoPhiProjs.push_back(iElement);
   //assert(0==iElement || iElement->GetNChildren() == m_item->size());
}
void 
FWRPZ2DDataProxyBuilder::addRhoZProj(TEveElement* iElement)
{
   assert(0!=iElement);
   m_rhoZProjs.push_back(iElement);
   //assert(0==iElement || iElement->GetNChildren() == m_item->size());
}

void 
FWRPZ2DDataProxyBuilder::clearRhoPhiProjs()
{
   m_rhoPhiProjs.clear();
}
void 
FWRPZ2DDataProxyBuilder::clearRhoZProjs()
{
   m_rhoZProjs.clear();
}

//
// const member functions
//

//
// static member functions
//
