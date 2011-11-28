Usage of SmartBody Testing System

* Usage
./sbtest.sh -update <casename1> <casename2> <casename3>...
	Update certain cases, e.g. gaze_test1 gaze_test2
	If no casename is specified, all the case would be updated
	
./sbtest.sh -create <path of seqfile1> <path of seqfile2>...
	Create new cases
	<path of seqfile> example:	e:/test.seq f:/smartbody/test1.seq, .seq suffix is necessary. If no directory is specified, default is smartbody/test_suite
	
./sbtest.sh <case-name>
	Run cases
	Test suite would search for all the cases including the input string. If no input, all the cases would be run

./sbtest.sh -delete <casename1> <casename2> <casename3>...
	Delete certain cases

./sbtest.sh -print	
	List All cases

./sbtest.sh -edit	
	Go to the main menu

* Known issues
- For now, if you have seperate data for your test case (especially for the deformable geometry). You have to create your own .seq file first, using -update to generate .ppms, then manually create .sh file. This has to be improved.