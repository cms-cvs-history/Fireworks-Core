#ifndef Fireworks_Core_FW3DLegoDataProxyBuilder_h
#define Fireworks_Core_FW3DLegoDataProxyBuilder_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FW3DLegoDataProxyBuilder
// 
/**\class FW3DLegoDataProxyBuilder FW3DLegoDataProxyBuilder.h Fireworks/Core/interface/FW3DLegoDataProxyBuilder.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Original Author:  
//         Created:  Sat Jan  5 15:02:03 EST 2008
// $Id: FW3DLegoDataProxyBuilder.h,v 1.4 2008/03/07 09:06:48 dmytro Exp $
//

// system include files

// user include files

// forward declarations
class FWEventItem;
class TH2;
class TObject;
class TEveElementList;
  
namespace fw3dlego
{
  extern const double xbins[83];
}

class FW3DLegoDataProxyBuilder
{

   public:
      FW3DLegoDataProxyBuilder();
      virtual ~FW3DLegoDataProxyBuilder();

      // ---------- const member functions ---------------------

      // ---------- static member functions --------------------

      // ---------- member functions ---------------------------
      void setItem(const FWEventItem* iItem);
      void build(TObject** product);
      virtual void message( int type, int xbin, int ybin ){}

   protected:
      int legoRebinFactor() const {return 1;}
      const FWEventItem* getItem() const { return m_item; }
	
   private:
      virtual void build(const FWEventItem* iItem, TH2** product){}
      virtual void build(const FWEventItem* iItem, 
			 TEveElementList** product){}

      FW3DLegoDataProxyBuilder(const FW3DLegoDataProxyBuilder&); // stop default

      const FW3DLegoDataProxyBuilder& operator=(const FW3DLegoDataProxyBuilder&); // stop default

      // ---------- member data --------------------------------
      const FWEventItem* m_item;

};


#endif
