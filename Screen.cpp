
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

std::string GetFillLine(char chr)
{
	return std::string(getmaxx(stdscr)-getcurx(stdscr), chr);
}

std::string GetFillLine(WINDOW *win, char chr)
{
	return std::string(getmaxx(win)-getcurx(win), chr);
}

void Border(WINDOW *win, int x1, int y1, int x2, int y2)
{
	wmove(win, y1, x1+1); whline(win, ACS_HLINE, x2-x1);
	wmove(win, y2, x1+1); whline(win, ACS_HLINE, x2-x1);
	wmove(win, y1+1, x1); wvline(win, ACS_VLINE, y2-y1);
	wmove(win, y1+1, x2); wvline(win, ACS_VLINE, y2-y1);
	
	wmove(win, y1, x1); waddch(win, ACS_ULCORNER);
	wmove(win, y1, x2); waddch(win, ACS_URCORNER);
	wmove(win, y2, x1); waddch(win, ACS_LLCORNER);
	wmove(win, y2, x2); waddch(win, ACS_LRCORNER);
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
	init_color(COLOR_WHITE, 1000,1000,1000);
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
	
	
	TopPad = newpad(1, 1000);
	Resize();
	
	
	
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
	getmaxyx(stdscr, Height, Width);
	
	PrinterCols = Width / MaxPrinterWidth + 1;
	PrinterWidth = Width / PrinterCols - 2;
	
	if (Pad) { delwin(Pad); }
	Pad = newpad(MAX(((int)PrinterList.size()/PrinterCols + 1)*(PrinterHeight+1)+1, Height-2), (PrinterWidth+2)*PrinterCols);
	
	
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		if (PrinterList[i].Pad) { delwin(PrinterList[i].Pad); }
		PrinterList[i].Pad = subpad(Pad, PrinterHeight, PrinterWidth, 0, 0);
	}
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
	int printerPos = 1;
	//int selectedPrinterPos = 0;
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		int x = (i % PrinterCols) * (PrinterWidth+1);
		
		//if (i == Cursor) { selectedPrinterPos = printerPos; }
		Border(Pad, x, printerPos-1, x+PrinterWidth+1, printerPos+PrinterHeight);
		//if (i) { wmove(Pad, printerPos-1, 0); waddch(Pad, ACS_LTEE); wmove(Pad, printerPos-1, PrinterWidth+1); waddch(Pad, ACS_RTEE); }
		
		mvderwin(PrinterList[i].Pad, printerPos, x+1); if ((i+1)%PrinterCols == 0) { printerPos+=PrinterHeight+1; } 
		PrinterList[i].Draw(this);
		//wborder(PrinterList[i].Pad, '1','2','3','4','5','6','7','8');
	}
	
	/*{
		wattrset(Pad, COLOR_PAIR(0b110110));
		Border(Pad, 0, selectedPrinterPos-1, PrinterWidth+1, selectedPrinterPos+PrinterHeight);
		//if (i) { wmove(Pad, printerPos-1, 0); waddch(Pad, ACS_LTEE); wmove(Pad, printerPos-1, PrinterWidth+1); waddch(Pad, ACS_RTEE); }
		wattrset(Pad, COLOR_PAIR(NORMAL));
	}*/
	
	/*waddstr(Pad, "\n");
	for (unsigned int i = 0; i <= 0b111111 * 2; i++)
	{
		if (!(i % 8)) { waddstr(Pad, "\n"); }
		if (i==0b1000000) { waddstr(Pad, "\n\n"); }
		wattrset(Pad, COLOR_PAIR(i%0b111111) | (i&0b1000000?A_BOLD : 0));
		waddstr(Pad, "Apple");
	}*/
	
	wattrset(Pad, 0);
	
	BottomText += "Width:" + std::to_string(Width) + "  Height:" + std::to_string(Height) + "  PrinterWidth:" + std::to_string(PrinterWidth) + "  PrinterCols:" + std::to_string(PrinterCols);
	mvaddstr(Height-1, 0, BottomText.c_str());
	BottomText = "";
	
	refresh();
	prefresh(TopPad, 0, ScrollX,0,0,1,Width-1);
	
	touchwin(Pad);
	/*for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		//prefresh(PrinterList[i].Pad, 0, 0, i*PrinterHeight+1, 0, PrinterHeight, PrinterWidth);
		
		//wrefresh(PrinterList[i].Pad);
	}*/
	prefresh(Pad, ScrollY, ScrollX,1,0,Height-2,Width-1);
}