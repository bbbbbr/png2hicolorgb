rgbasm -rd -ocolor.obj color.z80
IF ERRORLEVEL GT 0 goto :end
Xlink -mmap -ncolor.sym color.lnk 
rgbfix -p -v color.GB
rem emu -v -b -p 2 -f color.gb
:end

