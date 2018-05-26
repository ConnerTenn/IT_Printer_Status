
//#include <iostream>

#include <unistd.h>
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
	
	bool run = true;
	
	while (run == true)
	{
		
		screen.Draw();
		
		
		int key = getch();
		if (key == 27)
		{
			run = false;
		}
		else if (key == KEY_UP)
		{
			screen.Cursor = (screen.Cursor < 1 ? 0 : screen.Cursor - 1);
		}
		else if (key == KEY_DOWN)
		{
			screen.Cursor = (screen.Cursor >= (int)PrinterList.size() - 1 ? PrinterList.size() - 1 : screen.Cursor + 1);
		}
	}
	return 0;
}
