#ifndef Fireworks_Core_FWOverlapTableManager_h
#define Fireworks_Core_FWOverlapTableManager_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWOverlapTableManager
// 
/**\class FWOverlapTableManager FWOverlapTableManager.h Fireworks/Core/interface/FWOverlapTableManager.h

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  
//         Created:  Wed Jan  4 20:34:38 CET 2012
// $Id: FWOverlapTableManager.h,v 1.1.2.7 2012/01/20 01:55:15 amraktad Exp $
//

#include "Fireworks/Core/interface/FWGeometryTableManagerBase.h"
#include "TGeoMatrix.h"
#include "TGeoNode.h"
#include "TGeoOverlap.h"



class FWOverlapTableView;
class TGeoOverlap;

class FWOverlapTableManager : public FWGeometryTableManagerBase
{
   struct Overlap
   {
      Overlap(TGeoOverlap* geoOverlap)
      {
         firstFoundFirst = true;
         ovl = geoOverlap; 
         n1 = n2 = mothern = 0;
         v1 = v2 = motherv = 0;
         l1 = l2 = -1;
      }
      /*
      bool matchFirstNode(TGeoNode* n, TGeoIterator& it)
      {
         if (n->GetVolume() == ovl->GetFirstVolume() 
         {
            firstFoundFirst = true;
            n1 = n;
            v1 = n->GetVolume();
            l1 = it.GetLevel();
            it.GetPath(iteratorPath1);
            printf("Match first volume first[1] %s \n",iteratorPath1.Data());
            return true;
         }
         else if (n->GetVolume() == ovl->GetSecondVolume()
         {
            if (n->GetMatrix() != ovl->GetSecondMatrix()) return false;
            firstFoundFirst = false;
            n2 = n;
            v2 = n->GetVolume();
            l2 = it.GetLevel();
            it.GetPath(iteratorPath2);
            printf("Match first volume first[0] %s \n",iteratorPath2.Data());
            return true;
         }
         return false;
      }


      bool matchSecondNode(TGeoNode* n, TGeoIterator& it)
      {
         if (firstFoundFirst && n->GetVolume() == ovl->GetSecondVolume())
         {
            n2 = n;
            v2 = n->GetVolume();
            l2 = it.GetLevel();
            it.GetPath(iteratorPath2);
            printf("Matched second firstnodefirst[1] %s \n",iteratorPath2.Data());
            return true;
         }
         else if (!firstFoundFirst && n->GetVolume() == ovl->GetFirstVolume())
         {
            n1 = n;
            v1 = n->GetVolume();
            l1 = it.GetLevel();
            it.GetPath(iteratorPath1);
            printf("Matched second firstnodefirst[0] %s \n",iteratorPath1.Data());
            return true;
         }
         return false;
      }*/

      bool firstFoundFirst;

      TGeoOverlap* ovl;
      TGeoNode    *n1, *n2, *mothern;
      TGeoVolume  *v1, *v2, *motherv;

      TGeoHMatrix  motherm;
      Int_t        l1,l2, motherl;

      TString iteratorMotherPath, iteratorPath1, iteratorPath2;
   };

public:
   FWOverlapTableManager(FWOverlapTableView*);
   virtual ~FWOverlapTableManager();

   virtual void recalculateVisibility();
   void importOverlaps(std::string path, double precision);

   virtual int numberOfColumns() const {return 6;}

   virtual std::vector<std::string> getTitles() const;
 
   FWTableCellRendererBase* cellRenderer(int iSortedRowNumber, int iCol) const;

   const TGeoOverlap* referenceOverlap(int idx) const;

protected:
   virtual bool nodeIsParent(const NodeInfo&) const;
   virtual  const char* cellName(const NodeInfo& data) const;

private:
   FWOverlapTableManager(const FWOverlapTableManager&); // stop default
   const FWOverlapTableManager& operator=(const FWOverlapTableManager&); // stop default

   void addOverlapEntry(Overlap&);
   FWOverlapTableView* m_browser;
};


#endif
