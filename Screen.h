
#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <vector>
#include <ncurses.h>
#include <signal.h>
#include "Printer.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))

extern std::vector<Printer> PrinterList;

enum Colours
{
	NORMAL = 1,
	TOPBAR = 2,
	YELLOW = 0x5,
	RED = 0x4,
	HIGHLIGHT = 0x8,
	YELLOW_HIGHLIGHT = HIGHLIGHT | YELLOW,
	RED_HIGHLIGHT = HIGHLIGHT | RED,
};

struct Screen
{
	int Width, Height;
	static Screen *This;
	int Cursor = 0;
	int ScrollX = 0, ScrollY = 0;
	
	WINDOW *TopPad;
	WINDOW *Pad;
	
	Screen();
	~Screen();
	
	static void Resize(int val);
	
	void FillLine(char chr);
	void FillLine(WINDOW *win, char chr);
	std::string GetFullLine(char chr);
	std::string GetFullLine(WINDOW *win, char chr);
	
	void Draw();
};

#endif
