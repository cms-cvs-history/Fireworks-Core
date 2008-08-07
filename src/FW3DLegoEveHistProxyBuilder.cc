// -*- C++ -*-
//
// Package:     Core
// Class  :     FW3DLegoEveHistProxyBuilder
// 
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Sat Jul  5 11:26:11 EDT 2008
// $Id: FW3DLegoEveHistProxyBuilder.cc,v 1.2 2008/07/09 20:05:27 chrjones Exp $
//

// system include files
#include "TEveCaloData.h"
#include "TH2F.h"

// user include files
#include "Fireworks/Core/interface/FW3DLegoEveHistProxyBuilder.h"
#include "Fireworks/Core/interface/FWEventItem.h"


//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
FW3DLegoEveHistProxyBuilder::FW3DLegoEveHistProxyBuilder():
m_hist(0), m_data(0)
{
}

// FW3DLegoEveHistProxyBuilder::FW3DLegoEveHistProxyBuilder(const FW3DLegoEveHistProxyBuilder& rhs)
// {
//    // do actual copying here;
// }

FW3DLegoEveHistProxyBuilder::~FW3DLegoEveHistProxyBuilder()
{
}

//
// assignment operators
//
// const FW3DLegoEveHistProxyBuilder& FW3DLegoEveHistProxyBuilder::operator=(const FW3DLegoEveHistProxyBuilder& rhs)
// {
//   //An exception safe implementation is
//   FW3DLegoEveHistProxyBuilder temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void 
FW3DLegoEveHistProxyBuilder::attach(TEveElement* iElement,
                                    TEveCaloData* iHist)
{
   m_data = iHist;
}

void 
FW3DLegoEveHistProxyBuilder::build()
{
  /*
   build(item(),&m_hist);
   TEveCaloDataHist* data = dynamic_cast<TEveCaloDataHist*>(m_data);
   if(0!=m_hist && -1 == m_sliceIndex && data) {
      m_sliceIndex = data->AddHistogram(m_hist);
      data->RefSliceInfo(m_sliceIndex).Setup(item()->name().c_str(), 0., item()->defaultDisplayProperties().color());
   }
   m_data->DataChanged();
  */
   build(item(),&m_hist);
   TEveCaloDataVec* data = dynamic_cast<TEveCaloDataVec*>(m_data);
   if(0!=m_hist && data) {
      fillCorrectlyBinnedLego( data, m_hist, slice() );
      data->RefSliceInfo(slice()).Setup(item()->name().c_str(), 0., item()->defaultDisplayProperties().color());
   }
   m_data->DataChanged();
}

void 
FW3DLegoEveHistProxyBuilder::modelChangesImp(const FWModelIds&)
{
   applyChangesToAllModels();
   m_data->SetSliceColor(slice(),item()->defaultDisplayProperties().color());
   m_data->DataChanged();
}

void 
FW3DLegoEveHistProxyBuilder::itemChangedImp(const FWEventItem*)
{
   
}

void 
FW3DLegoEveHistProxyBuilder::itemBeingDestroyedImp(const FWEventItem* iItem)
{
   m_hist->Reset();
   m_data->DataChanged();
}


//
// const member functions
//

//
// static member functions
//

