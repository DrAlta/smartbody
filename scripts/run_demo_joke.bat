echo off
echo
echo
echo ########################################
echo # run-demo-joke.bat
echo # 
echo # Runs SBM, initialized for the
echo # joke-telling demonstration,
echo # first presented at AAMAS 2008.
echo # Requires RemoteSpeech module
echo # (timed for use with RvoiceRelay)
echo # 
echo # When the prompt and viewer are ready,
echo # type:
echo #    seq demo-joke
echo ########################################

set SBM_PATH=..\core\smartbody\sbm\bin
set SBM_EXE=sbm-fltkd.exe
set SBM_DATA=..\..\..\..\data\sbm-demo-joke\scripts
set SBM_SEQ=demo-joke-init.seq
set SBM_ARGS=-seqpath %SBM_DATA% -seq %SBM_SEQ% -host=localhost -audio -fps=60

pushd %SBM_PATH%
echo %SBM_EXE% %SBM_ARGS%
%SBM_EXE% %SBM_ARGS%

popd
echo on
