
//#include <iostream>
#ifdef WINDOWS
#include <windows.h>
#undef MOUSE_MOVED
#elif LINUX
#include <unistd.h>
#endif
#include <time.h>
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
	
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		PrinterList[i].Mutex = new std::mutex;
	}
}

bool Run = true;
#ifdef LINUX
std::mutex PrinterLock;
time_t Timer = 0;
#endif

void UpdatePrinters()
{
	while (Run)
	{
#ifdef WINDOWS
		Sleep(500);
#elif LINUX
		sleep(1);
		PrinterLock.lock();
#endif
		
		for (int i = 0; Run && i < (int)PrinterList.size(); i++)
		{
			PrinterList[i].Update();
		}
#ifdef LINUX
		PrinterLock.unlock();
#endif
	}
}


int main()
{
	InitPrinters();
	Screen screen; screen.Draw();
	
	std::thread printerThread(UpdatePrinters);

#ifdef LINUX
	Timer = 1;
#endif

	while (Run == true)
	{
		//screen.Cursor = (screen.Cursor+1)%PrinterList.size();
		
		
		screen.Draw();
		
#ifdef LINUX
		if (Timer && time(0) - Timer > 5)
		{
			PrinterLock.unlock();
			Timer = 0;
			
		}
		else if (Timer == 0)
		{
			if (PrinterLock.try_lock())
			{
				
				Timer = time(0);
			}
		}
		//printf("%d\n", time(0) - Time1);
#endif
		
		int key = getch();
		
		if (key == 27)
		{
			Run = false;
		}
		else if (key == KEY_RESIZE)
		{
			screen.Resize();
		}
		else if (key == 'r')
		{
			screen.Resize();
		}
		else if (key == KEY_UP)
		{
			//screen.Cursor = (screen.Cursor < 1 ? 0 : screen.Cursor - 1);
			screen.ScrollY=MAX(screen.ScrollY-3, 0);
		}
		else if (key == KEY_DOWN)
		{
			//screen.Cursor = (screen.Cursor >= (int)PrinterList.size() - 1 ? PrinterList.size() - 1 : screen.Cursor + 1);
			//if ((screen.Cursor + 1) * (PrinterHeight + 1) > screen.Height + screen.ScrollY) { screen.ScrollY+=PrinterHeight; }
			screen.ScrollY+=3;
		}
		else if (key == KEY_LEFT)
		{
			screen.ScrollX=MAX(screen.ScrollX-3, 0);
		}
		else if (key == KEY_RIGHT)
		{
			screen.ScrollX+=3;
		}
	}

#ifdef LINUX
	PrinterLock.unlock();
#endif
	printerThread.join();

	return 0;
}
