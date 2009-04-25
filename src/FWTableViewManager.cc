// -*- C++ -*-
//
// Package:     Core
// Class  :     FWTableViewManager
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:
//         Created:  Sun Jan  6 22:01:27 EST 2008
// $Id: FWTableViewManager.cc,v 1.2.2.9 2009/04/25 18:24:29 jmuelmen Exp $
//

// system include files
#include <iostream>
#include <boost/bind.hpp>
#include <algorithm>
#include "TView.h"
#include "TList.h"
#include "TEveManager.h"
#include "TClass.h"
#include "Reflex/Base.h"
#include "Reflex/Type.h"

// user include files
#include "Fireworks/Core/interface/FWTableViewManager.h"
#include "Fireworks/Core/interface/FWTableView.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/FWGUIManager.h"
#include "Fireworks/Core/interface/FWColorManager.h"

#include "TEveSelection.h"
#include "Fireworks/Core/interface/FWSelectionManager.h"

#include "Fireworks/Core/interface/FWEDProductRepresentationChecker.h"
#include "Fireworks/Core/interface/FWSimpleRepresentationChecker.h"
#include "Fireworks/Core/interface/FWTypeToRepresentations.h"


//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
FWTableViewManager::FWTableViewManager(FWGUIManager* iGUIMgr) :
     FWViewManagerBase()
{
     FWGUIManager::ViewBuildFunctor f;
     f=boost::bind(&FWTableViewManager::buildView,
		   this, _1);
     iGUIMgr->registerViewBuilder(FWTableView::staticTypeName(), f);

     // ---------- for some object types, we have default table contents ----------
     TableEntry muon_table_entries[] = { 
	  { "pt"				, "pT"			, 1 			},
	  { "isGlobalMuon"			, "global"		, TableEntry::BOOL	},
	  { "isTrackerMuon"			, "tracker"		, TableEntry::BOOL	},
	  { "isStandAloneMuon"			, "SA"			, TableEntry::BOOL	},
	  { "isCaloMuon"			, "calo"		, TableEntry::BOOL	},
	  { "track().pt()"			, "tr pt"		, 1 			},
	  { "eta"				, "eta"			, 3 			},
	  { "phi"				, "phi"			, 3 			},
	  { "numberOfMatches('SegmentArbitration')"	, "matches"	, TableEntry::INT	},
	  { "track().d0()"			, "d0"			, 3			},
	  { "track().d0() / track().d0Error()"	, "d0 / d0Err"		, 3			},
     };
     TableEntry electron_table_entries[] = { 
	  { "et"				, "ET"		, 1 	},
	  { "eta"				, "eta"		, 3 	},
	  { "phi"				, "phi"		, 3 	},
	  { "eSuperClusterOverP"		, "E/p"		, 3 	},
	  { "hadronicOverEm"			, "H/E"		, 3 	},
	  { "(trackMomentumAtVtx().R() - trackMomentumOut().R()) / trackMomentumAtVtx().R()"			, "fbrem"	, 3 	},
	  { "deltaEtaSuperClusterTrackAtVtx()"	, "dei"		, 3 	},
	  { "deltaPhiSuperClusterTrackAtVtx()"	, "dpi"		, 3 	} 
     };
     TableEntry genparticle_table_entries[] = { 
	  { "pt"	, "pT"		, 1 			},
	  { "eta"	, "eta"		, 3 			},
	  { "phi"	, "phi"		, 3 			},
	  { "status"	, "status"	, TableEntry::INT 	},
	  { "pdgId"	, "pdgId"	, TableEntry::INT 	},
     };
     TableEntry jet_table_entries[] = { 
	  { "et"	, "ET"		, 1 	},
	  { "eta"	, "eta"		, 3 	},
	  { "phi"	, "phi"		, 3 	},
	  { "p4().E() * emEnergyFraction()"		, "ECAL"	, 1 	},
	  { "p4().E() * energyFractionHadronic()"	, "HCAL"	, 1 	},
	  { "emEnergyFraction()"			, "emf"		, 3 	},
     };
     TableEntry met_table_entries[] = { 
	  { "et"	, "MET"		, 1 	},
	  { "phi"	, "phi"		, 3 	},
	  { "sumEt"	, "sumEt"	, 1 	},
	  { "mEtSig"	, "mEtSig"	, 3 	},
     };
     m_tableFormats["reco::Muon"	].insert(m_tableFormats["reco::Muon"		].end(), muon_table_entries		, muon_table_entries 		+ sizeof(muon_table_entries		) / sizeof(TableEntry));
     m_tableFormats["reco::GsfElectron"	].insert(m_tableFormats["reco::GsfElectron"	].end(), electron_table_entries		, electron_table_entries 	+ sizeof(electron_table_entries		) / sizeof(TableEntry));
     m_tableFormats["reco::GenParticle"	].insert(m_tableFormats["reco::GenParticle"	].end(), genparticle_table_entries	, genparticle_table_entries 	+ sizeof(genparticle_table_entries	) / sizeof(TableEntry));
     m_tableFormats["reco::Jet"		].insert(m_tableFormats["reco::Jet"		].end(), jet_table_entries		, jet_table_entries 		+ sizeof(jet_table_entries		) / sizeof(TableEntry));
     m_tableFormats["reco::MET"		].insert(m_tableFormats["reco::MET"		].end(), met_table_entries		, met_table_entries 		+ sizeof(met_table_entries		) / sizeof(TableEntry));
//      m_tableFormats["reco::Photon"	];
//      m_tableFormats["reco::Track"	];
//      m_tableFormats["reco::Vertex"	];
//      m_tableFormats["l1extra::L1JetParticle"	];
//      m_tableFormats["l1extra::L1EtMissParticle"	];
//      m_tableFormats["l1extra::L1MuonParticle"	];
//      m_tableFormats["l1extra::L1EmParticle"	];
}

