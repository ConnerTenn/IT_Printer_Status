
//#include <iostream>

#include <time.h>
//#include <stdio.h>
//#include <vector>
//#include "Printer.h"
#include "Screen.h"


bool Run = true;

time_t Timer = 0;
std::vector<std::mutex *> PrinterLocks;
std::vector<std::thread *> PrinterThreads;
int UpdateIndex = 0;
std::mutex UpdateIndexMutex;

void UpdatePrinters(int index)
{
	while (Run)
	{
		PrinterLocks[index]->lock();
		
		while (UpdateIndex < (int)PrinterUpdateThreadList.size() && Run)
		{
			int updateI = 0;
			UpdateIndexMutex.lock();
			updateI = UpdateIndex;
			//UpdateIndex = (UpdateIndex < (int)PrinterUpdateThreadList.size() ? UpdateIndex+1 : 0);
			UpdateIndex++;
			UpdateIndexMutex.unlock();
			
			PrinterUpdateThreadList[updateI]->Update();
		}
		
		PrinterLocks[index]->unlock();
		
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



void InitThreads()
{
	for (int i = 0; i < 4; i++)
	{
		PrinterLocks.push_back(new std::mutex);
		PrinterThreads.push_back(new std::thread(UpdatePrinters, i));
	}
	UpdateIndex = 0;
}

void JoinThreads()
{
	for (int i = 0; i < (int)PrinterLocks.size(); i++) { PrinterLocks[i]->unlock(); }
	for (int i = 0; i < (int)PrinterThreads.size(); i++)
	{
		PrinterThreads[i]->join();
		delete PrinterThreads[i];
	}
	for (int i = 0; i < (int)PrinterLocks.size(); i++) { delete PrinterLocks[i]; }
	
	PrinterThreads.clear();
	PrinterLocks.clear();
}

int main()
{
	MEVENT mouseEvent;
	
	
	InitPrinters();
	Screen screen; screen.Draw();
	
	InitThreads();
	
	Timer=0;
	
	if (PrinterList.size()) { Selected = PrinterList[0]; }
	
	screen.Draw();
	
	while (Run == true)
	{
		
		if (Timer && time(0) - Timer > 10)
		{
			UpdateIndex = 0;
			for (int i = 0; i < (int)PrinterLocks.size(); i++) { PrinterLocks[i]->unlock(); }
			Timer = 0;
			
		}
		else if (UpdateIndex >= (int)PrinterUpdateThreadList.size())
		{
			bool lock = true;
			for (int i = 0; i < (int)PrinterLocks.size(); i++) { lock = lock && PrinterLocks[i]->try_lock(); }
			
			if (lock) { Timer = time(0); }
		}
		
		
		SortPrinters();
		screen.Draw();
		
		int key = getch();
		
		if (key == 27 && !screen.Popup) //Escape
		{
			Run = false;
		}
		else if (key == KEY_RESIZE)
		{
			screen.Resize();
		}
		else if (key == KEY_MOUSE && getmouse(&mouseEvent) == OK)
		{
			//detect leftclick
			if(mouseEvent.bstate & (BUTTON1_PRESSED | BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED))
			{
				int cursorY = (mouseEvent.y - 1) + screen.ScrollY;
				int clickedI = 0, cursorMaxY = 0, cursorMinY = 0;//, maxY = 0;
				
				//calculate index of printer that was clicked on
				for (int i = 0; i < (int)PrinterList.size(); i++)
				{
					cursorMaxY += (PrinterList[i]->Expanded ? PrinterHeight : 1) + (i < (int)PrinterList.size() - 1 ? 1 : 0);
					if (i-1 >= 0) { cursorMinY += (PrinterList[i-1]->Expanded ? PrinterHeight : 1) + (i < (int)PrinterList.size() - 1 ? 1 : 0); }
					
					if (cursorMinY <= cursorY && cursorY < cursorMaxY) { clickedI = i; i = PrinterList.size(); }
				}
				if (cursorY > cursorMaxY) { clickedI = (int)PrinterList.size() - 1; }
				
				if (mouseEvent.bstate & (BUTTON1_PRESSED | BUTTON1_CLICKED))
				{
					//Select printer
					if (screen.Cursor != clickedI)
					{
						screen.Cursor = clickedI;
						Selected = PrinterList[screen.Cursor];
					}
					//Already selected; toggle expanded
					else
					{
						Selected->Expanded = !Selected->Expanded;
						screen.Scroll();
					}
				}
				//Immediately select and expand
				else if(mouseEvent.bstate & BUTTON1_DOUBLE_CLICKED)
				{
					screen.Cursor = clickedI;
					Selected = PrinterList[screen.Cursor];
					Selected->Expanded = !Selected->Expanded;
					screen.Scroll();
				}
			}
			//Scroll Up
			else if (mouseEvent.bstate & 0x10000)
			{
				//screen.Cursor = (screen.Cursor < 1 ? 0 : screen.Cursor - 1);
				//Selected = PrinterList[screen.Cursor];
				//screen.Scroll();
				screen.ScrollY -= MIN(6, screen.ScrollY);
				
				//screen.AutoScroll = false;
			}
			//Scroll Down
			else if (mouseEvent.bstate & 0x200000)
			{
				//screen.Cursor = (screen.Cursor >= (int)PrinterList.size() - 1 ? PrinterList.size() - 1 : screen.Cursor + 1);
				//Selected = PrinterList[screen.Cursor];
				//screen.Scroll();
				int maxY = 0;
				GetPrinterDisplayHeight(&maxY);
				screen.ScrollY += MAX(0, MIN(6, maxY - (screen.ScrollY + screen.Height - 2)));
				
				//screen.AutoScroll = false;
			}
		}
		else if (key == 'r')
		{
			Run = false;
			JoinThreads();
			Run = true;
			InitPrinters();
			screen.Resize();
			if (PrinterList.size()) { Selected = PrinterList[0]; }
#ifdef LINUX
			Timer = time(0) - 6;
#endif
			InitThreads();
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
		else if (key == 27 && screen.Popup)
		{
			delwin(screen.Popup);
			screen.Popup = 0;
			delwin(screen.PopupBorder);
			screen.PopupBorder = 0;
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
			Selected = PrinterList[screen.Cursor];
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
	
	JoinThreads();
	
	DestroyPrinters();

	return 0;
}
