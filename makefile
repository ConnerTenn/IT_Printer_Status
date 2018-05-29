
F=-D LINUX $(if $(filter debug,$(MAKECMDGOALS)),-D DEBUG -ggdb, )
GPP=g++ -std=c++17 -Wall -lncurses -lcurl -lpthread $F
D=Build

all: $D/run.exe

$D/run.exe: $D/main.o $D/Screen.o $D/Printer.o
	$(GPP) $D/main.o $D/Screen.o $D/Printer.o -o $D/run.exe

$D/main.o: main.cpp Printer.h
	$(GPP) main.cpp -c -o $D/main.o
	
$D/Screen.o: Screen.cpp Screen.h
	$(GPP) Screen.cpp -c -o $D/Screen.o

$D/Printer.o: Printer.cpp Printer.h
	$(GPP) Printer.cpp -c -o $D/Printer.o

run: all
	$D/run.exe
	
clean:
	rm $D/* -f
	
force: clean all

debug:
	


#windows:
#	cl main.cpp Screen.cpp Printer.cpp /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /sdl /Fd"Release\vc141.pdb" /Zc:inline /fp:precise /D "WINDOWS" /D "_CRT_SECURE_NO_WARNINGS" /D "_MBCS" /WX- /Zc:forScope /Gd /Oy- /Oi /MD /Fa"Release\" /EHsc /nologo /Fo"Release\" /Fp"Release\IT_Printer_Status.pch"    /OUT:"C:\Sync\Git\IT_Printer_Status\Release\IT_Printer_Status.exe" /MANIFEST /LTCG:incremental /NXCOMPAT /PDB:"C:\Sync\Git\IT_Printer_Status\Release\IT_Printer_Status.pdb" /DYNAMICBASE "pdcurses.lib" "libcurl.a" "libcurl.dll.a" "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /DEBUG:FULL /MACHINE:X86 /OPT:REF /SAFESEH /PGD:"C:\Sync\Git\IT_Printer_Status\Release\IT_Printer_Status.pgd"  /I C:\Sync\Programming Libraries\curl-win32\include /I C:\Sync\Programming Libraries\PDCurses\PDCurses-master
