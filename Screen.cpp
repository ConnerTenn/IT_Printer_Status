
#include "Screen.h"

Screen *Screen::This = 0;

Screen::Screen()
{
	Screen::This = this;
	
	initscr();
	
	cbreak(); //disable buffereing until enter is pressed
	noecho(); //dont print character to termial
	keypad(stdscr, TRUE); //retrive special keys
	//nodelay(stdscr, TRUE); //dont delay
	timeout(1000); //block getch for 1 sec
	set_escdelay(10); //escape character dalay is 10 ms
	//raw();
	curs_set(0);//invisible cursor
	
	start_color();
	
	//init_color(27, 0,0,0); //Colour 27 of RGB values
	init_pair(NORMAL, COLOR_WHITE, COLOR_BLACK);
	init_pair(TOPBAR, COLOR_BLACK, COLOR_GREEN);
	init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);
	init_pair(RED, COLOR_RED, COLOR_BLACK);
	init_pair(HIGHLIGHT, COLOR_BLACK, COLOR_CYAN);
	
	
	getmaxyx(stdscr, Height, Width);
	
	TopPad = newpad(1, 1000);
	Pad = newpad(MAX((int)PrinterList.size()*5, Height-2), 1000);
	
	signal(SIGWINCH, Resize);
}

Screen::~Screen()
{	
	delwin(TopPad);
	delwin(Pad);
	endwin();
}

void Screen::Resize(int val)
{
	getmaxyx(stdscr, Screen::This->Height, Screen::This->Width);
	addstr("Resize\n");
	refresh();
}

void Screen::FillLine(char chr)
{
	addstr(std::string(getmaxx(stdscr)-getcurx(stdscr), chr).c_str());
}

void Screen::FillLine(WINDOW *win, char chr)
{
	waddstr(win, std::string(getmaxx(win)-getcurx(win), chr).c_str());
}

void Screen::Draw()
{
	clear();
	wclear(TopPad);
	wclear(Pad);
	
	wattrset(TopPad, COLOR_PAIR(2));
	waddstr(TopPad, "Name\tStatus"); FillLine(TopPad, ' ');
	//wattrset(stdscr, 0);
	
	
	wborder(Pad, '1','2','3','4','5','6','7','8');
	
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		if (i == Cursor)
		{
			wattrset(Pad,COLOR_PAIR(HIGHLIGHT));
		}
		else
		{
			wattrset(Pad,COLOR_PAIR(NORMAL));
		}
		PrinterList[i].Update();
		waddstr(Pad, (PrinterList[i].Name + "\t").c_str());
		//if (i != Cursor) { if (PrinterList[i].StatusColour == 0) { wattrset(Pad, COLOR_PAIR(1)); } if (PrinterList[i].StatusColour == 1) { wattrset(Pad, A_BOLD | COLOR_PAIR(4)); } }
		waddstr(Pad, (PrinterList[i].Status).c_str()); FillLine(Pad, ' ');
	}
	
	wattrset(Pad, 0);
	
	refresh();
	prefresh(TopPad, 0, ScrollX,0,0,1,Width-1);
	prefresh(Pad, ScrollY, ScrollX,1,0,Height-2,Width-1);
}