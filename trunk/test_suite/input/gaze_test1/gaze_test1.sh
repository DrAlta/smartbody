#!/bin/sh

# initialize parameters, MODIFY HERE
SBMINPUT="-seqpath ../../../../test_suite/input/gaze_test1 -seq default-init.seq"
OPTION_1=IMG
OPTION_2=IMG
OPTION_3=IMG
OPTION_4=IMG
OPTION_5=IMG
NUMOFCOMP=5
THRESHOLD_1=3500
THRESHOLD_2=4000
THRESHOLD_3=4200
THRESHOLD_4=4000
THRESHOLD_5=4200


# hard coded, DO NOT MODIFY
SBMBIN=$1
INPUTDIR=$2
OUTPUTDIR=$3
SBMEXE=$4
BASENAME=`basename $0 .sh`

source ../../common_test.sh

COUNTER=0
RESULT=""
FLAG="SUCCESS"
while [ $COUNTER -lt $NUMOFCOMP ]; do
	let COUNTER=COUNTER+1
	OPTION=$(eval echo \$$"OPTION_$COUNTER")
	THRESHOLD=$(eval echo \$$"THRESHOLD_$COUNTER")
	RESULT=`source ../../common_comp.sh $OPTION $COUNTER $THRESHOLD` 
	if [ $RESULT = "FAILURE" ]; then
		FLAG=FAILURE
	fi

done

if [ $FLAG = "FAILURE" ]; then
	echo FAILURE
else
	echo SUCCESS
fi

	
