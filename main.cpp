
//#include <iostream>

#include <time.h>
//#include <stdio.h>
//#include <vector>
//#include "Printer.h"
#include "Screen.h"


bool Run = true;
#ifdef LINUX
std::mutex PrinterLock;
time_t Timer = 0;
#endif

void UpdatePrinters()
{
	while (Run)
	{
#ifdef LINUX
		PrinterLock.lock();
#endif
		
		for (int i = 0; Run && i < (int)PrinterList.size(); i++)
		{
			PrinterList[i]->Update();
		}
		
#ifdef LINUX
		PrinterLock.unlock();
#endif
		
		if (Run)
		{
#ifdef WINDOWS
			Sleep(500);
#elif LINUX
			sleep(1);
#endif
		}
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
	
	if (PrinterList.size()) { Selected = PrinterList[0]; }
	
	screen.Draw();
	
	while (Run == true)
	{
		//screen.Cursor = (screen.Cursor+1)%PrinterList.size();
		
		
		
#ifdef LINUX
		//Used to create Network update delay
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
		
		SortPrinters();
		screen.Draw();
		
		int key = getch();
		
		if (key == 27) //Escape
		{
			Run = false;
		}
		else if (key == KEY_RESIZE)
		{
			screen.Resize();
		}
		else if (key == 'r')
		{
			Run = false;
#ifdef LINUX
			PrinterLock.unlock();
#endif
			printerThread.join();
			Run = true;
			InitPrinters();
			screen.Resize();
			if (PrinterList.size()) { Selected = PrinterList[0]; }
#ifdef LINUX
			Timer = time(0) - 6;
#endif
			printerThread = std::thread(UpdatePrinters);
		}
#ifdef WINDOWS
		else if (key == 'm')
		{
			resize_term(2000, 2000);
			screen.Resize();
		}
#endif
		else if (key == 'a')
		{
			screen.AutoScroll = !screen.AutoScroll;
		}
		else if (key == 'h' || key == 'i')
		{			
			if (!screen.Popup)
			{
				screen.PopupBorder = subwin(stdscr, screen.Height / 2 + 2, screen.Width / 2 + 2, screen.Height / 4 - 1, screen.Width / 4 - 1);
				screen.Popup = subwin(screen.PopupBorder, screen.Height / 2, screen.Width / 2, screen.Height / 4, screen.Width / 4);
			}
			else
			{
				delwin(screen.Popup);
				screen.Popup = 0;
				delwin(screen.PopupBorder);
				screen.PopupBorder = 0;
			}
		}
		else if (key == 'e')
		{
			bool allExpanded = true;
			for (int i = 0; i < (int)PrinterList.size(); i++) { allExpanded = allExpanded && PrinterList[i]->Expanded; }
			for (int i = 0; i < (int)PrinterList.size(); i++) { PrinterList[i]->Expanded = !allExpanded; }
			screen.Scroll();
		}
		else if (key == 's')
		{
			SortOrder = (SortOrder > 0 ? 0 : SortOrder+1);
			SortPrinters();
		}
		else if (key == KEY_UP)
		{
			screen.Cursor = (screen.Cursor < 1 ? 0 : screen.Cursor - 1);
			Selected = PrinterList[screen.Cursor];
			//screen.ScrollY=MAX(screen.ScrollY-3, 0);
			screen.Scroll();
			
			screen.AutoScroll = false;
		}
		else if (key == KEY_DOWN)
		{
			screen.Cursor = (screen.Cursor >= (int)PrinterList.size() - 1 ? PrinterList.size() - 1 : screen.Cursor + 1);
			Selected = PrinterList[screen.Cursor];
			//if ((screen.Cursor + 1) * (PrinterHeight + 1) > screen.Height + screen.ScrollY) { screen.ScrollY+=PrinterHeight; }
			//screen.ScrollY+=3;
			screen.Scroll();
			
			screen.AutoScroll = false;
		}
		else if (key == 10) //Enter Key
		{
			Selected->Expanded = !Selected->Expanded;
			screen.Scroll();
			
			screen.AutoScroll = false;
		}
		else if (key == KEY_LEFT)
		{
			screen.ScrollX=MAX(screen.ScrollX-3, 0);
			screen.AutoScrollDelay = (screen.AutoScrollDelay > 0 ? screen.AutoScrollDelay - 1 : 0);
		}
		else if (key == KEY_RIGHT)
		{
			screen.ScrollX+=3;
			screen.AutoScrollDelay = (screen.AutoScrollDelay > 0 ? screen.AutoScrollDelay - 1 : 0);
		}
	}

#ifdef LINUX
	PrinterLock.unlock();
#endif
	printerThread.join();
	
	DestroyPrinters();

	return 0;
}