void 
FW3DLegoEveHistProxyBuilder::fillCorrectlyBinnedLego( TEveCaloDataVec* data, 
						      const TH2F* histo,
						      unsigned int slice )
{
   if ( data->GetNCells() > 0 && data->GetNCells()!=histo->GetSize() ) {
      std::cout << 
	"new histogram has different size than already registered slices. Skip" 
	<< std::cout;
      return;
   }
   
   const float epsilon = 1e-3;
	 
   if ( data->GetNCells() == 0 ) {
      // make towers
      Int_t xNBins = histo->GetXaxis()->GetNbins();
      Int_t yNBins = histo->GetYaxis()->GetNbins();
      for ( int i = 0; i < histo->GetSize(); ++i ) {
	 Int_t binx(0), biny(0), binz(0);
	 histo->GetBinXYZ(i,binx,biny,binz);
	 // float xWidth = histo->GetXaxis()->GetBinWidth(binx);
	 float yWidth = histo->GetYaxis()->GetBinWidth(biny);
	 float xLow =   histo->GetXaxis()->GetBinLowEdge(binx);
	 float yLow =   histo->GetYaxis()->GetBinLowEdge(biny);
	 float xUp =    histo->GetXaxis()->GetBinUpEdge(binx);
	 float yUp =    histo->GetYaxis()->GetBinUpEdge(biny);
	 
	 // special treatment for the endcaps
	 // |eta| in [0, 1.74], 72 phi bins
	 // |eta| in [1.74, 4.716], 36 phi bins
	 // |eta| in [4.716, 5.191], 18 phi bins
	 
	 // NOTE:
	 //       Here we assume 72 bins in phi. At high eta we have only 36 and at the
	 //       very end 18 bins. These large bins are splited among smaller bins 
	 //       decreasing energy in each entry by factor of 2 and 4 for 36 and 18 bin 
	 //       cases. Other options will be implemented later
	 // 
	 // http://ecal-od-software.web.cern.ch/ecal-od-software/documents/documents/cal_newedm_roadmap_v1_0.pdf
	 // Eta mapping:
	 //   ieta - [-41,-1]+[1,41] - total 82 bins 
	 //   calo tower gives eta of the ceneter of each bin
	 //   size:
	 //      0.087 - [-20,-1]+[1,20]
	 //      the rest have variable size from 0.09-0.30
	 // Phi mapping:
	 //   iphi - [1-72]
	 //   calo tower gives phi of the center of each bin
	 //   for |ieta|<=20 phi bins are all of the same size
	 //      iphi 36-37 transition corresponds to 3.1 -> -3.1 transition
	 //   for 20 < |ieta| < 40
	 //      there are only 36 active bins corresponding to odd numbers
	 //      iphi 35->37, corresponds to 3.05 -> -3.05 transition
	 //   for |ieta| >= 40
	 //      there are only 18 active bins 3,7,11,15 etc
	 //      iphi 31 -> 35, corresponds to 2.79253 -> -3.14159 transition
	 
	 // all bins start from 1, but information is stored in
	 // 1,5,9 etc for |eta|>4.71
	 // 2,4,6 etc for |eta| in [1.74, 4.716]
	 // we will make non-informative bins very thin
	 
	 if ( binx > 0 && binx <= xNBins &&
	      biny > 0 && biny <= yNBins )
	   {
	      if ( xUp < -4.71 || xLow > 4.71 ){
		 switch ( (biny-1)%4 ) {
		  case 0: // information is here
		    yUp  += 3*(yWidth-epsilon);
		    break;
		  case 1:
		    yLow += 3*(yWidth-epsilon);
		    yUp = yLow + epsilon;
		    break;
		  case 2:
		    yLow += 2*(yWidth-epsilon);
		    yUp = yLow + epsilon;
		    break;
		  case 3:
		    yLow = yUp - epsilon;
		    break;
		 }
	      }
	      if ( ( xLow > -4.72 && xUp  < -1.73 ) ||
		   ( xUp  <  4.72 && xLow > 1.73 ) )
		 switch ( (biny-1)%2 ) {
		  case 0: 
		    yUp = yLow + epsilon;
		    break;
		  case 1: // information is here
		    yLow -= (yWidth-epsilon);
		    break;
		 }
	   }
	      
	 // make tiny overflow bins
	 if ( binx == 0 )        xLow = xUp  - epsilon;
	 if ( binx == xNBins+1 ) xUp  = xLow + epsilon;
	 if ( biny == 0 )        yLow = yUp  - epsilon;
	 if ( biny == yNBins+1 ) yUp  = yLow + epsilon;
	 
	 data->AddTower( xLow, xUp, yLow, yUp );
	 data->FillSlice( slice, i, histo->GetBinContent(binx,biny) );
      }
      data->SetEtaBins( histo->GetXaxis() );
      data->SetPhiBins( histo->GetYaxis() );
   } else {
      for ( int i = 0; i < histo->GetSize(); ++i ) {
	 Int_t binx(0), biny(0), binz(0);
	 histo->GetBinXYZ(i,binx,biny,binz);
	 data->FillSlice( slice, i, histo->GetBinContent(binx,biny) );
      }
   }
}
