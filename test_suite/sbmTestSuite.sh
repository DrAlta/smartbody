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

if [ $# -eq 0 ]; then
  echo -n "Press any key to continue..."
  read -n 1 -s
fi

if [ "$configFlag" = "1" ]; then
  source ./src/testMenu.sh $@
fi 