FWTableViewManager::~FWTableViewManager()
{
}

//
// member functions
//
std::map<std::string, std::vector<FWTableViewManager::TableEntry> >::iterator 
FWTableViewManager::tableFormats (const Reflex::Type &key) 
{
//      printf("trying to find a table for %s\n", key.Name(ROOT::Reflex::SCOPED).c_str());
     std::map<std::string, std::vector<FWTableViewManager::TableEntry> >::iterator 
	  ret = m_tableFormats.find(key.Name(ROOT::Reflex::SCOPED));
     if (ret != m_tableFormats.end())
	  return ret;
//      for (Reflex::Base_Iterator it = key.Base_Begin(); it != key.Base_End(); ++i) {
// 	  ret = m_tableFormats.find(it->Name(ROOT::Reflex::SCOPED));
// 	  if (ret != m_tableFormats.end())
// 	       return ret;
//      }
     // if there is no exact match for the type, try the base classes
     for (Reflex::Base_Iterator it = key.Base_Begin(); it != key.Base_End(); ++it) {
	  ret = tableFormats(it->ToType());
 	  if (ret != m_tableFormats.end()) {
	       std::pair<std::string, std::vector<FWTableViewManager::TableEntry> > 
		    new_format(key.Name(ROOT::Reflex::SCOPED), ret->second);
	       std::cout << "adding new type " << key.Name(ROOT::Reflex::SCOPED) << std::endl;
	       return m_tableFormats.insert(new_format).first;
	  }
     }
     // if there is no match at all, we just start with a blank table
     std::pair<std::string, std::vector<FWTableViewManager::TableEntry> > 
	  new_format(key.Name(ROOT::Reflex::SCOPED), std::vector<FWTableViewManager::TableEntry>());
     std::cout << "adding new type " << key.Name(ROOT::Reflex::SCOPED) << std::endl;
     return m_tableFormats.insert(new_format).first;
}

std::map<std::string, std::vector<FWTableViewManager::TableEntry> >::iterator 
FWTableViewManager::tableFormats (const TClass &key) 
{
     return tableFormats(Reflex::Type::ByName(key.GetName()));
}

class FWViewBase*
FWTableViewManager::buildView(TEveWindowSlot* iParent)
{
   TEveManager::TRedrawDisabler disableRedraw(gEve);
   boost::shared_ptr<FWTableView> view( new FWTableView(iParent, this) );
   view->setBackgroundColor(colorManager().background());
   m_views.push_back(view);
   view->beingDestroyed_.connect(boost::bind(&FWTableViewManager::beingDestroyed,
					     this,_1));
   return view.get();
}

