###  This file has be deprecated in favor of init-common-assets.seq
###  See SVN r444 comment.

0 echo == Beginning: data/sbm-common/scripts/common-init.seq

###  Assumes current directory is: core/smartbody/sbm/bin
0	path ME ../../../../data/sbm-common/common-sk

0	echo >>> Loading common motions and poses...
0	load motions -R ../../../../data/sbm-common/common-sk
0	load skeletons -R ../../../../data/sbm-common/common-sk
0	load poses   -R ../../../../data/sbm-common/common-sk

0 echo == Completed: data/sbm-common/scripts/common-init.seq
