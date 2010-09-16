// -*- C++ -*-
//
// Package:     Core
// Class  :     FWViewGeometryList
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Alja Mrak-Tadel 
//         Created:  Tue Sep 14 13:28:13 CEST 2010
// $Id: FWViewGeometryList.cc,v 1.1 2010/09/15 11:48:42 amraktad Exp $
//

#include <boost/bind.hpp>
#include "TEveScene.h"
#include "TEveManager.h"
#include "TEveCompound.h"

#include "Fireworks/Core/interface/FWViewGeometryList.h"
#include "Fireworks/Core/interface/FWGeometry.h"
#include "Fireworks/Core/interface/Context.h"

FWViewGeometryList::FWViewGeometryList(const fireworks::Context& context):
   m_context(context),
   m_geom(0)
{ 
   m_geom = context.getGeom();

   for (int i = 0; i < kFWGeomColorSize; ++i)
   {
      m_colorComp[i] = new TEveCompound(Form("3D view color compund [%d]", i));
      m_colorComp[i]->SetMainColor(m_context.colorManager()->geomColor(FWGeomColorIndex(i)));
      m_colorComp[i]->SetMainTransparency(m_context.colorManager()->geomTransparency(projected()));
      m_colorComp[i]->CSCApplyMainColorToAllChildren();
      m_colorComp[i]->CSCApplyMainTransparencyToMatchingChildren();
      gEve->GetGlobalScene()->AddElement(m_colorComp[i]);
   }

   context.colorManager()->geomColorsHaveChanged_.connect(boost::bind(&FWViewGeometryList::updateColors, this));
   context.colorManager()->geomTransparencyHaveChanged_.connect(boost::bind(&FWViewGeometryList::updateTransparency,this,  _1));
}

FWViewGeometryList::~FWViewGeometryList()
{
}

void
FWViewGeometryList::addToCompound(TEveElement* el, FWGeomColorIndex colIdx ,  bool applyTransp) const
{
   el->SetMainColor( m_colorComp[colIdx]->GetMainColor()); 
   if (applyTransp) 
      el->SetMainTransparency( m_colorComp[colIdx]->GetMainTransparency());  

   el->CSCTakeAnyParentAsMaster();
   m_colorComp[colIdx]->AddElement(el);
}

void
FWViewGeometryList::updateColors()
{ 
   for (int i = 0; i < kFWGeomColorSize; ++i)
   {
      m_colorComp[i]->SetMainColor(m_context.colorManager()->geomColor(FWGeomColorIndex(i)));
      m_colorComp[i]->SetMainTransparency(m_context.colorManager()->geomTransparency(projected()));
      m_colorComp[i]->ElementChanged();
   }
}

void
FWViewGeometryList::updateTransparency(bool projectedType)
{
   //  printf("%p transp [%d]\n", this, iTransp);
   if (projectedType == projected())
   { 
      for (int i = 0; i < kFWGeomColorSize; ++i)
      {
         m_colorComp[i]->SetMainTransparency(m_context.colorManager()->geomTransparency(projectedType));
         m_colorComp[i]->ElementChanged();
      }
   }
}
