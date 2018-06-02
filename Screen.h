
struct Screen;

#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <vector>
#include <curl/curl.h>
#include <signal.h>
#include "Printer.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
//#define ROUND(a) (((a) - (int)(a)) > 0.5 ? (int)(a) + 1 : (int)(a) )

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

void FillLine(int chr);
void FillLine(WINDOW *win, int chr);
void Border(WINDOW *win, int x1, int y1, int x2, int y2);

struct Screen
{
	int Width, Height;
	static Screen *This;
	int Cursor = 0;
	int ScrollX = 0, ScrollY = 0;
	bool AutoScroll = false;
	int AutoScrollDelay = 0;
	
	WINDOW *TopPad = 0;
	WINDOW *Pad = 0;
	WINDOW *Debug = 0;
	WINDOW *PopupBorder = 0;
	WINDOW *Popup = 0;
	
	Screen();
	~Screen();
	
	static void Resize(int val);
	
	void Resize();
	
	void Draw();
	
	//Update scroll position based on boundaries
	void Scroll();
};

#endif
