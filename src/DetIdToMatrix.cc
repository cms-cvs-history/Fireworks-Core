#include "Fireworks/Core/interface/DetIdToMatrix.h"
#include "DataFormats/MuonDetId/interface/MuonSubdetId.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "TGeoManager.h"
#include "TFile.h"
#include "TTree.h"
#include "TEveGeoShapeExtract.h"
#include "TEveTrans.h"
#include "TColor.h"
#include "TROOT.h"
#include <iostream>
#include <sstream>
DetIdToMatrix::~DetIdToMatrix()
{
   // ATTN: not sure I own the manager
   // if ( manager_ ) delete manager_;
}

void DetIdToMatrix::loadGeometry(const char* fileName) 
{
   // ATTN: not sure if I can close the file and keep the manager in the memory
   //       it's not essential for id to matrix functionality, but the geo manager
   //       should be available for access to the geometry if it's needed
   TFile* f = new TFile(fileName);
   manager_ = 0;
   // load geometry
   manager_ = (TGeoManager*)f->Get("cmsGeo");
   if (!manager_) {
      std::cout << "ERROR: cannot find geometry in the file. Initialization failed." << std::endl;
      return;
   }
}

void DetIdToMatrix::loadMap(const char* fileName) 
{
   // CSC chamber frame has local coordinates rotated with respect
   // to the reference framed used in the offline reconstruction
   // -z is endcap is also reflected
   TGeoRotation inverseCscRotation("iCscRot",0,90,0); 
   
   if (!manager_) {
      std::cout << "ERROR: CMS detector geometry is not available. DetId to Matrix map Initialization failed." << std::endl;
      return;
   }
   
   TFile f(fileName);
   // ATTN: not sure who owns the object 
   TTree* tree = (TTree*)f.Get("idToGeo");
   if (!tree) {
      std::cout << "ERROR: cannot find detector id map in the file. Initialization failed." << std::endl;
      return;
   }
   unsigned int id;
   char path[1000];
   tree->SetBranchAddress("id",&id);
   tree->SetBranchAddress("path",&path);
   for ( unsigned int i = 0; i < tree->GetEntries(); ++i) {
      tree->GetEntry(i);
      if ( manager_->cd(path) ) {
	 idToPath_[id] = path;
	 DetId detId(id);
	 if ( detId.subdetId() == MuonSubdetId::CSC ) {
	    TGeoHMatrix m = (*(manager_->GetCurrentMatrix()))*inverseCscRotation;
	    if ( m.GetTranslation()[2]<0 ) m.ReflectX(kFALSE);
	    idToMatrix_[id] = m;
	 }
	 else
	   idToMatrix_[id] = *(manager_->GetCurrentMatrix());
      }
      else
	std::cout << "WARNING: incorrect path " << path << "\nSkipped DetId: " << id << std::endl;
   }
   f.Close();
}

const TGeoHMatrix* DetIdToMatrix::getMatrix( unsigned int id ) const
{
   std::map<unsigned int, TGeoHMatrix>::const_iterator itr = idToMatrix_.find(id);
   if ( itr != idToMatrix_.end() )
     return &(itr->second);
   else
     return 0;
}

const char* DetIdToMatrix::getPath( unsigned int id ) const 
{
   std::map<unsigned int, std::string>::const_iterator itr = idToPath_.find(id);
   if ( itr != idToPath_.end() )
     return itr->second.c_str();
   else
     return 0;
}
   
const TGeoVolume* DetIdToMatrix::getVolume( unsigned int id ) const
{
   std::map<unsigned int, std::string>::const_iterator itr = idToPath_.find(id);
   if ( itr != idToPath_.end() ) {
      manager_->cd(itr->second.c_str());
      return manager_->GetCurrentVolume();
   }
   else
     return 0;
}
   
std::vector<unsigned int> DetIdToMatrix::getAllIds() const
{
   std::vector<unsigned int> ids;
   for ( std::map<unsigned int, std::string>::const_iterator itr = idToPath_.begin(); itr != idToPath_.end(); ++itr )
     ids.push_back( itr->first );
   return ids;
}
   

TEveGeoShapeExtract* DetIdToMatrix::getExtract(const char* path, const char* name) const
{
   if ( ! manager_ || ! path || ! name ) return 0;
   manager_->cd(path);
   TGeoMatrix* matrix = manager_->GetCurrentMatrix();
   const Double_t* rm = matrix->GetRotationMatrix();
   const Double_t* tv = matrix->GetTranslation();
   TEveTrans t;
   t(1,1) = rm[0]; t(1,2) = rm[1]; t(1,3) = rm[2];
   t(2,1) = rm[3]; t(2,2) = rm[4]; t(2,3) = rm[5];
   t(3,1) = rm[6]; t(3,2) = rm[7]; t(3,3) = rm[8];
   t(1,4) = tv[0]; t(2,4) = tv[1]; t(3,4) = tv[2];
   
   TEveGeoShapeExtract* extract = new TEveGeoShapeExtract(name,path);
   extract->SetTrans(t.Array());
   
   TGeoVolume* volume = manager_->GetCurrentVolume();
   Int_t ci = volume->GetLineColor();
   TColor* c = gROOT->GetColor(ci);
   Float_t rgba[4] = { 1, 0, 0, 1 };
   if (c) {
      rgba[0] = c->GetRed();
      rgba[1] = c->GetGreen();
      rgba[2] = c->GetBlue();
   }
   
   extract->SetRGBA(rgba);
   extract->SetRnrSelf(kTRUE);
   extract->SetRnrElements(kTRUE);
   extract->SetShape( manager_->GetCurrentVolume()->GetShape() );
   return extract;
}

TEveGeoShapeExtract* DetIdToMatrix::getExtract( unsigned int id ) const
{
   std::ostringstream s;
   s << id;
   return getExtract( getPath(id), s.str().c_str() );
}

TEveGeoShapeExtract* DetIdToMatrix::getAllExtracts(const char* elementListName /*= "CMS"*/) const
{
   TEveGeoShapeExtract* container = new TEveGeoShapeExtract( elementListName );
   for ( std::map<unsigned int, std::string>::const_iterator itr = idToPath_.begin(); itr != idToPath_.end(); ++itr )
     container->AddElement( getExtract(itr->first) );
   return container;
}

