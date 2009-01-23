{
   gSystem->Load("libCintex.so");
   Cintex::Enable();
   gSystem->Load("libFWCoreFWLite.so");
   gSystem->Load("libFireworksCore.so");
   gSystem->Load("libDataFormatsFWLite.so");
   AutoLibraryLoader::enable();
}
