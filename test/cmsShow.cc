#include "FWCore/FWLite/interface/AutoLibraryLoader.h"
#include "Rtypes.h"
#include "TROOT.h"
#include "TApplication.h"
#include "Fireworks/Core/src/CmsShowMain.h"

int main (int argc, char **argv)
{
   TApplication app("cmsShow", &argc, argv);
   AutoLibraryLoader::enable();
   new CmsShowMain(argc,argv);
   app.Run();
   return 0;
}
