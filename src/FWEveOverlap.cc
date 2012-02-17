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
   FWGeoTopNode::Paint();

   TEveGeoManagerHolder gmgr( FWGeometryTableViewManager::getGeoMangeur());

   FWGeometryTableManagerBase::Entries_i parentIt = m_browser->getTableManager()->refEntries().begin();
   bool visChld = false;
   int cnt = 0;

   for (FWGeometryTableManagerBase::Entries_i it = parentIt;
        it != m_browser->getTableManager()->refEntries().end(); ++it, ++cnt)
   {
      if (it->m_parent == -1)
      { 
         if (it->testBit(FWGeometryTableManagerBase::kVisNodeSelf) )
            paintShape(*it, cnt, *(it->m_node->GetMatrix()), false);
      }
      else if (it->m_parent == 0)
      {      
         if ((m_browser->m_rnrOverlap.value() && it->testBit(FWOverlapTableManager::kOverlap)) ||
             (m_browser->m_rnrExtrusion.value() && !it->testBit(FWOverlapTableManager::kOverlap)) )
         {          
            if (it->testBit(FWGeometryTableManagerBase::kVisNodeSelf) )
            {
               paintShape(*it, cnt, *(it->m_node->GetMatrix()), false);
            }
            visChld = it->testBit(FWGeometryTableManagerBase::kVisNodeChld);
         }
         else {
            visChld = false;
         }

         parentIt = it;
      }
      else if (visChld && it->testBit(FWGeometryTableManagerBase::kVisNodeSelf) )
      {  
         TGeoHMatrix nm = *(parentIt->m_node->GetMatrix());
         nm.Multiply(it->m_node->GetMatrix());
         paintShape(*it, cnt , nm, false);
      }
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
}
