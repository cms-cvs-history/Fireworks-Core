ifeq ($(shell uname), Linux)
   ProjectLibs :=  boost_thread-gcc-mt boost_signals-gcc-mt boost_program_options-gcc-mt CLHEP-1.9.4.2 gsl gslcblas HepMCfio HepMC pcre Cint Core  Net Tree Hist Matrix MathCore MathMore Reflex sigc-2.0 nsl crypt dl z EG Eve FTGL Ged Geom GeomPainter Gpad Graf Graf3d Gui Physics RGL Rint TreePlayer
   ProjectIncludes :=  external/inc/boost/1.34.1-cms/include external/inc/clhep/1.9.3.2-cms/include external/inc/gsl/1.10-cms/include external/inc/hepmc/2.03.06/include external/inc/pcre/4.4-cms/include external/inc/sigcpp/2.0.17-cms/include/sigc++-2.0 external/inc/zlib/1.2.3-cms/include
else 
   ifeq ($(shell uname), Darwin)
      ProjectLibs := boost_filesystem-gcc-mt boost_regex-gcc-mt boost_program_options-gcc-mt CLHEP-1.9.3.2 gsl gslcblas HepMCfio HepMC pcre TreePlayer Gpad Graf3d Graf Hist Matrix Physics Postscript Cint Cintex Core Net Tree MathCore MathMore Reflex sigc-2.0 EG Eve FTGL Ged Geom GeomPainter Gui RGL Rint
      ProjectIncludes :=  external/inc/boost/1.34.1-cms/include external/inc/clhep/1.9.3.2-cms/include external/inc/gsl/1.10/include external/inc/hepmc/2.01.10/include external/inc/pcre/4.4-cms/include external/inc/sigcpp/2.0.17-cms/include/sigc++-2.0 external/inc/zlib/1.2.3-cms/include
   endif     
endif
