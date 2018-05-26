
GPP=g++ -std=c++17 -Wall -lncurses -lcurl
D=Build

all: $D/run.exe

$D/run.exe: $D/main.o
	$(GPP) $D/main.o -o $D/run.exe

$D/main.o: main.cpp Printer.h Screen.h
	$(GPP) main.cpp -c -o $D/main.o
	
run: all
	$D/run.exe
	
clean:
	rm $D/* -f
	
force: clean all

