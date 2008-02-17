#ifndef Fireworks_Calo_ElectronsProxySCBuilder_h
#define Fireworks_Calo_ElectronsProxySCBuilder_h
// -*- C++ -*-
//
// Package:     Calo
// Class  :     ElectronsProxySCBuilder
// 
/**\class ElectronsProxySCBuilder ElectronsProxySCBuilder.h Fireworks/Calo/interface/ElectronsProxySCBuilder.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Original Author:  
//         Created:  Sun Jan  6 23:42:33 EST 2008
// $Id: ElectronsProxySCBuilder.h,v 1.1.2.1 2008/02/15 21:48:18 jmuelmen Exp $
//

// system include files

// user include files

// forward declarations

class FWEventItem;
class TEveElementList;

class ElectronsProxySCBuilder {
     
public:
     ElectronsProxySCBuilder();
     virtual ~ElectronsProxySCBuilder();
     
     // ---------- const member functions ---------------------
     
     // ---------- static member functions --------------------
     
     // ---------- member functions ---------------------------
     
  //    virtual void buildRhoPhi(const FWEventItem* iItem,
// 			      TEveElementList** product);
     
//      virtual void buildRhoZ(const FWEventItem* iItem, 
// 			    TEveElementList** product);
     void setItem (const FWEventItem *iItem) { m_item = iItem; }
     void build (TEveElementList **product);

private:
     ElectronsProxySCBuilder(const ElectronsProxySCBuilder&); // stop default
     
     const ElectronsProxySCBuilder& operator=(const ElectronsProxySCBuilder&); // stop default
     
     // ---------- member data --------------------------------
     const FWEventItem* m_item;
};


#endif
