@echo on

bcc64 -c  -tW prozessinfo.cpp
brc32 -r prozessinfo.rc
ilink64 /C /aa /Gn  prozessinfo.o c0w64.o ,prozessinfo.exe,prozessinfo.map,import64.a cw64mt.a,,prozessinfo.res 
