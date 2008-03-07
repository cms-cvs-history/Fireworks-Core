.PHONY: all

all: 	fireworks

fireworks: fwmain.cc RUNME.C Makefile
	$(CC) -I$(CMSSW_BASE)/src -I$(CMSSW_RELEASE_BASE)/src -I$(ROOTSYS)/include -o fireworks fwmain.cc \
		`root-config --glibs` \
		-L$(CMSSW_RELEASE_BASE)/lib/$(SCRAM_ARCH) -lFWCoreFWLite -lDataFormatsFWLite \
		-L$(CMSSW_BASE)/lib/$(SCRAM_ARCH) -lFireworksCore