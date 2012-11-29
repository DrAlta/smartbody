os=`uname`
export BASEDIR=`pwd`
export MEDIAPATH=$BASEDIR/../data
export SRCDIR=$BASEDIR/src
export SBMBIN=$BASEDIR/../core/smartbody/sbm/bin
export OUTPUTDIR=$BASEDIR/output
export INPUTDIR=$BASEDIR/input
if [ $os = "Linux" ]; then
export SBMEXE=sbm-fltk
else
#export SBMEXE=sbm-fltkd.exe
export SBMEXE=sbm-fltk.exe
fi
