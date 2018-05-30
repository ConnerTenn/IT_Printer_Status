
struct Screen;

#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <vector>
#include <curl/curl.h>
#ifdef WINDOWS
#include <curses.h>
#elif LINUX
#include <ncurses.h>
#endif
#include <signal.h>
#include "Printer.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
//#define ROUND(a) (((a) - (int)(a)) > 0.5 ? (int)(a) + 1 : (int)(a) )

extern std::vector<Printer> PrinterList; //Defined in main.cpp

//Colour Pair definitions
//Linux:	BGRBGR
//Windows:	RGBRGB
enum Colours
{
	NORMAL = 0b111000,
	TOPBAR = 0b000010,
	YELLOW = 0b011000,
	RED = 0b001000,
	HIGHLIGHT = 0b000110,
	YELLOW_HIGHLIGHT = HIGHLIGHT | YELLOW,
	RED_HIGHLIGHT = HIGHLIGHT | RED,
	GREY = 64,
};

void FillLine(char chr);
void FillLine(WINDOW *win, char chr);
/* Warning, Do No Use; Cursor does not get updated before function call. If used concatinated with other characters to print a full line, will result in extra characters.
std::string GetFullLine(char chr);
std::string GetFullLine(WINDOW *win, char chr);*/
void Border(WINDOW *win, int x1, int y1, int x2, int y2);

struct Screen
{
	int Width, Height;
	static Screen *This;
	int Cursor = 0;
	int ScrollX = 0, ScrollY = 0;
	
	std::string BottomText;
	
	WINDOW *TopPad = 0;
	WINDOW *Pad = 0;
	WINDOW *Debug = 0;
	
	Screen();
	~Screen();
	
	static void Resize(int val);
	
	void Resize();
	
	void Draw();
};

#endif
