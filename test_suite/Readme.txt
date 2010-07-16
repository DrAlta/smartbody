Usage of SBM Testing System

* Main Menu:
0. Update certain cases
1. Create new cases
2. Run certain cases
3. Delete certain cases
4. List All cases
5. Exit

* Run sbmTestSuite.sh
  If no input parameter, system would go to main menu, run in NORMAL MODE
  
  If there are input parameters, they should be in the following format
    ./sbmTestSuite.sh 1 test.seq
  first parameter indicate the operation, all the left is the parameters for that particular operation
  This is run in BATCH MODE

* How to use different functions
  0. Input parameters' format: <casename1> <casename2> <casename3>...
                          e.g. gaze_test1 gaze_test2
                          p.s. It would search for the input directoy
  1. Input parameters' format: <path of seqfile1> <path of seqfile2> ... (seperate by space)
                          e.g. e:/test.seq f:/smartbody/test1.seq
                          p.s. If .seq suffix is necessary; If no directory is specified, default is smartbody/test_suite
  2. Input parameters' format: <test name>
                          e.g. gaze_test
                          p.s. It would search for the case that contains the input substring. This means if input gaze_test, it would run gaze_test, gaze_test1, gaze_test2.
                               If no input, then run all the tests
  3. Input parameters' format: <casename1> <casename2> <casename3>...
                          e.g. gaze_test1 gaze_test2
                          p.s. It would search for the input directoy and delete these cases
  4. Input parameters' format: N/A
  5. Input parameters' format: N/A
  