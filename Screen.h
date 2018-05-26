
#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <vector>
#include <ncurses.h>
#include "Printer.h"

extern std::vector<Printer> PrinterList;

struct Screen
{
	int X;
	int Y;
	
	Screen()
	{
		initscr();
		cbreak();
		noecho();
		keypad(stdscr, TRUE);
		start_color();
		
		//nodelay(stdscr, TRUE);
		init_pair(1, COLOR_WHITE, COLOR_BLACK);
		init_pair(2, COLOR_BLACK, COLOR_CYAN);
		//init_pair(2, COLOR_BLACK, COLOR_GREEN);
		//attron(COLOR_PAIR(1));
		getmaxyx(stdscr, Y, X);
	}
	
	~Screen()
	{
		endwin();
	}
	
	void FillLine(char chr)
	{
		int x = getcurx(stdscr);
		addstr((std::string(X-x - 1, chr) + "\n").c_str());
	}
	
	void Draw()
	{
		attron(COLOR_PAIR(2));
		addstr("Name\tStatus"); FillLine(' ');
		
		attron(COLOR_PAIR(1));
		for (int i = 0; i < (int)PrinterList.size(); i++)
		{
			addstr((PrinterList[i].Name + PrinterList[i].GetStatus()).c_str()); FillLine(' ');
		}
		
		refresh();
	}
};

#endif
