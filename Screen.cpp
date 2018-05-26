
#include "Screen.h"

Screen *Screen::This = 0;

void FillLine(char chr)
{
	addstr(std::string(getmaxx(stdscr)-getcurx(stdscr), chr).c_str());
}

void FillLine(WINDOW *win, char chr)
{
	waddstr(win, std::string(getmaxx(win)-getcurx(win), chr).c_str());
}

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
	
	//init_color(27, 0,0,0); //Colour 27 of BGR values
	/*init_pair(NORMAL, COLOR_WHITE, COLOR_BLACK);
	init_pair(TOPBAR, COLOR_BLACK, COLOR_GREEN);
	init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);
	init_pair(RED, COLOR_RED, COLOR_BLACK);
	init_pair(HIGHLIGHT, COLOR_BLACK, COLOR_CYAN);
	init_pair(YELLOW_HIGHLIGHT, COLOR_YELLOW, COLOR_CYAN);
	init_pair(RED_HIGHLIGHT, COLOR_RED, COLOR_CYAN);*/
	
	for (unsigned short i = 0; i <= 0b111111; i++)
	{
		init_pair(i, (i & 0b111000) >> 3, i & 0b000111);
	}
	
	
	getmaxyx(stdscr, Height, Width);
	
	TopPad = newpad(1, 1000);
	Pad = newpad(MAX((int)PrinterList.size()*7, Height-2), 60);
	
	//signal(SIGWINCH, Resize);
}

Screen::~Screen()
{	
	delwin(TopPad);
	delwin(Pad);
	endwin();
}

void Screen::Resize(int val)
{
	//getmaxyx(stdscr, Screen::This->Height, Screen::This->Width);
	Screen::This->Resize();
	//addstr("Resize\n");
	//refresh();
}

void Screen::Resize()
{
	endwin();
	
	refresh();
	getmaxyx(stdscr, Screen::This->Height, Screen::This->Width);
	
	
}

void Screen::Draw()
{
	clear();
	wclear(TopPad);
	wclear(Pad);
	
	wattrset(TopPad, COLOR_PAIR(TOPBAR));
	waddstr(TopPad, "Name v\tStatus"); FillLine(TopPad, ' ');
	//wattrset(stdscr, 0);
	
	
	//wborder(Pad, '1','2','3','4','5','6','7','8');
	
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		/*auto tab = [&](){ wattrset(Pad, COLOR_PAIR(i==Cursor ? HIGHLIGHT : NORMAL)); waddstr(Pad, "    "); wattrset(Pad, COLOR_PAIR(NORMAL)); };
		//wattrset(Pad, COLOR_PAIR(i==Cursor ? HIGHLIGHT : NORMAL));
		wattrset(Pad, (i==Cursor ? A_BOLD : 0));
		wattrset(Pad, (i==Cursor ? COLOR_PAIR(HIGHLIGHT) | A_BOLD | A_DIM : COLOR_PAIR(NORMAL)));
		waddstr(Pad, (PrinterList[i].Name).c_str());
		wattrset(Pad, COLOR_PAIR(NORMAL));
		waddstr(Pad, ("\t" + PrinterList[i].Status).c_str());  FillLine(Pad, ' ');
		
		tab(); waddstr(Pad, "Toner ["); for (int i=0;i<10;i++) {waddch(Pad, i<5?ACS_CKBOARD:' ');} waddstr(Pad, "]"); FillLine(Pad, ' ');
		tab(); waddstr(Pad, "Tray 1\tOK\t500\tLetter\tPlain Paper"); FillLine(Pad, ' ');
		tab(); waddstr(Pad, "Tray 2\tOK\t500\tLetter\tPlain Paper"); FillLine(Pad, ' ');
		tab(); waddstr(Pad, "Tray 3\tOK\t500\tLetter\tPlain Paper"); FillLine(Pad, ' ');
		tab(); waddstr(Pad, "Tray 4\tOK\t500\tLetter\tPlain Paper"); FillLine(Pad, ' ');*/
		
		PrinterList[i].Draw(this, Pad, i == Cursor);
		
	}
	
	wattrset(Pad, 0);
	
	BottomText += "Width:" + std::to_string(Width) + "  Height:" + std::to_string(Height);
	mvaddstr(Height-1, 0, BottomText.c_str());
	BottomText = "";
	
	refresh();
	prefresh(TopPad, 0, ScrollX,0,0,1,Width-1);
	prefresh(Pad, ScrollY, ScrollX,1,0,Height-2,Width-1);
}