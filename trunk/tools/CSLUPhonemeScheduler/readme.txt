CSLU Phoneme Scheduler

/fa and /Tcl80 folders are not included by default due to license. Please download cslu toolkit at http://www.cslu.ogi.edu/toolkit/. 
Copy /fa from CSLU_ROOT/Toolkit/2.0/apps/fa and /Tcl80 from CSLU_ROOT/Tcl80.

-------------------------------------------------File Layout--------------------------------------------------------------------

/fa:		script for generating phonemes and doing force alignment. (two scripts are used)
		gen_pronun.tcl: generate list of phonemes in .ntp file, parameters: <.txt> <.ntp> [-dict curstom_dictionary.txt]
		fa_new_multichan.tcl: generate phoneme time schedules in .txt1 file, parameters: <.wave> <.ntp> <.txt1>. Note that wav file has to be mono track.

/files:		put .wav, .txt files here they will be used to generate .txt1 files.
		converter will automatically look for all .txt1 files in this folder and generate .bml files.


/Tcl80:		where tcl80sh.exe sits

mapping.txt:	where you can find the mapping from worldbet symbols to timit symbols to diphone phonemes used for converter. It has to be a four column text file.

auto.sh:	automatic script that generate .txt1 files given .wav and .txt files under "files" folder and convert .txt1 to .bml files






Some useful links:
http://web.mit.edu/course/6/6.863/share/nltk_lite/timit/
http://www.cslu.ogi.edu/toolkit/
http://www.ncbi.nlm.nih.gov/pmc/articles/PMC2682710/