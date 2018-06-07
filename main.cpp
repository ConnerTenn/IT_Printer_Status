
//#include <iostream>

#include <time.h>
//#include <stdio.h>
//#include <vector>
//#include "Printer.h"
#include "Threads.h"
#include "Screen.h"

bool Run = true;

void GetConfigValue(std::string str, std::string *identifier, std::string *value)
{
	std::string id, val;
	int state = 0; //0:space, 1:identifier, 2:space, 3:equals, 4:space, 5:value
	
	for (int i = 0; i < (int)str.size(); i++)
	{
		if (str[i] == ' ') 
		{
			if (state == 1) { state = 2; }
			else if (state == 3) { state = 4; }
		}
		else if (str[i] == '#')
		{
			i = str.size();
		}
		else if (str[i] == '=')
		{
			if (state == 1 || state == 2) { state = 3; }
		}
		else if (isalnum(str[i]))
		{
			if (state == 0)
			{
				state = 1;
				id += str[i];
			}
			else if (state == 4 || state == 5)
			{
				val += str[i];
			}
		}
	}
	
	*identifier = id;
	*value = val;
}

void LoadConfig()
{
	std::ifstream file("config.txt");
	std::string line;
	
	while(std::getline(file, line))
	{
		if (line.size())
		{
			std::string identifier;
			std::string value;
			
			GetConfigValue(line, &identifier, &value);
			
			if (identifier == "MinStatusLength")
			{
				MinStatusLength = atoi(value.c_str());
			}
			else if (identifier == "MaxStatusLength")
			{
				MaxStatusLength = atoi(value.c_str());
			}
			else if (identifier == "DynamicColumns")
			{
				
			}
			else if (identifier == "RefreshDelay")
			{
				RefreshDelay = atoi(value.c_str());
			}
			else if (identifier == "UpdateThreads")
			{
				ThreadNum = atoi(value.c_str());
			}
			else if (identifier == "NetworkTimeout")
			{
				NetworkTimeout = atoi(value.c_str());
			}
			else if (identifier == "URLTopbar")
			{
				
			}
			else if (identifier == "URLStatus")
			{
				
			}
		}
	}
}



int main()
{
	MEVENT mouseEvent;
	
	LoadConfig();
	InitPrinters();
	Screen screen; screen.Draw();
	
	InitThreads();
	
	Timer=0;
	
	if (PrinterList.size()) { Selected = PrinterList[0]; }
	
	screen.Draw();
	
	while (Run == true)
	{
		UpdateThreadTimer();
		
		
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
						screen.UpdateScroll();
					}
				}
				//Immediately select and expand
				else if(mouseEvent.bstate & BUTTON1_DOUBLE_CLICKED)
				{
					screen.Cursor = clickedI;
					Selected = PrinterList[screen.Cursor];
					Selected->Expanded = !Selected->Expanded;
					screen.UpdateScroll();
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
			LoadConfig();
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
			screen.UpdateScroll();
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
			screen.UpdateScroll();
			
			screen.AutoScroll = false;
		}
		else if (key == KEY_DOWN)
		{
			screen.Cursor = (screen.Cursor >= (int)PrinterList.size() - 1 ? PrinterList.size() - 1 : screen.Cursor + 1);
			Selected = PrinterList[screen.Cursor];
			//if ((screen.Cursor + 1) * (PrinterHeight + 1) > screen.Height + screen.ScrollY) { screen.ScrollY+=PrinterHeight; }
			//screen.ScrollY+=3;
			screen.UpdateScroll();
			
			screen.AutoScroll = false;
		}
		else if (key == 10) //Enter Key
		{
			Selected->Expanded = !Selected->Expanded;
			screen.UpdateScroll();
			
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
