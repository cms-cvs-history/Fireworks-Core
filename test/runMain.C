#ifndef __CINT__
#include "CmsShow/Core/interface/CmsShowMain.h"
#endif

void runMain() {
  //  gSystem->Load("libCMSDataFormats.so");
  //  gSystem->Load("libCMSDataFormats");
  //  AutoLibraryLoader::enable();
   std::vector<std::string> args;
   args.push_back("cmsShow");
   
   const char* argEnv = gSystem->Getenv("CMSSHOW_ARGS");
   if(argEnv) {
      std::string argString(argEnv);
      //for now assume double quotes and just treat spaces as a delimiter
      int start=0;
      int find = 0;
      do {
	 find = argString.find(" ",start);
	 if(find != std::string::npos) {
	    args.push_back(argString.substr(start,find-start));
	    start = find+1;
	 } else {
	    args.push_back(argString.substr(start));
	 }
	 cout <<"'"<<args.back()<<"'"<<endl;
      } while(find != std::string::npos) ;
   }
   char* argv[20];
   if( args.size() > 20 ) {
      cout <<"Too many arguments passed"<<endl;
      exit(1);
   }
   int argc=0;
   for(; argc != args.size();++argc) {
      argv[argc]=args[argc].c_str();
   }
   new CmsShowMain(argc, argv);
   //  new FWGUIManager("data.root", "myconfig.fwc", false);
   //  new CmsShowMainFrame(gClient->GetRoot(), 1024, 768);
}
