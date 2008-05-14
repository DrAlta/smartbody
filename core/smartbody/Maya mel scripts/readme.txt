================================================
 THE SMARTBODY & MOTION ENGINE MAYA MEL SCRIPTS
================================================

==== USING MEL SCRIPTS ====
To use a .mel script from a file, you use Maya's "source <file>" 
directive from a shelf script or within the script editor.
<file> can either be a full path name or a file within the paths
defined by the MAYA_SCRIPT_PATH environment variable.

When working from an active repository, we highly recommend
you modify this variable to include this directory of you SVN
working copy.  This way, scripts can be accessed by their basename:
   source "export_skm";
You will also have access to the latest revision and new scripts.

Alternatively, you can use the full file name:
   source "C:\saso\core\smartbody\Maya mel scripts\export_skm.mel";

To start a script from the shelf, type the source command into the
Script Editor (without running it).  Select the command, either
via Ctrl-A (Cmd-A on Mac) or using your mouse.  In the Shelf Editor
menu, select File > Save Selected to Shelf...  The first few
characters of the name will be added to the button icon, so choose
wisely.


==== EXPORTING SKELETONS ====

Three steps are required for exporting a new
skeleton/character from Maya:
 - First, use the sk exporter mel script to export a .sk
   skeleton definition file
 - Second, use the obj exporter mel script to export any
   used rigid models. Make sure the exported .objs are saved
   to the same directory of the .sk file
 - Finally, the geometries need to be converted to local
   coordinates. There are 2 ways for doing that:
   From inside metool: select Tools->Convert Skeleton.
   From inside skviewer: My Tab->Geometries To Local and then Save.
   A new correct skeleton (with srm geometries) will be created.

IMPORTANT NOTES:
 - If "Mirror Joint" is used to create the skeleton
   in Maya, be sure to use the "orientation" option (not "behavior").
 - If you get the error "More than one object matches name: xxx";
   you must rename the conflicting objects until all errors disappear.

SK EXPORTER VERSIONS
   02 - rounding 5th decimal
   01 - first version of the SK exporter

OBJ EXPORTER VERSIONS
   01 - first version of the OBJ exporter

==== EXPORTING MOTIONS ====

 - The order of the joint rotations is considered to be Euler XYZ
 - You cannot change the default name of the animation curves 
   (they have to have the form jointName_rotateX, etc)
 - If the animation uses IK handles, it must be baked before exporting
 - To make sure the animation export will be fine, remove all IK handles
   and check if the animation in Maya is still correct
 - Although the script checks it, change the settings to cms

 SKM EXPORTER VERSION HISTORY
   04 - corrected recursing into joint hierarchy even if joint has no keys
   03 - rounding 5th decimal
   02 - smaller files exported (less decimals used)
   01 - first version

