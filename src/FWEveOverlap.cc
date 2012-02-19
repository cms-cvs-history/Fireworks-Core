#include "Fireworks/Core/src/FWEveOverlap.h"
#include "TGeoOverlap.h"
#include "Fireworks/Core/src/FWOverlapTableView.h"
#include "Fireworks/Core/src/FWOverlapTableManager.h"
#include "Fireworks/Core/interface/FWGeometryTableViewManager.h"
#include "Fireworks/Core/interface/fwLog.h"
#include "Fireworks/Core/src/FWPopupMenu.cc"
//==============================================================================
//==============================================================================
//==============================================================================
FWEveOverlap::FWEveOverlap(FWOverlapTableView* v):
   m_browser(v)
{
} 
FWGeometryTableManagerBase* FWEveOverlap::tableManager()
{
   return m_browser->getTableManager();
}
//______________________________________________________________________________

void FWEveOverlap::Paint(Option_t*)
{

   if (m_browser->getTableManager()->refEntries().empty()) return; 

   FWGeoTopNode::Paint();

   TEveGeoManagerHolder gmgr( FWGeometryTableViewManager::getGeoMangeur());

  
  int topNodeIdx =  m_browser->getTopNodeIdx();

  FWGeometryTableManagerBase::Entries_i sit = m_browser->getTableManager()->refEntries().begin();    std::advance(sit,topNodeIdx );
  TGeoHMatrix mtx;
    m_browser->getTableManager()->getNodeMatrix(*sit, mtx);

  if (sit->testBit(FWGeometryTableManagerBase::kVisNodeSelf))
    paintShape(*sit,  topNodeIdx,mtx, false );
  
  
  if ( m_browser->getTableManager()->getVisibilityChld(*sit))
   paintChildNodesRecurse( sit, topNodeIdx, mtx);
}


// ______________________________________________________________________
void FWEveOverlap::paintChildNodesRecurse (FWGeometryTableManagerBase::Entries_i pIt, Int_t cnt, const TGeoHMatrix& parentMtx)
{ 
   TGeoNode* parentNode =  pIt->m_node;
   int nD = parentNode->GetNdaughters();

   int dOff=0;

   pIt++;
   int pcnt = cnt+1;

   FWGeometryTableManagerBase::Entries_i it;
   for (int n = 0; n != nD; ++n)
   {
      it =  pIt;
      std::advance(it,n + dOff);
      cnt = pcnt + n+dOff;

      TGeoHMatrix nm = parentMtx;
      nm.Multiply(it->m_node->GetMatrix());

      if (it->testBit(FWGeometryTableManagerBase::kVisNodeSelf))
         paintShape(*it, cnt , nm, false);

     if ( it->testBit(FWGeometryTableManagerBase::kVisNodeChld) && it->testBit(FWOverlapTableManager::kOverlapChild))
         paintChildNodesRecurse(it,cnt , nm);



      FWGeometryTableManagerBase::getNNodesTotal(parentNode->GetDaughter(n), dOff);  
   }
}



//______________________________________________________________________________

void FWEveOverlap::popupMenu(int x, int y)
{
 
   if (getFirstSelectedTableIndex() < 0)
   {
      if (fSted.empty()) fwLog(fwlog::kInfo) << "No menu -- no node/entry selected \n";
      return;
   }
   FWPopupMenu* nodePopup = new FWPopupMenu();

   
   nodePopup->AddEntry("Switch Visibility Self ", kOvlSwitchVis);

   nodePopup->AddEntry("Swithc Visibility Mother ", kOvlVisMother);

   nodePopup->AddSeparator();
   nodePopup->AddEntry("Print Overlap Info", kOvlPrintOvl);
   nodePopup->AddEntry("Print Path ", kOvlPrintPath);
   nodePopup->AddSeparator();
   nodePopup->AddEntry("Set As Top Node", kOvlSetTopNode);
   nodePopup->AddEntry("Set Camera Center", kOvlCamera);
   /*
   nodePopup->AddSeparator();
   nodePopup->AddEntry("Rnr Off Everything", kOvlVisOff);
   nodePopup->AddEntry("Rnr On Overlaps, Extrusions", kOvlVisOnOvl);
   nodePopup->AddEntry("Rnr On Mother Volumes", kOvlVisOnAllMother);
   */

   nodePopup->PlaceMenu(x, y,true,true); 
   nodePopup->Connect("Activated(Int_t)",
                      "FWOverlapTableView",
                      m_browser,
                      "chosenItem(Int_t)");
}

//______________________________________________________________________________

TString  FWEveOverlap::GetHighlightTooltip()
{
   //   printf("highlight tooltio \n");
     std::set<TGLPhysicalShape*>::iterator it = fHted.begin();
     int idx = tableIdx(*it);
     if ( idx < 0) 
     {
     return Form("TopNode ");
     }
    
 

      FWGeometryTableManagerBase::NodeInfo& data = m_browser->getTableManager()->refEntries().at(idx);


      return data.name();
     /*
     if (data.m_parent <= 0)
     {
     return data.name();
     }
     else {
     TString pname =  m_browser->getTableManager()->refEntries().at(data.m_parent).name();
     TString text;
     const TGeoOverlap* ovl =  ((FWOverlapTableManager*)m_browser->getTableManager())->referenceOverlap(idx);
     text =  data.name();
     text += Form("\noverlap = %g cm", ovl->GetOverlap());

     if (ovl->IsOverlap()) 
     {
     int nidx = (idx == (data.m_parent + 1) ) ? (data.m_parent + 2) : (data.m_parent + 1);
     text += Form("\nsister = %s", m_browser->getTableManager()->refEntries().at(nidx).name() );
     }
     else
     {  
     text += Form("\nmother = %s",m_browser->getTableManager()->refEntries().at(data.m_parent).m_node->GetVolume()->GetName());
     }
     return text.Data();
     }
   */
}
