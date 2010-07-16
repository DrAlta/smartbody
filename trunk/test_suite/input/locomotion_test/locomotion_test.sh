#!/bin/sh
SBMINPUT="-seqpath E:/smartbody/test_suite/input/locomotion_test -seq locomotion_test.seq"
OPTION_1=IMG
THRESHOLD_1=3000
OPTION_2=IMG
THRESHOLD_2=3000
OPTION_3=IMG
THRESHOLD_3=3000
NUMOFCOMP=3

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