// -*- C++ -*-
#ifndef Fireworks_Core_FWTextView_h
#define Fireworks_Core_FWTextView_h

#include "RQ_OBJECT.h"
#include <vector>

namespace fwlite {
     class Event;
}

class ElectronTableManager;
class MuonTableManager;
class JetTableManager;
class HLTTableManager;
class TrackTableManager;
class VertexTableManager;
class TGMainFrame;
class TGTab;
class TGTextEntry;
class TGCheckButton;
class TGCompositeFrame;
class TGTransientFrame;
class FWTableManager;
class FWTextView;
class FWDisplayEvent;
class FWSelectionManager;
class FWModelChangeManager;
class FWEventItem;
class FWGUIManager;
class CmsShowMain;

class FWTextViewPage {
     RQ_OBJECT("FWTextViewPage") 
public:
     FWTextViewPage (const std::string &title, 
		     const std::vector<FWTableManager *> &tables,
		     TGCompositeFrame *frame,
		     TGTab *parent_tab,
		     FWTextView *view);
     void	setNext (FWTextViewPage *);
     void	setPrev (FWTextViewPage *);
     void	deselect ();
     void	select ();
     void	update ();
     void	undock ();
     void	redock ();
     void	dumpToFile ();
     void	dumpToPrinter ();
		     
public:
     std::string			title;
     std::vector<FWTableManager *>	tables;
     TGCompositeFrame			*frame;
     TGTab				*parent_tab;
     TGTransientFrame			*undocked;
     TGCompositeFrame			*parent;
     TGTextEntry			*file_name;
     TGCheckButton			*append_button;
     TGTextEntry			*print_command;
     FWTextView				*view;
     FWTextViewPage			*prev;
     FWTextViewPage			*next;
     TGCompositeFrame			*nav_frame;
};

class FWTextView {
     RQ_OBJECT("FWTextView") 
public:
     FWTextView (CmsShowMain *, FWSelectionManager *, FWModelChangeManager *,
		 FWGUIManager *);
     void newEvent (const fwlite::Event &, const CmsShowMain *);
     void nextPage ();
     void prevPage ();
     void selectionChanged (const FWSelectionManager &);
     void itemChanged (const FWEventItem *);
     void changesDone (const CmsShowMain *);
     void update (int tab);

protected:
     // objects
     ElectronTableManager	*el_manager;
     MuonTableManager		*mu_manager;
     JetTableManager		*jet_manager;
     // trigger
     ElectronTableManager	*l1_manager;
     HLTTableManager		*hlt_manager;
     // tracks
     TrackTableManager		*track_manager;
     VertexTableManager		*vertex_manager;
//       TGMainFrame		*fMain;

     std::vector<FWTableManager *>	managers;

     // display pages
     FWTextViewPage		*page;
     FWTextViewPage		*pages[3];

     FWSelectionManager		*seleman;
     FWModelChangeManager	*changeman;

     TGTab			*parent_tab;
};

#endif
