
//#include <iostream>

//#include <unistd.h>
//#include <stdio.h>
#include <fstream>
//#include <vector>
//#include "Printer.h"
#include "Screen.h"

std::vector<Printer> PrinterList;

void InitPrinters()
{
	std::ifstream file("Printers.txt");
	std::string line;
	
	while(std::getline(file, line))
	{
		PrinterList.push_back(Printer(line));
	}
}




int main()
{
	Screen screen;
	InitPrinters();
	
	screen.Draw();
	
	
	getch();
	return 0;
}
