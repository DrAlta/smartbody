echo off
echo
echo
echo ########################################
echo # run-aamas2008-demo.bat
echo # 
echo # Runs SBM, initialized for the
echo # AAMAS 2008 demo sequence.
echo # Requires RemoteSpeech for module
echo # (timed for use with RvoiceRelay)
echo # 
echo # When the prompt and viewer are ready,
echo # type:
echo #    seq demo-aamas2008
echo ########################################

set SBM_PATH=..\core\smartbody\sbm\bin
set SBM_EXE=sbm-fltkd.exe
set SBM_DATA=..\..\..\..\data\sbm-testdata
set SBM_SEQ=demo-aamas2008-init.seq
set SBM_ARGS=-seqpath %SBM_DATA% -seq %SBM_SEQ% SBM_ARGS=-host=localhost -audio -fps=60

pushd %SBM_PATH%
echo %SBM_EXE% %SBM_ARGS%
%SBM_EXE% %SBM_ARGS%

popd
echo on
