#!/bin/sh
#SBMINPUT="-noninteractive -seqpath ../../../../test_suite/input/headnod_test -seq headnod_test.seq"
SBMINPUT="-noninteractive -scriptpath ../../../../test_suite/input/headnod_test -script headnod_test.py"
OPTION_1=IMG
THRESHOLD_1=500
OPTION_2=IMG
THRESHOLD_2=500
OPTION_3=IMG
THRESHOLD_3=500
OPTION_4=IMG
THRESHOLD_4=500
OPTION_5=IMG
THRESHOLD_5=500
OPTION_6=IMG
THRESHOLD_6=500
NUMOFCOMP=6

SBMBIN=$1
INPUTDIR=$2
OUTPUTDIR=$3
SBMEXE=$4
BASENAME=`basename $0 .sh`

cd $INPUTDIR
source ../src/common_test.sh

COUNTER=0
RESULT=""
FLAG="SUCCESS"
REDOFLAG="NOREDO"
while [ $COUNTER -lt $NUMOFCOMP ]; do
	let COUNTER=COUNTER+1
	OPTION=$(eval echo \$$"OPTION_$COUNTER")
	THRESHOLD=$(eval echo \$$"THRESHOLD_$COUNTER")
	cd $INPUTDIR
	RESULT=`source ../src/common_comp.sh $OPTION $COUNTER $THRESHOLD` 
	if [ $RESULT = "FAILURE" ]; then
		FLAG=FAILURE
	fi
	if [ $RESULT = "REDO" ]; then
		REDOFLAG=REDO
	fi

done

if [ $FLAG = "FAILURE" ]; then
	echo FAILURE
else
	echo SUCCESS
fi
if [ $REDOFLAG = "REDO" ]; then
	echo REDO
fi