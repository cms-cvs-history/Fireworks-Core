#include "FWCore/FWLite/interface/AutoLibraryLoader.h"
#include "Cintex/Cintex.h"
#include "Rtypes.h"
#include "TROOT.h"
#include "TApplication.h"

#include "RUNME.C"

int main (int argc, char **argv)
{
     TApplication app("FireWorks", &argc, argv);
     ROOT::Cintex::Cintex::Enable();
     AutoLibraryLoader::enable();
     RUNME();
     return 0;
}
