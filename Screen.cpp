
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
	
	//Normal
	//Bar
	//Selected
	//
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_BLACK, COLOR_GREEN);
	init_pair(3, COLOR_BLACK, COLOR_CYAN);
	init_pair(4, COLOR_YELLOW, COLOR_BLACK);
	init_pair(5, COLOR_RED, COLOR_BLACK);
	
	
	getmaxyx(stdscr, Y, X);
	
	signal(SIGWINCH, Resize);
}

Screen::~Screen()
{	
	endwin();
}

void Screen::Resize(int val)
{
	getmaxyx(stdscr, Screen::This->Y, Screen::This->X);
	addstr("Resize\n");
	refresh();
}

void Screen::FillLine(char chr)
{
	int x = getcurx(stdscr);
	addstr(std::string(X-x, chr).c_str());
}

void Screen::Draw()
{
	clear();
	
	wattrset(stdscr, COLOR_PAIR(2));
	waddstr(stdscr, "Name\tStatus"); FillLine(' ');
	
	
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		if (i == Cursor)
		{
			attrset(COLOR_PAIR(3));
		}
		else
		{
			attrset(COLOR_PAIR(1));
		}
		PrinterList[i].Update();
		waddstr(stdscr, (PrinterList[i].Name + "\t").c_str());
		if (i != Cursor) { if (PrinterList[i].StatusColour == 0) { attrset(COLOR_PAIR(1)); } if (PrinterList[i].StatusColour == 1) { attrset(A_BOLD | COLOR_PAIR(4)); } }
		waddstr(stdscr, PrinterList[i].Status.c_str()); FillLine(' ');
	}
	
	refresh();
}