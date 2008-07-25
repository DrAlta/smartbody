CerevoiceRelay looks for:

 * cerevoice_2.0.0_katherine_00009.lic
 * cerevoice_2.0.0_katherine_00009.voice

In the relative directory "..\..\data\cereproc\voices" (should correspond to SASO's data directory).

It looks for:

 * abb.txt
 * pbreak.txt
 * homographrules.dat
 * reductionrules.dat
 * normrules.py

In the relative directory "..\..\data\cereproc\cerevoice" (again, should correspond to SASO's data directory).

These files are not on the SVN and must be copied to the correct location by the user.

The generated audio files are saved in "%SASO_ROOT%\dimr\tmpaudio".  If %SASO_ROOT% is not defined, they are saved in the relative directory "..\..\..\dimr\tmpaudio".  It uses the file name given to it by SmartBody, with the exception that it replaces the ".aiff" extension with ".wav" since it is outputing a .wav audio file.