
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
	wmove(win, y1, x1+1); whline(win, ACS_HLINE, x2-x1-1);
	wmove(win, y2, x1+1); whline(win, ACS_HLINE, x2-x1-1);
	wmove(win, y1+1, x1); wvline(win, ACS_VLINE, y2-y1-1);
	wmove(win, y1+1, x2); wvline(win, ACS_VLINE, y2-y1-1);
	
	auto replace = [&](int place)->int
	{
		const int map[4][11][2] = 
			{ 
				//ULCorner
				{ { ACS_HLINE, ACS_TTEE }, { ACS_VLINE, ACS_LTEE }, { ACS_ULCORNER, ACS_ULCORNER }, { ACS_URCORNER, ACS_TTEE }, { ACS_LLCORNER, ACS_LTEE }, { ACS_LRCORNER, ACS_PLUS }, { ACS_TTEE, ACS_TTEE }, { ACS_BTEE, ACS_PLUS }, { ACS_LTEE, ACS_LTEE }, { ACS_RTEE, ACS_PLUS }, { ACS_PLUS, ACS_PLUS } },
				//URCorner
				{ { ACS_HLINE, ACS_TTEE }, { ACS_VLINE, ACS_RTEE }, { ACS_ULCORNER, ACS_TTEE }, { ACS_URCORNER, ACS_URCORNER }, { ACS_LLCORNER, ACS_PLUS }, { ACS_LRCORNER, ACS_RTEE }, { ACS_TTEE, ACS_TTEE }, { ACS_BTEE, ACS_PLUS }, { ACS_LTEE, ACS_PLUS }, { ACS_RTEE, ACS_RTEE }, { ACS_PLUS, ACS_PLUS } },
				//LLCorner
				{ { ACS_HLINE, ACS_BTEE }, { ACS_VLINE, ACS_LTEE }, { ACS_ULCORNER, ACS_LTEE }, { ACS_URCORNER, ACS_PLUS }, { ACS_LLCORNER, ACS_LLCORNER }, { ACS_LRCORNER, ACS_BTEE }, { ACS_TTEE, ACS_PLUS }, { ACS_BTEE, ACS_BTEE }, { ACS_LTEE, ACS_LTEE }, { ACS_RTEE, ACS_PLUS }, { ACS_PLUS, ACS_PLUS } },
				//LRCorner
				{ { ACS_HLINE, ACS_BTEE }, { ACS_VLINE, ACS_RTEE }, { ACS_ULCORNER, ACS_PLUS }, { ACS_URCORNER, ACS_RTEE }, { ACS_LLCORNER, ACS_BTEE }, { ACS_LRCORNER, ACS_LRCORNER }, { ACS_TTEE, ACS_PLUS }, { ACS_BTEE, ACS_BTEE }, { ACS_LTEE, ACS_PLUS }, { ACS_RTEE, ACS_RTEE }, { ACS_PLUS, ACS_PLUS } },
			};
		int curr = winch(win), placeNum = 0;
		if (place == (int)ACS_ULCORNER) { placeNum = 0; } else if (place == (int)ACS_URCORNER) { placeNum = 1; } else if (place == (int)ACS_LLCORNER) { placeNum = 2; } else if (place == (int)ACS_LRCORNER) { placeNum = 3; }
		else { return place; }
		for (int i = 0; i < 11; i++)
		{
			if (curr == map[placeNum][i][0]) { return map[placeNum][i][1]; }
		}
		return place;
	};
	
	wmove(win, y1, x1); waddch(win, replace(ACS_ULCORNER));
	wmove(win, y1, x2); waddch(win, replace(ACS_URCORNER));
	wmove(win, y2, x1); waddch(win, replace(ACS_LLCORNER));
	wmove(win, y2, x2); waddch(win, replace(ACS_LRCORNER));
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
	
	for (int i = 0; Pad && i < (int)PrinterList.size(); i++)
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
	waddstr(TopPad, "Name\tStatus"); FillLine(TopPad, ' ');
	//wattrset(stdscr, 0);
	
	
	//wborder(Pad, '1','2','3','4','5','6','7','8');
	int printerPos = 1;
	//int selectedPrinterPos = 0;
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		int x = (i % PrinterCols) * (PrinterWidth+1);
		
		//if (i == Cursor) { selectedPrinterPos = printerPos; }
		//if (i) { wmove(Pad, printerPos-1, 0); waddch(Pad, ACS_LTEE); wmove(Pad, printerPos-1, PrinterWidth+1); waddch(Pad, ACS_RTEE); }
		
		
		Border(Pad, x, printerPos-1, x+PrinterWidth+1, printerPos+PrinterHeight);
		PrinterList[i].Draw(this);
		//wborder(PrinterList[i].Pad, '1','2','3','4','5','6','7','8');
		
		mvderwin(PrinterList[i].Pad, printerPos, x+1);
		if ((i+1)%PrinterCols == 0) { printerPos+=PrinterHeight+1; } 
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
	
	/*move(1,1); 
	addch(ACS_ULCORNER); addch(ACS_URCORNER); addch(ACS_LLCORNER); addch(ACS_LRCORNER); addch(ACS_PLUS);
	move(1,1); BottomText+=std::to_string(inch()); BottomText+=":"; BottomText+=std::to_string(ACS_ULCORNER); BottomText+=" ";
	move(1,2); BottomText+=std::to_string(inch()); BottomText+=":"; BottomText+=std::to_string(ACS_URCORNER); BottomText+=" ";
	move(1,3); BottomText+=std::to_string(inch()); BottomText+=":"; BottomText+=std::to_string(ACS_LLCORNER); BottomText+=" ";
	move(1,4); BottomText+=std::to_string(inch()); BottomText+=":"; BottomText+=std::to_string(ACS_LRCORNER); BottomText+=" ";
	move(1,5); BottomText+=std::to_string(inch()); BottomText+=":"; BottomText+=std::to_string(ACS_PLUS); BottomText+=" ";*/
	
	
	BottomText += "Width:" + std::to_string(Width) + "  Height:" + std::to_string(Height) + "  PrinterWidth:" + std::to_string(PrinterWidth) + "  PrinterCols:" + std::to_string(PrinterCols) + "  Val:" + std::to_string(A_CHARTEXT);
	mvaddstr(Height-1, 0, BottomText.c_str());
	BottomText = "";
	
	refresh();
	
	//touchwin(Pad);
	//prefresh(Pad, ScrollY, ScrollX, 1, 0, Height-2, Width-1);
	prefresh(Pad, ScrollY, ScrollX, 1, 0, Height-2, Width-1);
	
	prefresh(TopPad, 0, ScrollX,0,0,1,Width-1);
}