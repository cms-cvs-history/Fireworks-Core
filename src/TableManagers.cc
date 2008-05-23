#include <string.h>
#include "TableManagers.h"

std::string format_string (const std::string &fmt, int x)
{
     if (fmt == "%c") {
	  switch (x) {
	  case FLAG_YES:
	       return "yes";
	  case FLAG_NO:
	       return "no";
	  case FLAG_MAYBE:
	       return "maybe";
	  default:
	       return "unknown";
	  }
     } else {
	  char str[100];
	  snprintf(str, 100, fmt.c_str(), x);
	  return str;
     }
}

std::string format_string (const std::string &fmt, double x)
{
     char str[100];
     snprintf(str, 100, fmt.c_str(), x);
     return str;
}

void FWTableManager::MakeFrame (TGMainFrame *parent, int width, int height) 
{
     TGCompositeFrame *tFrame  = new TGCompositeFrame(parent, width, height);
     TGLayoutHints *tFrameHints = 
	  new TGLayoutHints(kLHintsTop|kLHintsLeft|
			    kLHintsExpandX|kLHintsExpandY);
     parent->AddFrame(tFrame,tFrameHints);
     
     TableWidget *widget = new TableWidget(tFrame, this); 
//      widget->HighlightRow(0);
}
