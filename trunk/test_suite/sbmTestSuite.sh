#!/bin/sh


echo -e -n "\E[2J"
source ./src/testTitle.sh
# ---------------- Read Configuration
configFlag=1
CONFIGFILE=./src/testConfig.sh
echo ""
echo =======================================================;
echo -e "\033[40;32m READING CONFIGURATION SETTINGS......\033[0m"
echo =======================================================;
if [ -f "$CONFIGFILE" ]; then
  source $CONFIGFILE
  echo "BASEDIR:	$BASEDIR"
  echo "SRCDIR:		$SRCDIR"
  echo "OUTPUTDIR:	$OUTPUTDIR"
  echo "INPUTDIR:	$INPUTDIR"
  cd $SBMBIN
  echo "SBMBIN:		"`pwd`
  cd $BASEDIR
  echo "SBMEXE:		$SBMEXE"  
  echo "Finish reading configuration settings..."  
else
	echo "ERROR: Can not find $CONFIGFILE"
	configFlag=0
fi
echo ""

if [ "$configFlag" = "1" ]; then
  if [ $1 = "-edit" ]; then
    source ./src/testMenu.sh 
  elif [ $1 = "-update" ]; then
    shift
    source ./src/testCreateCases.sh -update $@
  elif [ $1 = "-create" ]; then
    shift
    source ./src/testCreateCases.sh -create $@
  elif [ $1 = "-print" ]; then
    source ./src/testListCases.sh
  elif [ $1 = "-delete" ]; then
    @shift
    source testDeleteCases.sh $@
  else
    source ./src/testRunCases.sh $1
  fi
fi