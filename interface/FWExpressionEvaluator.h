// -*- C++ -*-
#ifndef Fireworks_Core_FWExpressionEvaluator_h
#define Fireworks_Core_FWExpressionEvaluator_h
//
// Package:     Core
// Class  :     FWExpressionEvaluator
//
/**\class FWExpressionEvaluator FWExpressionEvaluator.h Fireworks/Core/interface/FWExpressionEvaluator.h

   Description: <one line class summary>

   Usage:
    <usage>

 */
//
// Original Author:  Chris Jones
//         Created:  Fri Feb 29 13:39:51 PST 2008
// $Id: FWExpressionEvaluator.h,v 1.6 2009/01/23 21:35:41 amraktad Exp $
//

// system include files
#include <string>
#include <Reflex/Type.h>

// user include files
#include "PhysicsTools/Utilities/src/SelectorPtr.h"
#include "PhysicsTools/Utilities/src/SelectorBase.h"
#include "PhysicsTools/Utilities/src/ExpressionPtr.h"
#include "PhysicsTools/Utilities/src/ExpressionBase.h"


// forward declarations

class FWExpressionEvaluator {

public:
   FWExpressionEvaluator(const std::string& iExpression,
			 const std::string& iClassName);
   virtual ~FWExpressionEvaluator();

   // ---------- const member functions ---------------------

   const std::string& expression() const;

   double evalExpression(const void*) const;

   // ---------- static member functions --------------------

   // ---------- member functions ---------------------------
   /** Throws an FWExpressionException if there is a problem */
   void setExpression(const std::string& );
   void setClassName(const std::string& );

private:
   //FWExpressionEvaluator(const FWExpressionEvaluator&); // stop default

   //const FWExpressionEvaluator& operator=(const FWExpressionEvaluator&); // stop default

   // ---------- member data --------------------------------
   std::string m_expression;
   std::string m_className;
   reco::parser::ExpressionPtr m_expr;
   ROOT::Reflex::Type m_type;
};

#endif
