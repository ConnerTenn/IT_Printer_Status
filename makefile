

#Please ensure you have ncurses and curl installed before you compile this project

F=-D LINUX# $(if $(filter debug,$(MAKECMDGOALS)),-D DEBUG -ggdb, )
T=

GPP=g++ -std=c++17 -Wall -lncurses -lcurl -lpthread $F $T
D=Build

all: dir $D/run.exe

dir:
	mkdir -p $D

$D/run.exe: dir $D/main.o $D/Screen.o $D/ScreenDrawing.o $D/PrinterClass.o $D/PrinterFunctions.o
	$(GPP) $D/main.o $D/Screen.o $D/ScreenDrawing.o $D/PrinterClass.o $D/PrinterFunctions.o -o $D/run.exe

$D/main.o: dir main.cpp Printer.h
	$(GPP) main.cpp -c -o $D/main.o
	
$D/Screen.o: dir Screen.cpp Screen.h
	$(GPP) Screen.cpp -c -o $D/Screen.o

$D/ScreenDrawing.o: dir ScreenDrawing.cpp Screen.h
	$(GPP) ScreenDrawing.cpp -c -o $D/ScreenDrawing.o

$D/PrinterClass.o: dir PrinterClass.cpp Printer.h
	$(GPP) PrinterClass.cpp -c -o $D/PrinterClass.o
	
$D/PrinterFunctions.o: dir PrinterFunctions.cpp Printer.h
	$(GPP) PrinterFunctions.cpp -c -o $D/PrinterFunctions.o

run: all
	$D/run.exe
	
clean:
	rm $D/* -f
	
force: clean all

debug: T=$F -D DEBUG -ggdb
debug: force

