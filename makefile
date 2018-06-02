

#Please ensure you have ncurses and curl installed before you compile this project

F=-D LINUX# $(if $(filter debug,$(MAKECMDGOALS)),-D DEBUG -ggdb, )
T=

GPP=g++ -std=c++17 -Wall -lncurses -lcurl -lpthread $F $T
D=Build

all: dir $D/run.exe

dir:
	mkdir -p $D

$D/run.exe: dir $D/main.o $D/Screen.o $D/Printer.o
	$(GPP) $D/main.o $D/Screen.o $D/Printer.o -o $D/run.exe

$D/main.o: dir main.cpp Printer.h
	$(GPP) main.cpp -c -o $D/main.o
	
$D/Screen.o: dir Screen.cpp Screen.h
	$(GPP) Screen.cpp -c -o $D/Screen.o

$D/Printer.o: dir Printer.cpp Printer.h
	$(GPP) Printer.cpp -c -o $D/Printer.o

run: all
	$D/run.exe
	
clean:
	rm $D/* -f
	
force: clean all

debug: T=$F -D DEBUG -ggdb
debug: force

