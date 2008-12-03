// -*- C++ -*-
//
// Package:     Core
// Class  :     FW3DLegoDataProxyBuilder
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:
//         Created:  Thu Dec  6 17:49:54 PST 2007
// $Id: FW3DLegoDataProxyBuilder.cc,v 1.11 2008/11/06 22:05:24 amraktad Exp $
//

// system include files
#include <iostream>
#include <boost/bind.hpp>
#include "TEveElement.h"
#include "TEveManager.h"
#include "TEveSelection.h"

// user include files
#include "Fireworks/Core/interface/FW3DLegoDataProxyBuilder.h"
#include "TH2F.h"
#include "TEveElement.h"
#include <iostream>
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/FWModelId.h"

//
// constants, enums and typedefs
//
namespace fw3dlego
{
  const double xbins[83] = {
           -5.191, -4.889,
	   -4.716, -4.538, -4.363, -4.191, -4.013, -3.839, -3.664, -3.489, -3.314,
	   -3.139, -2.964, -2.853, -2.650, -2.500, -2.322, -2.172, -2.043, -1.930, -1.830,
	   -1.740, -1.653, -1.566, -1.479, -1.392, -1.305, -1.218, -1.131, -1.044, -0.957,
	   -0.870, -0.783, -0.696, -0.609, -0.522, -0.435, -0.348, -0.261, -0.174, -0.087,
	   0.000,
	    0.087,  0.174,  0.261,  0.348,  0.435,  0.522,  0.609,  0.696,  0.783,  0.870,
	    0.957,  1.044,  1.131,  1.218,  1.305,  1.392,  1.479,  1.566,  1.653,  1.740,
	    1.830,  1.930,  2.043,  2.172,  2.322,  2.500,  2.650,  2.853,  2.964,  3.139,
	    3.314,  3.489,  3.664,  3.839,  4.013,  4.191,  4.363,  4.538,  4.716,
            4.889,  5.191
  };

}

//
// static data member definitions
//

//
// constructors and destructor
//
FW3DLegoDataProxyBuilder::FW3DLegoDataProxyBuilder():
  m_item(0), m_modelsChanged(false), m_haveViews(false), m_mustBuild(true)
{
}

// FW3DLegoDataProxyBuilder::FW3DLegoDataProxyBuilder(const FW3DLegoDataProxyBuilder& rhs)
// {
//    // do actual copying here;
// }

FW3DLegoDataProxyBuilder::~FW3DLegoDataProxyBuilder()
{
}

//
// assignment operators
//
// const FW3DLegoDataProxyBuilder& FW3DLegoDataProxyBuilder::operator=(const FW3DLegoDataProxyBuilder& rhs)
// {
//   //An exception safe implementation is
//   FW3DLegoDataProxyBuilder temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void
FW3DLegoDataProxyBuilder::setItem(const FWEventItem* iItem)
{
   m_item = iItem;
   if(0 != m_item) {
      m_item->changed_.connect(boost::bind(&FW3DLegoDataProxyBuilder::modelChanges,this,_1));
      m_item->itemChanged_.connect(boost::bind(&FW3DLegoDataProxyBuilder::itemChanged,this,_1));
      m_item->goingToBeDestroyed_.connect(boost::bind(&FW3DLegoDataProxyBuilder::itemBeingDestroyed,this,_1));
   }

}

void
FW3DLegoDataProxyBuilder::itemChanged(const FWEventItem* iItem)
{
   //std::cout <<"item changed "<<iItem->name()<<std::endl;
   if(m_haveViews) {
      //std::cout <<"  building..."<<std::endl;
      build();
      m_mustBuild=false;
   } else {
      m_mustBuild=true;
   }
   m_modelsChanged=false;
}

void
FW3DLegoDataProxyBuilder::itemBeingDestroyed(const FWEventItem* iItem)
{
   m_item=0;
   m_ids.clear();
   itemBeingDestroyedImp(iItem);
}

void
FW3DLegoDataProxyBuilder::itemBeingDestroyedImp(const FWEventItem*)
{
}


void
FW3DLegoDataProxyBuilder::setHaveAWindow(bool iFlag)
{
   bool oldValue = m_haveViews;

   m_haveViews=iFlag;

   if(iFlag && !oldValue) {
      //this is our first view so may need to rerun our building
      if(m_mustBuild) {
         build();
         m_mustBuild=false;
      }
      if(m_modelsChanged) {
         //Need to update all the models
         applyChangesToAllModels();
         m_modelsChanged=false;
      }
   }
}

void
FW3DLegoDataProxyBuilder::modelChanges(const FWModelIds& iIds)
{
   if(m_haveViews) {
      // printf("number of model ids: %d\n", iIds.size() );
      modelChangesImp(iIds);
      m_modelsChanged=false;
   } else {
      m_modelsChanged=true;
   }
}

std::string 
FW3DLegoDataProxyBuilder::typeOfBuilder()
{
   return std::string();
}

