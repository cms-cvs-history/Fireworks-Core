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
// $Id: FWListEventItem.cc,v 1.19 2008/07/08 20:09:42 chrjones Exp $
//

// system include files
#include <boost/bind.hpp>
#include <iostream>
#include <sstream>
#include <map>
#include "TEveManager.h"
#include "TEveSelection.h"

#include "TClass.h"
#include "Reflex/Type.h"
#include "Reflex/Member.h"
#include "Reflex/Object.h"
#include "Reflex/Base.h"

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
static
const std::vector<std::string>&
defaultMemberFunctionNames()
{
   static std::vector<std::string> s_names;
   if(s_names.empty()){
      s_names.push_back("pt");
      s_names.push_back("et");
      s_names.push_back("energy");
   }
   return s_names;
}

static
ROOT::Reflex::Member
recursiveFindMember(const std::string& iName, 
                    const ROOT::Reflex::Type& iType)
{
   using namespace ROOT::Reflex;

   Member temp = iType.MemberByName(iName);
   if(temp) {return temp;}
   
   //try all base classes
   for(Base_Iterator it = iType.Base_Begin(), itEnd = iType.Base_End();
       it != itEnd;
       ++it) {
      temp = recursiveFindMember(iName,it->ToType());
      if(temp) {break;}
   }
   return temp;
}


static
ROOT::Reflex::Member
findDefaultMember(const TClass* iClass) {
   using namespace ROOT::Reflex;
   if(0==iClass) {
      return Member();
   }
   
   Type rType = Type::ByTypeInfo(*(iClass->GetTypeInfo()));
   assert(rType != Type() );
   //std::cout <<"Type "<<rType.Name()<<std::endl;
   
   Member returnValue;
   const std::vector<std::string>& names = defaultMemberFunctionNames();
   for(std::vector<std::string>::const_iterator it = names.begin(), itEnd=names.end();
       it != itEnd;
       ++it) {
      //std::cout <<" trying function "<<*it<<std::endl;
      Member temp = recursiveFindMember(*it,rType);
      if(temp) {
         if(0==temp.FunctionParameterSize(true)) {
            //std::cout <<"    FOUND "<<temp.Name()<<std::endl;
            returnValue = temp;
            break;
         }
      }
   }
   return returnValue;
}

namespace {
   template <class T>
   std::string valueToString(const std::string& iName, const ROOT::Reflex::Object& iObj) {
      std::stringstream s;
      s.setf(std::ios_base::fixed,std::ios_base::floatfield);
      s.precision(2);
      T temp = *(reinterpret_cast<T*>(iObj.Address()));
      s<<iName <<" = "<<temp;
      return s.str();
   }

   typedef std::string(*FunctionType)(const std::string&,const ROOT::Reflex::Object&);
   typedef std::map<std::string, FunctionType> TypeToStringMap;
    
   template<typename T>
   static void addToStringMap(TypeToStringMap& iMap) {
      iMap[typeid(T).name()]=valueToString<T>;
   }

   template <class T>
   double valueToDouble(const ROOT::Reflex::Object& iObj) {
      return double(*(reinterpret_cast<T*>(iObj.Address())));
   }
   
   typedef double(*DoubleFunctionType)(const ROOT::Reflex::Object&);
   typedef std::map<std::string, DoubleFunctionType> TypeToDoubleMap;
   
   template<typename T>
   static void addToDoubleMap(TypeToDoubleMap& iMap) {
      iMap[typeid(T).name()]=valueToDouble<T>;
   }
   
}

static
std::string
stringValueFor(const ROOT::Reflex::Object& iObj, const ROOT::Reflex::Member& iMember) {
   static TypeToStringMap s_map;
   if(s_map.empty() ) {
      addToStringMap<float>(s_map);
      addToStringMap<double>(s_map);
   }
   
   ROOT::Reflex::Object val = iMember.Invoke(iObj);
   
   TypeToStringMap::iterator itFound =s_map.find(val.TypeOf().TypeInfo().name());
   if(itFound == s_map.end()) {
      //std::cout <<" could not print because type is "<<iObj.TypeOf().TypeInfo().name()<<std::endl;
      return std::string();
   }
   
   return itFound->second(iMember.Name(),val);
}

static
double
doubleValueFor(const ROOT::Reflex::Object& iObj, const ROOT::Reflex::Member& iMember) {
   static TypeToDoubleMap s_map;
   if(s_map.empty() ) {
      addToDoubleMap<float>(s_map);
      addToDoubleMap<double>(s_map);
   }

   ROOT::Reflex::Object val = iMember.Invoke(iObj);

   TypeToDoubleMap::iterator itFound =s_map.find(val.TypeOf().TypeInfo().name());
   if(itFound == s_map.end()) {
      //std::cout <<" could not print because type is "<<iObj.TypeOf().TypeInfo().name()<<std::endl;
      return -999.0;
   }
   
   return itFound->second(val);
 }

