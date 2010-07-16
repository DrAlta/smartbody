#!/bin/bash
# script to send simple email
# email subject
SUBJECT="[Smartbody] Test Results "$CURRENTDATE
echo $SUBJECT
# Email To ?
MAIL_LIST=testMailList.txt
#EMAIL="jingqiaf@usc.edu, ghostblade1982@hotmail.com"
FILE1=$1"/../../"$MAIL_LIST
ALLTESTIFLE=$1"/alltests.log"
echo $FILE1
while read line
do
	if [ "$EMAIL" = "" ]; then
		EMAIL=$line;
	else
		EMAIL=$EMAIL", "$line;
	fi;
	echo $EMAIL;
done<$FILE1;
# Email text/message
echo $EMAIL
echo $ALLTESTIFLE

EMAILMESSAGE=""
while read line
do
	if [ "$EMAILMESSAGE" = "" ]; then
		EMAILMESSAGE=$line;
	else
		EMAILMESSAGE=$EMAILMESSAGE"\n"$line;
	fi;
	echo $EMAILMESSAGE;
done<$ALLTESTIFLE;
# send an email exim
echo -e 'Subject: '$SUBJECT'\nresult\n' $EMAILMESSAGE | exim -v -odf $EMAIL