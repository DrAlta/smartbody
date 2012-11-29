#!/bin/sh

# first input parameter is 
echo "Mode: $1"
echo "Starting $1..."
processMode=$1

while [ $# -gt 1 ]
do
  cd $BASEDIR
  if [ $processMode = "-create" ]; then
	  if [ -e "$2" ]; then
	    echo "*  $2 found!"
	    caseName=`basename $2`
	    folderName=`basename $2 .py`
	    if [ $folderName = $caseName ]; then
	      echo "*  Suffix not correct, skip"
	      shift
	      continue
	    fi	    
	    cp $2 $SBMBIN
	    cd $INPUTDIR
	    if [ -d $folderName ]; then
	      echo "* Error: test case already exist, unable to create"
	      shift
	      continue
	    fi
	    mkdir "$folderName"
	    if [ $2 = `basename $2` ]; then
	      cd $BASEDIR
	    fi 
	    cp $2 "$INPUTDIR/$folderName" 
	    cd $INPUTDIR/$folderName
	  else
	    echo "*  $2 not exist, skip!"
	    shift
	    continue
	  fi     
  fi
  
  if [ $processMode = "-update" ]; then
    folderName=$2
    cd $INPUTDIR
    if [ ! -d "$2" ]; then 
      echo "Test case not exist, skip!"
      shift
      continue
    fi
    cd $folderName
    caseName="${folderName}.py"
    cp $caseName $SBMBIN
  fi
  
  cd $SBMBIN
  ./$SBMEXE -noninteractive -script $caseName
	
	imgNum=0  
  for image in *.ppm
  do
    (( imgNum ++ ))
    newimg=${image/#*_/${folderName}_std_}
    echo $newimg
    mv $image $newimg
    mv $newimg $INPUTDIR/$folderName
  done
  rm $caseName  
  
  if [ "$processMode" = "-create" ]; then
    cd $INPUTDIR/$folderName
    scriptName="${folderName}.sh"
    touch $scriptName
    echo "#!/bin/sh" >> $scriptName
    winInputDir=`cygpath -m $INPUTDIR`
    echo "SBMINPUT=\"-scriptpath ../../../../test_suite/input/$folderName -script $folderName.py\"" >> $scriptName  
    imgCounter=0
    while [ $imgCounter -lt $imgNum ]; do
      (( imgCounter ++ ))
      echo "OPTION_${imgCounter}=IMG" >> $scriptName
      echo "THRESHOLD_${imgCounter}=3000" >> $scriptName
    done
    echo "NUMOFCOMP=$imgNum" >> $scriptName
    echo "" >> $scriptName
    cat $SRCDIR/testTemplate.sh >> $scriptName
  fi
  shift
done