void
FWTableViewManager::beingDestroyed(const FWViewBase* iView)
{
   for(std::vector<boost::shared_ptr<FWTableView> >::iterator it=
          m_views.begin(), itEnd = m_views.end();
       it != itEnd;
       ++it) {
      if(it->get() == iView) {
         m_views.erase(it);
         return;
      }
   }
}

void
FWTableViewManager::newItem(const FWEventItem* iItem)
{
     m_items.push_back(iItem);
     iItem->goingToBeDestroyed_.connect(boost::bind(&FWTableViewManager::destroyItem,
						    this, _1));
     // tell the views to update their item lists
     for(std::vector<boost::shared_ptr<FWTableView> >::iterator it=
	      m_views.begin(), itEnd = m_views.end();
	 it != itEnd; ++it) {
	  (*it)->updateItems();
	  (*it)->dataChanged();
     }
}

void FWTableViewManager::destroyItem (const FWEventItem *item)
{
     // remove the item from the list
     for (std::vector<const FWEventItem *>::iterator it = m_items.begin(), 
	       itEnd = m_items.end();
	  it != itEnd; ++it) {
	  if (*it == item) {
	       m_items.erase(it);
	       break;
	  }
     }
     // tell the views to update their item lists
     for(std::vector<boost::shared_ptr<FWTableView> >::iterator it=
	      m_views.begin(), itEnd = m_views.end();
	 it != itEnd; ++it) {
	  (*it)->updateItems();
	  (*it)->dataChanged();
     }
}

void
FWTableViewManager::modelChangesComing()
{
   gEve->DisableRedraw();
   printf("changes coming\n");
}

void
FWTableViewManager::modelChangesDone()
{
     gEve->EnableRedraw();
     // tell the views to update their item lists
     for(std::vector<boost::shared_ptr<FWTableView> >::iterator it=
	      m_views.begin(), itEnd = m_views.end();
	 it != itEnd; ++it) {
	  (*it)->dataChanged();
     }
     printf("changes done\n");
}

void
FWTableViewManager::colorsChanged()
{
   for(std::vector<boost::shared_ptr<FWTableView> >::iterator it=
       m_views.begin(), itEnd = m_views.end();
       it != itEnd;
       ++it) {
	(*it)->resetColors(colorManager());
//       printf("Changed the background color for a table to 0x%x\n", 
// 	     colorManager().background());
   }
}

void
FWTableViewManager::dataChanged()
{
     for(std::vector<boost::shared_ptr<FWTableView> >::iterator it=
	      m_views.begin(), itEnd = m_views.end();
	 it != itEnd;
	 ++it) {
	  (*it)->dataChanged();
//       printf("Changed the background color for a table to 0x%x\n", 
// 	     colorManager().background());
   }
}

FWTypeToRepresentations
FWTableViewManager::supportedTypesAndRepresentations() const
{
   FWTypeToRepresentations returnValue;
//    const std::string kSimple("simple#");

//    for(TypeToBuilders::const_iterator it = m_typeToBuilders.begin(), itEnd = m_typeToBuilders.end();
//        it != itEnd;
//        ++it) {
//       for ( std::vector<std::string>::const_iterator builderName = it->second.begin();
//             builderName != it->second.end(); ++builderName )
//       {
//          if(builderName->substr(0,kSimple.size()) == kSimple) {
//             returnValue.add(boost::shared_ptr<FWRepresentationCheckerBase>( new FWSimpleRepresentationChecker(
//                                                                                builderName->substr(kSimple.size(),
//                                                                                                    builderName->find_first_of('@')-kSimple.size()),
//                                                                                it->first)));
//          } else {

//             returnValue.add(boost::shared_ptr<FWRepresentationCheckerBase>( new FWEDProductRepresentationChecker(
//                                                                                builderName->substr(0,builderName->find_first_of('@')),
//                                                                                it->first)));
//          }
//       }
//    }
   return returnValue;
}

