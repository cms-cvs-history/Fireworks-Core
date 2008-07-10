#include "FWCore/FWLite/interface/AutoLibraryLoader.h"
#include "Rtypes.h"
#include "TROOT.h"
#include "TApplication.h"
#include "Fireworks/Core/src/CmsShowMain.h"

int main (int argc, char **argv)
{
   char* dummyArgv[] = {"cmsShow"};
   int dummyArgc = 1;
   TApplication app("cmsShow", &dummyArgc, dummyArgv);
   AutoLibraryLoader::enable();
   new CmsShowMain(argc,argv);
   app.Run();
   return 0;
}
