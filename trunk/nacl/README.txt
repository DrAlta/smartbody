12/1/11

Instructions to build a SmartBody application for Native Client
---------------------------------------------------------

The SmartBody code is cross-compiled for the Native Client platform using the Native Client SDK(NACL_SDK), which allows you to use gcc-like tools to build Native Client applications.

(1) Download and install native-sdk from : https://developers.google.com/native-client/sdk/download.hmtl

(Note that when building on the windows platform, you will also need to install cygwin 1.7 or higher from http://www.cygwin.com/ )

Follow the installation instruction to enable Chrome to support Native Client and set the Enviromemnt varaible NACL_SDK_ROOT= ..\nacl_sdk\pepper_xx or which ever version of pepper you are using. 

You also need to export the environment variable NACL_SDK_ROOT to the NACL SDK installation directory ( for example, export NACL_SDK_ROOT= "/path/to/nacl_sdk/pepper" )

i)To build the following Libraries:

bonebus
vhcl
tinyxml
pprAI
steerlib
vhmsg
wsp
smartbody-lib
smartbody-dll

Go to Smartbody/nacl directorty in the command prompt and enter "make"

ii) Building ode

Dowonload the ode (http://www.ode.org/download.html) and extract in the smartbody/nacl Directory:
Add /path/to/nacl_sdk/pepper_xx/toolchain/win_x86_glibc/bin to PATH in Cygwin. In the ode directory from Cygwin, enter:

./configure --host=i686-nacl CC=i686-nacl-gcc CXX=i686-nacl-g++ CROSS_COMPILE=yes

