Usage of SBM Testing System

* Main Menu:
0. Update certain cases
1. Create new cases
2. Run certain cases
3. Delete certain cases
4. List All cases
5. Exit

* Run sbmTestSuite.sh
  If no input parameter, system would go to menu 2, running all the tests exist
  If there is input parameter, system would go to main menu only if the input parameter is "edit"
  
//  This is for the old version - replaced by above 2010-10-01 Yuyu Xu
//  If there are input parameters, they should be in the following format
//    ./sbmTestSuite.sh 1 test.seq
//  first parameter indicate the operation, all the left is the parameters for that particular operation
//  This is run in BATCH MODE

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
  
* Known Issues:
- For now, if you have seperate data for your test case (especially for the deformable geometry). You have to run the test first. After the test folder is created, go there and create a data folder and put your data there. This means the seq file before test is created and the seq file after would be different because the data path is changed. This is a large amount of manual work. Has to be improved later.