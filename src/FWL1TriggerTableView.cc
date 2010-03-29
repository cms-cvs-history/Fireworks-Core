#include "TEveWindow.h"

#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/FWColorManager.h"
#include "Fireworks/Core/interface/FWL1TriggerTableView.h"
#include "Fireworks/Core/interface/FWL1TriggerTableViewManager.h"
#include "Fireworks/Core/interface/FWL1TriggerTableViewTableManager.h"
#include "Fireworks/TableWidget/interface/FWTableWidget.h"

#include "DataFormats/FWLite/interface/Handle.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GtTriggerMenuLite.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerReadoutRecord.h"
#include "FWCore/Utilities/interface/Exception.h"
#include <iomanip>

static const std::string kTableView = "L1TriggerTableView";
static const std::string kColumns = "columns";
static const std::string kSortColumn = "sortColumn";
static const std::string kDescendingSort = "descendingSort";

FWL1TriggerTableView::FWL1TriggerTableView(TEveWindowSlot* parent, FWL1TriggerTableViewManager *manager)
   : m_manager(manager),
     m_tableManager(new FWL1TriggerTableViewTableManager(this)),
     m_tableWidget(0),
     m_currentColumn(-1)
{
   m_columns.push_back(Column("Algorithm Name"));
   m_columns.push_back(Column("Result"));
   m_columns.push_back(Column("Bit Number"));
   m_eveWindow = parent->MakeFrame(0);
   TGCompositeFrame *frame = m_eveWindow->GetGUICompositeFrame();

   m_vert = new TGVerticalFrame(frame);
   frame->AddFrame(m_vert, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

   m_tableWidget = new FWTableWidget(m_tableManager, m_vert);
   resetColors(m_manager->colorManager());
   m_tableWidget->SetHeaderBackgroundColor(gVirtualX->GetPixel(kWhite));
   m_tableWidget->Connect("columnClicked(Int_t,Int_t,Int_t)", "FWL1TriggerTableView",
                          this, "columnSelected(Int_t,Int_t,Int_t)");
   m_vert->AddFrame(m_tableWidget, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
   dataChanged();
   frame->MapSubwindows();
   frame->Layout();
   frame->MapWindow();
}

FWL1TriggerTableView::~FWL1TriggerTableView(void)
{
   // take out composite frame and delete it directly (without the timeout)
   TGCompositeFrame *frame = m_eveWindow->GetGUICompositeFrame();
   frame->RemoveFrame(m_vert);
   delete m_vert;

   m_eveWindow->DestroyWindowAndSlot();
   delete m_tableManager;
}

void
FWL1TriggerTableView::setBackgroundColor(Color_t iColor)
{
   m_tableWidget->SetBackgroundColor(gVirtualX->GetPixel(iColor));
}

void FWL1TriggerTableView::resetColors (const FWColorManager &manager)
{
   m_tableWidget->SetBackgroundColor(gVirtualX->GetPixel(manager.background()));
   m_tableWidget->SetLineSeparatorColor(gVirtualX->GetPixel(manager.foreground()));
}

TGFrame*
FWL1TriggerTableView::frame(void) const
{
   return 0;
}

const std::string&
FWL1TriggerTableView::typeName(void) const
{
   return staticTypeName();
}

void
FWL1TriggerTableView::saveImageTo(const std::string& iName) const
{
  std::cout << "FWL1TriggerTableView::saveImageTo is not implemented." << std::endl;
}

void FWL1TriggerTableView::dataChanged(void)
{
   m_columns.at(0).values.clear();
   m_columns.at(1).values.clear();
   m_columns.at(2).values.clear();
   if(! m_manager->items().empty() && m_manager->items().front() != 0)
   {
      if(fwlite::Event* event = const_cast<fwlite::Event*>(m_manager->items().front()->getEvent()))
      {
	 fwlite::Handle<L1GtTriggerMenuLite> triggerMenuLite;
	 fwlite::Handle<L1GlobalTriggerReadoutRecord> triggerRecord;

	 try
	 {
	    // FIXME: Replace magic strings with configurable ones
	    triggerMenuLite.getByLabel(event->getRun(), "l1GtTriggerMenuLite", "", "HLT");
	    triggerRecord.getByLabel(*event, "gtDigis", "", "HLT");
	 }
	 catch(cms::Exception&)
	 {
	    std::cout << "Warning: no L1Trigger menu is available" << std::endl;
	    m_tableManager->dataChanged();
	    return;
	 }
	 std::ostringstream myCoutStream;
	  
	 if(triggerMenuLite.isValid() && triggerRecord.isValid())
	 {
	   const std::string& triggerMenuInterface = triggerMenuLite->gtTriggerMenuInterface();
	   const std::string& triggerMenuName = triggerMenuLite->gtTriggerMenuName();
	   const std::string& triggerMenuImplementation = triggerMenuLite->gtTriggerMenuImplementation();
	   const std::string& scaleDbKey = triggerMenuLite->gtScaleDbKey();

	   const L1GtTriggerMenuLite::L1TriggerMap& algorithmMap = triggerMenuLite->gtAlgorithmMap();
	   const L1GtTriggerMenuLite::L1TriggerMap& algorithmAliasMap = triggerMenuLite->gtAlgorithmAliasMap();
	   const L1GtTriggerMenuLite::L1TriggerMap& technicalTriggerMap = triggerMenuLite->gtTechnicalTriggerMap();

	   const std::vector<unsigned int>& triggerMaskAlgoTrig = triggerMenuLite->gtTriggerMaskAlgoTrig();
	   const std::vector<unsigned int>& triggerMaskTechTrig = triggerMenuLite->gtTriggerMaskTechTrig();

	   const std::vector<std::vector<int> >& prescaleFactorsAlgoTrig = triggerMenuLite->gtPrescaleFactorsAlgoTrig();
	   const std::vector<std::vector<int> >& prescaleFactorsTechTrig = triggerMenuLite->gtPrescaleFactorsTechTrig();

	   size_t nrDefinedAlgo = algorithmMap.size();
	   size_t nrDefinedTech = technicalTriggerMap.size();

	   const DecisionWord dWord = triggerRecord->decisionWord();

	   // header for printing algorithms
	     
	   myCoutStream << "\n   ********** L1 Trigger Menu - printing   ********** \n"
			<< "\nL1 Trigger Menu Interface: " << triggerMenuInterface
			<< "\nL1 Trigger Menu Name:      " << triggerMenuName
			<< "\nL1 Trigger Menu Implementation: " << triggerMenuImplementation
			<< "\nAssociated Scale DB Key: " << scaleDbKey << "\n\n"
			<< "\nL1 Physics Algorithms: " << nrDefinedAlgo << " algorithms defined." << "\n\n"
			<< "Bit Number "
			<< std::right << std::setw(35) << "Algorithm Name" << "  "
			<< std::right << std::setw(35) << "Algorithm Alias" << "  "
			<< std::right << std::setw(12) << "Trigger Mask";
	   for(unsigned iSet = 0; iSet < prescaleFactorsAlgoTrig.size(); ++iSet)
	   {
	      myCoutStream << std::right << std::setw(10) << "PF Set "
			   << std::right << std::setw(2)  << iSet;
	   }
	     
	   myCoutStream << std::endl;

	   for(L1GtTriggerMenuLite::CItL1Trig itTrig = algorithmMap.begin(), itTrigEnd = algorithmMap.end();
		itTrig != itTrigEnd; ++itTrig)
	   {
	     const unsigned int bitNumber = itTrig->first;
	     const std::string& aName = itTrig->second;
	     int errorCode = 0;
	     const bool result = triggerMenuLite->gtTriggerResult(aName, dWord, errorCode);

	     std::string aAlias;
	     L1GtTriggerMenuLite::CItL1Trig itAlias = algorithmAliasMap.find(bitNumber);
	     if (itAlias != algorithmAliasMap.end()) {
	        aAlias = itAlias->second;
	     }
	     m_columns.at(0).values.push_back(aName);
	     m_columns.at(1).values.push_back(Form("%d",result));
	     m_columns.at(2).values.push_back(Form("%d",bitNumber));
	    
	     myCoutStream << std::setw(6) << bitNumber << "     "
			  << std::right << std::setw(35) << aName << "  "
			  << std::right << std::setw(35) << aAlias << "  "
			  << std::right << std::setw(12) << triggerMaskAlgoTrig[bitNumber];
	     for (unsigned iSet = 0; iSet < prescaleFactorsAlgoTrig.size(); iSet++) {
	        myCoutStream << std::right << std::setw(12) << prescaleFactorsAlgoTrig[iSet][bitNumber];
	     }
	    
	     myCoutStream << std::endl;
	   }
	 }
	 std::cout << myCoutStream.str() << std::endl;
      }
   }
   
   m_tableManager->dataChanged();
}

void
FWL1TriggerTableView::columnSelected(Int_t iCol, Int_t iButton, Int_t iKeyMod)
{
   if (iButton == 1 || iButton == 3)
      m_currentColumn = iCol;
}

// void 
// FWL1TriggerTableView::updateFilter(void)
// {
//    dataChanged();
// }

//
// static member functions
//
const std::string&
FWL1TriggerTableView::staticTypeName(void)
{
   static std::string s_name("L1TriggerTable");
   return s_name;
}

