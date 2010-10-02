#!/bin/sh
exitFlag=0
if [ $1 = "edit" ]; then
  while [ $# -gt 0 ]
  do
    shift
  done
fi
numInput=$#

while [ $exitFlag = 0 ]
do
  echo -e -n "\E[2J"
  echo -e "\e[44;37m	Smartbody Testing Menu		\e[0m"
  echo ""
  echo "0. Update Test Cases"
  echo "1. Create New Test Cases"
  echo "2. Run All the Test Cases"
  echo "3. Delete Certain Test Cases"
  echo "4. List All Test Cases"
  echo "5. Exit"
  echo -n "Your Option: "
  if [ $# = 0 ]; then
    read -n 1 Operation
  else
    Operation=$1
    echo $Operation
    shift
  fi
  echo ""
  echo ""

  cd $SRCDIR
  case "$Operation" in
    "0") echo -n "Enter cases' name to update(seperate by space): "
         if [ $numInput = 0 ]; then
           read updateCases
         else
           updateCases="$@"
           echo $updateCases
         fi
         source testCreateCases.sh update $updateCases         
         ;;
    "1") echo -n "Enter seq files' path(seperate by space): "
         if [ $numInput = 0 ]; then    
           read createCases
         else
           createCases="$@"
           echo $createCases
         fi
         source testCreateCases.sh create $createCases
         ;;
    "2") echo -n "Enter cases' name to be run: "
         if [ $numInput = 0 ]; then     
           read runCases
         else
           runCases="$@"
           echo $runCases
         fi
         source testRunCases.sh "$runCases"
         ;;
    "3") echo -n "Enter cases' name to be deleted(seperate by space): "
         if [ $numInput = 0 ]; then       
           read deleteCases
         else
           deleteCases="$@"
           echo $deleteCases
         fi
         source testDeleteCases.sh $deleteCases
         ;;
    "4") source testListCases.sh 
         ;;
    "5") exitFlag=1
         ;;
    *)   echo "Invalid Operation. Please Re-enter..."
         ;;
  esac
  
  if [ $numInput = 0 ]; then
    echo "Press Any Key to Continue..."
    read -n 1 -s
  else
    exitFlag=1
  fi
done
echo -e -n "\E[2J"