#!bin/sh
# $1=OPTION, $2=INDEX, $3=THRESHOLD

# variables
SNAPSHOT="snapshot_$2.ppm"
PIC="${BASENAME}_$2.ppm"
STDPIC="${BASENAME}_std_$2.ppm"
STAT="${BASENAME}_stat.txt"
TEMPSTAT="stat.txt"
DIFFPIC="${BASENAME}_diff_$2.jpg"

# operations
if [ $1 = "IMG" ]; then
	cd $SBMBIN
	convert $SNAPSHOT $PIC
	rm $SNAPSHOT
	mv $PIC $OUTPUTDIR/$BASENAME
	WININPUTDIR=`cygpath -m $INPUTDIR`
	WINOUTPUTDIR=`cygpath -m $OUTPUTDIR`
	compare -metric MAE $WINOUTPUTDIR/$BASENAME/$PIC $WININPUTDIR/$BASENAME/$STDPIC $WINOUTPUTDIR/$BASENAME/$DIFFPIC &> $WINOUTPUTDIR/$BASENAME/$TEMPSTAT
	cd $OUTPUTDIR/$BASENAME
	cat $TEMPSTAT >> $STAT
	read var1 var2 < $TEMPSTAT
	rm $TEMPSTAT
	int_var1=${var1/.*}
	if [ $int_var1 -le $3 ]; then
		echo "SUCCESS" >> $STAT
		echo SUCCESS
	# redo functionality removed 9/26/11 by AS
	#elif [ $int_var1 -ge 10000 ]; then
	#	echo "REDO" >> $STAT
	#	echo REDO
	else
		echo "FAILURE" >> $STAT
		echo FAILURE
	fi	
fi


