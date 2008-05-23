// -*- C++ -*-
#ifndef Fireworks_Core_FWTextView_h
#define Fireworks_Core_FWTextView_h

namespace fwlite {
     class Event;
}

class ElectronTableManager;
class MuonTableManager;
class JetTableManager;
class TGMainFrame;

class FWTextView {
public:
     FWTextView ();
     void newEvent (const fwlite::Event &);
protected:
     ElectronTableManager	*el_manager;
     MuonTableManager		*mu_manager;
     JetTableManager		*jet_manager;
     TGMainFrame		*fMain;
};

#endif
