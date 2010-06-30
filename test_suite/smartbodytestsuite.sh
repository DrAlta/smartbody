#  Smartbody Test Suite
#  part of Smartbody Test System
#	 Copyright (C) 2010 University of Southern California
#  SmartBody-lib is free software: you can redistribute it and/or
#  modify it under the terms of the Lesser GNU General Public License
#  as published by the Free Software Foundation, version 3 of the
#  license.
#
#  SmartBody-lib is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  Lesser GNU General Public License for more details.
#
#  You should have received a copy of the Lesser GNU General Public
#  License along with SmartBody-lib.  If not, see:
#      http://www.gnu.org/licenses/lgpl-3.0.txt
#
#  CONTRIBUTORS:
#			Ari Shapiro,		ICT
#			Jingqiao Fu,		ICT
#			Yuyu Xu,				ICT

#!/bin/sh

# SmartBody test script


match()
{
KEYWORD=$1
DIRPATH=$2
RESULT=0
TLENGTH=`echo ${#DIRPATH}`
for ((i=$TLENGTH; i>=1; --i)); 
do
	CHAR=`echo ${DIRPATH:$i:1}`;
	if [ "$CHAR" = "/" ] || [ "$CHAR" = "\\" ]; then
		SEARCH=`echo ${DIRPATH:$i+1:$TLENGTH-$i-1}`;
		AFTERSEARCH=`echo -e ${SEARCH/$KEYWORD/" "}`;
		if [ "$AFTERSEARCH" = "$SEARCH" ]; then
			RESULT=0;
		else
			RESULT=1;
		fi;
		break;
	fi;
done
echo $RESULT
return $RESULT
}


bash title.sh

#==================================Read configurations================================
OUTPUTDIR=""
INPUTDIR=""
BASEDIR=""
SBMBIN=""
SBMEXE=""
CONFIGFILE=test.config
echo ""
echo =======================================================;
echo -e "\033[40;32m READING CONFIGURATION SETTINGS......\033[0m"
echo =======================================================;
if [ -f "$CONFIGFILE" ]; then
	while read line
	do
		LENGTH=`echo ${#line}`;
		POS=`expr index $line "="`;
		NAME=`echo ${line:0:$POS-1}`;
		VALUE=`echo ${line:$POS:$LENGTH-$POS}`;
		POS=`expr index $line "$"`;
		
		while read tline
		do
			TLENGTH=`echo ${#tline}`;
			TPOS=`expr index $tline "="`;
			TNAME=`echo ${tline:0:$TPOS-1}`;
			TVALUE=`echo ${tline:$TPOS:$TLENGTH-$TPOS}`;
			
			LASTCHAR=`echo ${tline:$TLENGTH-1:1}`;
			if [ "$LASTCHAR" = "/" ]; then
				TVALUE=`echo ${TVALUE%/}`; 
			fi;
			VALUE=`echo ${VALUE//"$"$TNAME/$TVALUE}`;
		done < $CONFIGFILE;
		
		LASTCHAR=`echo ${line:$LENGTH-1:1}`;
		if [ "$LASTCHAR" = "/" ]; then
			VALUE=`echo ${VALUE%/}`; 
		fi;
		
		if [ "$NAME" = "OUTPUTDIR" ]; then
			OUTPUTDIR=$VALUE;
		elif [ "$NAME" = "INPUTDIR" ]; then
			INPUTDIR=$VALUE;
		elif [ "$NAME" = "SBMBIN" ]; then
			SBMBIN=$VALUE;
		elif [ "$NAME" = "SBMEXE" ]; then
			SBMEXE=$VALUE;
		fi;
		echo -e "    \033[40;33m$NAME\033[0m=$VALUE";
	done < $CONFIGFILE;
else
	echo "ERROR: Can not find $CONFIGFILE, no default settings found"
fi
echo Finished reading configuration settings

echo ""
#=====================================================================================

if [ "$BASEDIR" = "" ]; then
	BASEDIR=`pwd`
fi
if [ "$INPUTDIR" = "" ]; then
	echo "WARNING: Can not find configuration for INPUTDIR, default settings applied"
	INPUTDIR=$BASEDIR/input
fi
if [ "$OUTPUTDIR" = "" ]; then
	echo "WARNING: Can not find configuration for OUTPUTDIR, default settings applied"
	OUTPUTDIR=$BASEDIR/output
fi

MYPATH=""

# make sure the output directory exists
if [ ! -d "$OUTPUTDIR" ]; then
	mkdir $OUTPUTDIR
fi

# create a directory based on the current date/time
DATEDIR=`date +%Y%m%d_%H%M`
TESTDIR=$OUTPUTDIR/$DATEDIR

# make sure that this directory does not already exist
if [ ! -d "$TESTDIR" ]; then
	mkdir $TESTDIR
fi

# create a log file for this test run
echo =======================================================;
echo -e "\033[40;32m SETUP OUTPUT DIRECTORIES AND LOG FILES......\033[0m"
echo =======================================================;
LOGFILE=$TESTDIR/alltests.log
touch "$LOGFILE"
PATHBUFFER="PATHBUFFER"

echo "    Test directory is $INPUTDIR"
echo "    Output directory is $TESTDIR"
echo "    Log file for test summary is: $LOGFILE"
echo -n -e "Starting SmartBody tests in $BASEDIR\n" >> $LOGFILE