//
// constructors and destructor
//
FWListEventItem::FWListEventItem(FWEventItem* iItem,
                                 FWDetailViewManager* iDV):
TEveElementList(iItem->name().c_str(),"",kTRUE),
m_item(iItem),
m_detailViewManager(iDV),
m_memberFunction(findDefaultMember(iItem->modelType()))
{
   m_item->itemChanged_.connect(boost::bind(&FWListEventItem::itemChanged,this,_1));
   m_item->changed_.connect(boost::bind(&FWListEventItem::modelsChanged,this,_1));
   m_item->goingToBeDestroyed_.connect(boost::bind(&FWListEventItem::deleteListEventItem,this));
   m_item->defaultDisplayPropertiesChanged_.connect(boost::bind(&FWListEventItem::defaultDisplayPropertiesChanged,this,_1));
   TEveElementList::SetMainColor(iItem->defaultDisplayProperties().color());
   TEveElementList::SetRnrState(iItem->defaultDisplayProperties().isVisible());
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
void
  FWListEventItem::deleteListEventItem() {
  delete this;
}

bool 
FWListEventItem::doSelection(bool iToggleSelection)
{
   return true;
}

void 
FWListEventItem::SetMainColor(Color_t iColor)
{
   FWDisplayProperties prop(iColor,m_item->defaultDisplayProperties().isVisible());
   m_item->setDefaultDisplayProperties(prop);
   TEveElementList::SetMainColor(iColor);
}


Bool_t 
FWListEventItem::SetRnrState(Bool_t rnr)
{
   FWDisplayProperties prop(m_item->defaultDisplayProperties().color(),rnr);
   m_item->setDefaultDisplayProperties(prop);
   return TEveElementList::SetRnrState(rnr);
}
Bool_t 
FWListEventItem::SingleRnrState() const
{
   return kTRUE;
}

void 
FWListEventItem::defaultDisplayPropertiesChanged(const FWEventItem* iItem)
{
   TEveElementList::SetMainColor(iItem->defaultDisplayProperties().color());
   TEveElementList::SetRnrState(iItem->defaultDisplayProperties().isVisible());
}

void 
FWListEventItem::itemChanged(const FWEventItem* iItem)
{
   //std::cout <<"item changed "<<eventItem()->size()<<std::endl;
   this->DestroyElements();
   m_indexOrderedItems.clear();
   m_indexOrderedItems.reserve(eventItem()->size());
   typedef std::multimap<double, FWListModel*, std::greater<double> > OrderedMap;
   OrderedMap orderedMap;
   if(iItem->isCollection() ) {
      for(unsigned int index = 0; index < eventItem()->size(); ++index) {
         ROOT::Reflex::Object obj;
         std::string data;
         double doubleData=index;
         if(m_memberFunction) {
            //the const_cast is fine since I'm calling a const member function
            ROOT::Reflex::Object temp(m_memberFunction.DeclaringType(),
                                      const_cast<void*>(eventItem()->modelData(index)));
            obj=temp;
            data = stringValueFor(obj,m_memberFunction);
            doubleData = doubleValueFor(obj,m_memberFunction);
         }
         FWListModel* model = new FWListModel(FWModelId(eventItem(),index), 
                                              m_detailViewManager,
                                              data);
         m_indexOrderedItems.push_back(model);
         orderedMap.insert(std::make_pair(doubleData, model));
         model->SetMainColor(m_item->defaultDisplayProperties().color());
      }
      for(OrderedMap::iterator it = orderedMap.begin(), itEnd = orderedMap.end();
          it != itEnd;
          ++it) {
         this->AddElement( it->second );
      }
   }
}

void 
FWListEventItem::modelsChanged( const std::set<FWModelId>& iModels )
{
   //std::cout <<"modelsChanged "<<std::endl;
   if(m_item->isCollection()) {
      bool aChildChanged = false;
      for(FWModelIds::const_iterator it = iModels.begin(), itEnd = iModels.end();
          it != itEnd;
          ++it) {
         int index = it->index();
         assert(index < static_cast<int>(m_indexOrderedItems.size()));
         FWListModel* element = m_indexOrderedItems[index];
         //std::cout <<"   "<<index<<std::endl;
         bool modelChanged = false;
         const FWEventItem::ModelInfo& info = it->item()->modelInfo(index);
         FWListModel* model = element;
         modelChanged = model->update(info.displayProperties());
         if(info.isSelected() xor element->GetSelectedLevel()==1) {
            modelChanged = true;
            if(info.isSelected()) {         
               gEve->GetSelection()->AddElement(element);
            } else {
               gEve->GetSelection()->RemoveElement(element);
            }
         }      
         if(modelChanged) {
            element->ElementChanged();
            aChildChanged=true;
            //(*itElement)->UpdateItems();  //needed to force list tree to update immediately
         }
      }
   }
   // obsolete
   // if(aChildChanged) {
   //    this->UpdateItems();
   // }
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
