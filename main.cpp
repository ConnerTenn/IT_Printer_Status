
//#include <iostream>

#include <time.h>
//#include <stdio.h>
//#include <vector>
//#include "Printer.h"
#include "Screen.h"


bool Run = true;

time_t Timer = 0;
const int ThreadNum = 10;
std::thread *PrinterThreads[ThreadNum];
bool PrinterComplete[ThreadNum];
int UpdateIndex = 0;
std::mutex UpdateIndexMutex;

void UpdatePrinters(int index)
{
	bool loop = true;
	while (loop)
	{
		int updateI = 0;
		UpdateIndexMutex.lock();
		loop = UpdateIndex < (int)PrinterUpdateThreadList.size() && Run;
		updateI = UpdateIndex;
		UpdateIndex++;
		UpdateIndexMutex.unlock();
			
		if (loop) { PrinterUpdateThreadList[updateI]->Update(); }
	}
		

	PrinterComplete[index] = true;
}



void InitThreads()
{
	UpdateIndex = 0;
	Timer = 0;
	for (int i = 0; i < ThreadNum; i++)
	{
		PrinterComplete[i] = false;
		PrinterThreads[i] = new std::thread(UpdatePrinters, i);
	}
}

void JoinThreads()
{
	Run = false;

	for (int i = 0; i < ThreadNum; i++)
	{
		if (PrinterThreads[i])
		{
			PrinterThreads[i]->join();
			delete PrinterThreads[i];
			PrinterThreads[i] = 0;
		}
	}

	Run = true;
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
		
		if (Timer && time(0) - Timer > 5)
		{
			UpdateIndex = 0;
			InitThreads();
			Timer = 0;
			
		}
		else if (!Timer && UpdateIndex >= (int)PrinterUpdateThreadList.size())
		{
			bool complete = true;
			for (int i = 0; i <ThreadNum; i++) { complete = complete && PrinterComplete[i]; }

			if (complete)
			{
				JoinThreads();
				Timer = time(0);
			}
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
			JoinThreads();
			InitPrinters();
			screen.Resize();
			if (PrinterList.size()) { Selected = PrinterList[0]; }
			Timer = 0;
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