# now run each test in INPUTDIR
echo ""
echo =======================================================;
echo -e "\033[40;32m START TESTING......\033[0m"
echo =======================================================;
DIRNUM=0
SEARCH=$1
TLENGTH=`echo ${#SEARCH}`;
for d in $INPUTDIR/*
do
	# make sure that this is a directory
	if [ -d "$d" ]; then
		if [ "$TLENGTH" != "0" ]; then
			RESULT=`match $SEARCH $d`
			if [ "$RESULT" = "0" ]; then
				echo -e "  \033[43;37m==>\033[0m Skipping directory: $d" 
				continue
			fi
		fi
		
		DIRNUM=$[$DIRNUM+1]
		echo ""
		echo "  -----------------------------------------"
		echo -e "  \033[42;37m==>\033[0m Examining directory: $d" 
		TESTNAME=$(basename $d)
		
		# write the current time to the log file

		# create a corresponding test directory in OUTPUTDIR
		TESTRESULTS=$TESTDIR/$TESTNAME
		if [ ! -d "$TESTRESULTS" ]; then
			mkdir $TESTRESULTS
			echo "      Created test output directory $TESTRESULTS"
		fi
		#TESTLOGFILE=$TESTRESULTS/test.log
		cd $INPUTDIR/$TESTNAME
		# find the test file	
		SH_NUM=0
		find . -name "*.sh" -print | (xargs -0 >$PATHBUFFER;
		while read line
		do
		if [ -f "$line" ] || [ "$SH_NUM" = "0" ]; then
			CURRENTDATE=`date`
			echo -n -e "$CURRENTDATE" >> $LOGFILE
			echo -n -e "\t$TESTNAME" >> $LOGFILE
		fi
		if [ -f "$line" ]; then
			SH_NUM=$[$SH_NUM+1];
			TESTLOGFILE=$TESTRESULTS/$line".log";
			bash $line $SBMBIN $INPUTDIR $TESTDIR $SBMEXE &>$TESTLOGFILE;
			
			# check for test success
			if [ -f "$TESTLOGFILE" ]; then

				RESULT=`tail --lines=1 $TESTLOGFILE`
				#echo "        Test result: $RESULT"
				if [ "$RESULT" = "SUCCESS" ] || [ "$RESULT" = "> SUCCESS" ]; then
					echo -n -e "\t\t\t\t\t\t\t\t$line\t\tSUCCESS\n" >> $LOGFILE;
				else
					echo -n -e "\t\t\t\t\t\t\t\t$line\t\tFAILURE\n" >> $LOGFILE;
				fi;
			fi;
			if [ "$RESULT" = "SUCCESS" ] || [ "$RESULT" = "> SUCCESS" ]; then
				echo -e "        Testing: $line      \033[42;37mSUCCESS\033[0m";
			else
				echo -e "        Testing: $line      \033[41;37mFAILURE\033[0m";
			fi
		else
			if [ "$SH_NUM" = "0" ] ; then
				echo -n -e "\t\t\t\t\t\t\t\tNOTEST\n" >> $LOGFILE;
				echo -e "        \033[43;37mNOTEST\033[0m";
			fi;
		fi;
		done < $PATHBUFFER;
		rm $PATHBUFFER;
		)
	fi
	cd $INPUTDIR
done
echo ""
echo =======================================================;
echo -e "\033[40;34m FINISHED TESTING\033[0m"
echo =======================================================;
echo ""

# now examine the test results and summarize
#TOTALLINES=`wc -l $LOGFILE | cut -f1 -d" "`
#TOTALNUMTESTS=$(($TOTALLINES - 1))

TOTALNUMSUCCESS=`grep SUCCESS $LOGFILE | wc -l`
TOTALNUMFAIL=`grep FAILURE $LOGFILE | wc -l`
TOTALNUMTESTS=$(($TOTALNUMSUCCESS + $TOTALNUMFAIL))
TOTALNUMMISSING=`grep NOTEST $LOGFILE | wc -l`

echo "Total directories found: $DIRNUM"
echo "Total tests run: $TOTALNUMTESTS"
if [ $TOTALNUMSUCCESS -eq 0 ]; then
	echo -e "Total success: \033[40;37m$TOTALNUMSUCCESS\033[0m"
else 
	echo -e "Total success: \033[40;32m$TOTALNUMSUCCESS\033[0m"
fi
if [ $TOTALNUMFAIL -eq 0 ]; then
	echo -e "Total failure: \033[40;37m$TOTALNUMFAIL\033[0m"
else 
	echo -e "Total failure: \033[41;37m$TOTALNUMFAIL\033[0m"
fi

if [ $TOTALNUMMISSING -eq 0 ]; then
	echo -e "Total missing: \033[40;37m$TOTALNUMMISSING\033[0m"
else
	echo -e "Total missing: \033[43;37m$TOTALNUMMISSING\033[0m"
fi

echo -n -e "\n\nTotal directories found: $DIRNUM\n" >> $LOGFILE;
echo -n -e "Total tests run: $TOTALNUMTESTS\n" >> $LOGFILE;
echo -n -e "Total success: $TOTALNUMSUCCESS\n" >> $LOGFILE;
echo -n -e "Total failure: $TOTALNUMFAIL\n" >> $LOGFILE;
echo -n -e "Total missing: $TOTALNUMMISSING\n" >> $LOGFILE;

