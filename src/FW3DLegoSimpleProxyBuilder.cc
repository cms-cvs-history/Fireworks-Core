// -*- C++ -*-
//
// Package:     Core
// Class  :     FW3DLegoSimpleProxyBuilder
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Tue Dec  2 16:34:50 EST 2008
// $Id: FW3DLegoSimpleProxyBuilder.cc,v 1.2 2009/01/23 21:35:42 amraktad Exp $
//

// system include files
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/mem_fn.hpp>

#include "TEveCompound.h"

// user include files
#include "Fireworks/Core/interface/FW3DLegoSimpleProxyBuilder.h"
#include "Fireworks/Core/src/changeElementAndChildren.h"


//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
FW3DLegoSimpleProxyBuilder::FW3DLegoSimpleProxyBuilder(const std::type_info& iType) :
   m_helper(iType)
{
}

// FW3DLegoSimpleProxyBuilder::FW3DLegoSimpleProxyBuilder(const FW3DLegoSimpleProxyBuilder& rhs)
// {
//    // do actual copying here;
// }

//FW3DLegoSimpleProxyBuilder::~FW3DLegoSimpleProxyBuilder()
//{
//}

//
// assignment operators
//
// const FW3DLegoSimpleProxyBuilder& FW3DLegoSimpleProxyBuilder::operator=(const FW3DLegoSimpleProxyBuilder& rhs)
// {
//   //An exception safe implementation is
//   FW3DLegoSimpleProxyBuilder temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void
FW3DLegoSimpleProxyBuilder::itemChangedImp(const FWEventItem* iItem)
{
   if(0!=iItem) {
      m_helper.itemChanged(iItem);
   }
}

void
FW3DLegoSimpleProxyBuilder::build(const FWEventItem* iItem,
                                  TEveElementList** product)
{
   if(0==*product) {
      *product = new TEveElementList();
   } else {
      (*product)->DestroyElements();
   }
   size_t size = iItem->size();
   for(int index = 0; index < static_cast<int>(size); ++index) {
      const void* modelData = iItem->modelData(index);
      std::string name;
      m_helper.fillTitle(*iItem,index,name);
      std::auto_ptr<TEveCompound> itemHolder(new TEveCompound(name.c_str(),name.c_str()));
      const FWEventItem::ModelInfo& info = iItem->modelInfo(index);
      if(info.displayProperties().isVisible()) {
         
         {
            itemHolder->OpenCompound();
            //guarantees that CloseCompound will be called no matter what happens
            boost::shared_ptr<TEveCompound> sentry(itemHolder.get(),
                                                   boost::mem_fn(&TEveCompound::CloseCompound));
            build(m_helper.offsetObject(modelData),index,*itemHolder);
         }
         changeElementAndChildren(itemHolder.get(), info);
      }
      itemHolder->SetRnrSelf(info.displayProperties().isVisible());
      itemHolder->SetRnrChildren(info.displayProperties().isVisible());
      itemHolder->ElementChanged();
      (*product)->AddElement(itemHolder.release());
   }
}

bool 
FW3DLegoSimpleProxyBuilder::specialModelChangeHandling(const FWModelId& iId, TEveElement* iCompound)
{
   const FWEventItem::ModelInfo& info = iId.item()->modelInfo(iId.index());
   bool returnValue = false;
   if(info.displayProperties().isVisible() && iCompound->NumChildren()==0) {
      TEveCompound* c = dynamic_cast<TEveCompound*> (iCompound);
      assert(0!=c);
      c->OpenCompound();
      //guarantees that CloseCompound will be called no matter what happens
      boost::shared_ptr<TEveCompound> sentry(c,
                                             boost::mem_fn(&TEveCompound::CloseCompound));
      const void* modelData = iId.item()->modelData(iId.index());
      
      build(m_helper.offsetObject(modelData),iId.index(),*iCompound);
      returnValue=true;
   }
   return returnValue;   
}

//
// const member functions
//

//
// static member functions
//
std::string
FW3DLegoSimpleProxyBuilder::typeOfBuilder()
{
   return std::string("simple#");
}
