#!/bin/bash

getExternals()
{
   mkdir  ${tard}/external

   # can be linked or installed at $ROOTSYS   
   ROOTSYS=`echo $ROOTSYS |  sed 's/\/$//'` # remove '/' character  at end of string, becuse it invalidates symblic link interpretation
   origr=$ROOTSYS
   if [ -L ${ROOTSYS} ]; then
      b=`dirname ${ROOTSYS}`   
      if [ `uname` = "Linux" ]; then
          origr=`readlink -f ${ROOTSYS}`
      else
          origr=`readlink ${ROOTSYS}`
      fi
   fi
   
   echo "copy root from $origr to ${tard}/external/root"
   cp -a $origr  ${tard}/external/root
   
   
   # external libraries
   echo "=========================================================="
   echo "=========================================================="
   extdl=${tard}/external/lib
   mkdir $extdl
   
   ext=`dirname ${CMSSW_DATA_PATH}`/external
   
   # on linux ship compiler
   gccd=${tard}/external
   if [ `uname` = "Linux" ]; then
      echo "Copy gcc from  $ext/gcc/4.3.4-cms/ to ${gccd}"
      cp -a $ext/gcc/4.3.4-cms/ ${gccd}/gcc
   fi
   
   echo "Copying external libraries from $ext to $extdl."
   # cp -a $ext/*/*/lib/*  ${tard}/external/lib
   for i in boost bz2lib castor clhep dcap db4 dcap \
   	elementtree expat fftw3 gdbm gsl hepmc\
   	libjpg libpng libtiff libungif \
   	openldap openssl pcre \
   	sigcpp sqlite xrootd zlib
   do
   	echo "cp -a $ext/$i/*/lib/* === ${extdl}"
   	cp -a $ext/$i/*/lib/* ${extdl}
   done
   
}
   
#----------------------------------------------------------------

getCmssw()
{
   echo "=========================================================="
   echo "=========================================================="
     
   # cms libraries
   mkdir -p ${tard}/lib
   
   if [ X$patchedBuild = Xon ]; then
      echo "getting libs from $CMSSW_RELEASE_BASE/lib/*/* ${tard}/lib/"
      cp -a $CMSSW_RELEASE_BASE/lib/*/* ${tard}/lib/
   fi
   
   echo "getting libs from $CMSSW_BASE/lib/*/ "
   cp -f $CMSSW_BASE/lib/*/* ${tard}/lib/
   
   # plugins cache file
   if [ X$patchedBuild = Xon ]; then
      echo "get $CMSSW_RELEASE_BASE/lib/*/.edmplugincache > ${tard}/lib/.edmplugincache"
      touch ${tard}/lib/.edmplugincache
      cat  $CMSSW_RELEASE_BASE/lib/*/.edmplugincache | grep -v Fireworks > /tmp/.edmplugincache
      cat  $CMSSW_BASE/lib/*/.edmplugincache >> /tmp/.edmplugincache
      cat /tmp/.edmplugincache | sort -u >  ${tard}/lib/.edmplugincache
   else
      echo "cp $CMSSW_BASE/lib/*/.edmplugincache  ${tard}/lib/.edmplugincache"
      cp $CMSSW_BASE/lib/*/.edmplugincache  ${tard}/lib/.edmplugincache
   fi
}

#----------------------------------------------------------------

getSources()
{
   echo "=========================================================="
   echo "getting sources."
   # binary 
   mkdir ${tard}/libexec
   cp $CMSSW_BASE/bin/*/cmsShow.exe ${tard}/libexec
   
   # src
   srcDir="${tard}/src/Fireworks/Core"
   mkdir -p $srcDir
   cp -a  $CMSSW_BASE/src/Fireworks/Core/macros $srcDir
   cp -a  $CMSSW_BASE/src/Fireworks/Core/icons $srcDir
   cp -a  $CMSSW_BASE/src/Fireworks/Core/data $srcDir
   cp -a  $CMSSW_BASE/src/Fireworks/Core/scripts $srcDir
   
   cd  $tard
   ln -s  src/Fireworks/Core/macros/default.fwc .
   ln -s  src/Fireworks/Core/macros/ispy.fwc  .
   ln -s  src/Fireworks/Core/macros/pflow.fwc  .
   ln -s  src/Fireworks/Core/macros/hfLego.fwc  
   ln -s  src/Fireworks/Core/macros/simGeo.fwc  .
   
   ln -s  src/Fireworks/Core/scripts/cmsShow .
   
   cp  $CMSSW_DATA_PATH/data-Fireworks-Geometry/4-cms/Fireworks/Geometry/data/* $tard
}

#----------------------------------------------------------------

getDataFiles()
{
   # sample files
   cd $tard
   dwnCmd="wget"
   if [ `uname` = "Darwin" ]; then
      # compatibility problem on 10.5
      dwnCmd="curl -O"
   fi
   cd ${tard}
   name=`perl -e '($ver, $a, $b, $c) = split('_', $ENV{CMSSW_VERSION}); print  "data", $a, $b, ".root"  '`
   $dwnCmd http://amraktad.web.cern.ch/amraktad/mail/scratch0/data/$name
   mv $name data.root
   $dwnCmd http://amraktad.web.cern.ch/amraktad/mail/scratch0/data/cmsSimGeom-14.root
}

#----------------------------------------------------------------

makeTar()
{
   cd $origd
   bdir=`basename $tard`
   if [ `uname` = "Darwin" ]; then
       echo "Packing tarball ${bdir}.mac.tar.gz"
       tar -czf ${bdir}.mac.tar.gz $bdir
   else
       echo "Packing tarball ${bdir}.linux.tar.gz"
       tar -czf ${bdir}.linux.tar.gz $bdir
   fi
}


#----------------------------------------------------------------
#----------------------------------------------------------------
#----------------------------------------------------------------

if [ $# -lt 1 ]; then
  echo "Usage: $0   [-s] [-p] [-v] destination_dir "
  exit 1
fi



while [ $# -gt 0 ]; do
   case "$1" in
    -s)  skipTar=on;;
    -p)  patchedBuild=on;;
    -v)  verbose=on;
   esac
   tard=${PWD}/$1
   shift
done



if [ "X$verbose" = Xon ] ; then
   set -xv
fi

if [ -f $tard ]; then
   echo "dir exist alreday"
   exit
fi
mkdir $tard

getExternals
getCmssw
getSources
getDataFiles
echo $tard
if [ "X$skipTar" != Xon ] ; then
   makeTar
fi

