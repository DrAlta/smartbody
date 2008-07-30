CerevoiceRelay looks for:

 * cerevoice_2.0.0_katherine_00009.lic
 * cerevoice_2.0.0_katherine_00009.voice

In the relative directory "..\..\data\cereproc\voices" (should correspond to SASO's data directory).  These files are not on the SVN and must be copied to the correct location by the user.

It also requires:

 * "../../lib/cerevoice/veng_db/en/norm/abb.txt"
 * "../../lib/cerevoice/veng_db/en/norm/pbreak.txt"
 * "../../lib/cerevoice/veng_db/en/homographs/rules.dat";
 * "../../lib/cerevoice/veng_db/en/reduction/rules.dat"
 * "../../lib/cerevoice/veng_db/en/gb/norm/rules.py"

These files are located on the SVN.

The generated audio files are saved in "%SASO_ROOT%\dimr\tmpaudio".  If %SASO_ROOT% is not defined, they are saved in the relative directory "..\..\..\dimr\tmpaudio".  It uses the file name given to it by SmartBody, with the exception that it replaces the ".aiff" extension with ".wav" since it is outputting a .wav audio file.
