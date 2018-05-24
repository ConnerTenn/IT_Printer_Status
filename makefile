
GPP=g++ -std=c++17 -Wall
D=Build

all: $D/run.exe

$D/run.exe: $D/main.o
	$(GPP) $D/main.o -o $D/run.exe

$D/main.o: main.cpp
	$(GPP) main.cpp -c -o $D/main.o
	
run: all
	$D/run.exe
	
clean:
	rm $D/* -f
	
force: clean all

