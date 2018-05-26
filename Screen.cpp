
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
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_BLACK, COLOR_GREEN);
	init_pair(3, COLOR_BLACK, COLOR_CYAN);
	
	
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
	
	wattron(stdscr, COLOR_PAIR(2));
	waddstr(stdscr, "Name\tStatus"); FillLine(' ');
	
	
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		if (i == Cursor)
		{
			attron(COLOR_PAIR(3));
		}
		else
		{
			attron(COLOR_PAIR(1));
		}
		waddstr(stdscr, (PrinterList[i].Name + PrinterList[i].GetStatus()).c_str()); FillLine(' ');
	}
	
	refresh();
}