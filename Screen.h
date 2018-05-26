
#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <vector>
#include <ncurses.h>
#include <signal.h>
#include "Printer.h"

extern std::vector<Printer> PrinterList;

struct Screen
{
	int X;
	int Y;
	static Screen *This;
	int Cursor = 0;
	
	WINDOW *Win;
	
	Screen();
	~Screen();
	
	static void Resize(int val);
	
	void FillLine(char chr);
	//void FillLine(WINDOW *win, char chr);
	
	void Draw();
};

#endif
