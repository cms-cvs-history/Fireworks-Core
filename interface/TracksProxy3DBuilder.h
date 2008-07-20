#ifndef Fireworks_Muons_TracksProxy3DBuilder_h
#define Fireworks_Muons_TracksProxy3DBuilder_h
//
// Original Author:  
//         Created:  Thu Dec  6 18:01:21 PST 2007
// $Id: TracksProxy3DBuilder.h,v 1.2 2008/06/09 19:54:03 chrjones Exp $
//

// system include files

class TEveElementList;
class FWEventItem;
class TEveElement;
class TEveTrackPropagator;
class TEveTrack;
namespace reco {
   class Track;
}
// user include files
#include "Fireworks/Core/interface/FWRPZDataProxyBuilder.h"
#include "Rtypes.h"
#include <vector>

class TracksProxy3DBuilder : public FWRPZDataProxyBuilder
{

   public:
      TracksProxy3DBuilder() {}
      virtual ~TracksProxy3DBuilder() {}

      // ---------- const member functions ---------------------

      // ---------- static member functions --------------------
      static std::vector<TEveTrack*> prepareTrack(const reco::Track& track,
						  TEveTrackPropagator* propagator,
						  TEveElement* trackList,
						  Color_t color);
      static std::vector<TEveTrack*> prepareSimpleTrack(const reco::Track& track,
							TEveTrackPropagator* propagator,
							TEveElement* trackList,
							Color_t color);

      // ---------- member functions ---------------------------
      REGISTER_PROXYBUILDER_METHODS();
   private:
      virtual void build(const FWEventItem* iItem, TEveElementList** product);

      TracksProxy3DBuilder(const TracksProxy3DBuilder&); // stop default

      const TracksProxy3DBuilder& operator=(const TracksProxy3DBuilder&); // stop default

      // ---------- member data --------------------------------

};

#endif
