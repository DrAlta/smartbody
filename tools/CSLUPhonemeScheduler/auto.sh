cd fa
ls ../files/*.txt | while read line; do
	filename=$(basename "$line")
	echo $filename
	filename="${filename%.*}"
	../Tcl80/bin/tclsh80.exe gen_pronun.tcl ../files/$filename.txt ${filename}_automatic.ntp
	../Tcl80/bin/tclsh80.exe fa_new_multichan.tcl ../files/$filename.wav ${filename}_automatic.ntp ../files/$filename.txt1
done
cd ..
cd converter
./converter ../files ../mapping.txt
cd ..