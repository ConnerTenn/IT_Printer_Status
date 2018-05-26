
GPP=g++ -std=c++17 -Wall -lncurses -lcurl
D=Build

all: $D/run.exe

$D/run.exe: $D/main.o $D/Screen.o
	$(GPP) $D/main.o $D/Screen.o -o $D/run.exe

$D/Screen.o: Screen.cpp Screen.h
	$(GPP) Screen.cpp -c -o $D/Screen.o

$D/main.o: main.cpp Printer.h
	$(GPP) main.cpp -c -o $D/main.o
	
run: all
	$D/run.exe
	
clean:
	rm $D/* -f
	
force: clean all

