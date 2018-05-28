
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

/*std::string GetFullLine(char chr)
{
	return std::string(getmaxx(stdscr)-getcurx(stdscr), chr);
}

std::string GetFullLine(WINDOW *win, char chr)
{
	return std::string(getmaxx(win)-getcurx(win), chr);
}*/

void Border(WINDOW *win, int x1, int y1, int x2, int y2)
{
	//Add straight lines
	wmove(win, y1, x1+1); whline(win, ACS_HLINE, x2-x1-1);
	wmove(win, y2, x1+1); whline(win, ACS_HLINE, x2-x1-1);
	wmove(win, y1+1, x1); wvline(win, ACS_VLINE, y2-y1-1);
	wmove(win, y1+1, x2); wvline(win, ACS_VLINE, y2-y1-1);
	
	//Checks what's under the cursor and replaces it with the new border character; handles border overlap
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
	
	//handle corners of the border
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
#ifdef LINUX
	set_escdelay(10); //escape character dalay is 10 ms
#endif
	//raw();
	curs_set(0);//invisible cursor
	
	start_color();
	
	init_color(GREY, 500, 500, 500); //Colour of BGR values, max 1000
	init_color(COLOR_WHITE, 1000,1000,1000);

	init_pair(GREY, GREY, 0);
#ifdef LINUX
	//Format: BGRBGR
	for (unsigned short i = 0; i <= 0b111111; i++)
	{
		init_pair(i, (i & 0b111000) >> 3, i & 0b000111);
	}
#elif WINDOWS
	//Format: RGBRGB
	for (unsigned short i = 0; i <= 0b111111; i++)
	{
		//swaps R and B signals for initilization
		init_pair((i & 0b010010) | ((i & 0b100100) >> 2) | ((i & 0b001001) << 2), (i & 0b111000) >> 3, i & 0b000111);
	}
#endif
	
	TopPad = newpad(1, 1000);
	Resize();

#ifdef WINDOWS
	//signal(SIGWINCH, Resize);
#endif
}

Screen::~Screen()
{	
	delwin(TopPad); TopPad = 0;
	delwin(Pad); Pad = 0;
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
#ifdef WINDOWS
	//resize_term(2000, 2000);
	getmaxyx(stdscr, Height, Width);
	resize_term(Height, Width);
	//getmaxyx(stdscr, Height, Width);
	refresh();
#endif


#ifdef LINUX
	endwin();
	
	refresh();

	
	getmaxyx(stdscr, Height, Width);
#endif
	
	//Get Nuber of columns and width of the printer tiles
	PrinterCols = MAX(1, Width / (MinPrinterWidth+2));
	PrinterWidth = MAX(Width / PrinterCols - 2, MinPrinterWidth);
	

	//FIX: This implementation works but is very slow

	//Recreate Pad
	if (Pad) { delwin(Pad); }
	Pad = newpad(MAX(((int)PrinterList.size()/PrinterCols + 1)*(PrinterHeight+1)+1, Height-2), (PrinterWidth+2)*PrinterCols);
	
	//Recreate Pads for each printer
	for (int i = 0; Pad && i < (int)PrinterList.size(); i++)
	{
		if (PrinterList[i].Pad) { delwin(PrinterList[i].Pad); }
		PrinterList[i].Pad = subpad(Pad, PrinterHeight, PrinterWidth, 0, 0);
	}
}

void Screen::Draw()
{
	//Clear Pads
	clear();
	wclear(TopPad);
	wclear(Pad);
	
	//Top Text Panel
	wattrset(TopPad, COLOR_PAIR(TOPBAR));
	waddstr(TopPad, "Name\tStatus"); FillLine(TopPad, ' ');
	wattrset(TopPad, COLOR_PAIR(NORMAL));
	
	//Do drawing for printer grid
	int printerPos = 1;
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		int x = (i % PrinterCols) * (PrinterWidth+1);
		
		
		//wattrset(Pad, COLOR_PAIR(GREY));
		Border(Pad, x, printerPos-1, x+PrinterWidth+1, printerPos+PrinterHeight);
		//wattrset(Pad, COLOR_PAIR(NORMAL));
		
		PrinterList[i].Mutex->lock();
		PrinterList[i].Draw(this);
		PrinterList[i].Mutex->unlock();
		//wborder(PrinterList[i].Pad, '1','2','3','4','5','6','7','8');
		
		mvderwin(PrinterList[i].Pad, printerPos, x+1);
		if ((i+1)%PrinterCols == 0) { printerPos+=PrinterHeight+1; } 
	}
	
	
	
	/*move(1,1); 
	addch(ACS_ULCORNER); addch(ACS_URCORNER); addch(ACS_LLCORNER); addch(ACS_LRCORNER); addch(ACS_PLUS);
	move(1,1); BottomText+=std::to_string(inch()); BottomText+=":"; BottomText+=std::to_string(ACS_ULCORNER); BottomText+=" ";
	move(1,2); BottomText+=std::to_string(inch()); BottomText+=":"; BottomText+=std::to_string(ACS_URCORNER); BottomText+=" ";
	move(1,3); BottomText+=std::to_string(inch()); BottomText+=":"; BottomText+=std::to_string(ACS_LLCORNER); BottomText+=" ";
	move(1,4); BottomText+=std::to_string(inch()); BottomText+=":"; BottomText+=std::to_string(ACS_LRCORNER); BottomText+=" ";
	move(1,5); BottomText+=std::to_string(inch()); BottomText+=":"; BottomText+=std::to_string(ACS_PLUS); BottomText+=" ";*/
	
	//Bottom Text Panel
	wattrset(stdscr, COLOR_PAIR(0b111100));
	BottomText += "Printers:" + std::to_string(PrinterList.size()) + "  Width:" + std::to_string(Width) + "  Height:" + std::to_string(Height) + "  PrinterWidth:" + std::to_string(PrinterWidth) + "  PrinterCols:" + std::to_string(PrinterCols);
	mvaddstr(Height - 1, 0, BottomText.c_str()); FillLine(stdscr, ' ');
	BottomText = "";
	wattrset(stdscr, COLOR_PAIR(NORMAL));
	
	
	//Refresh display elements
	wnoutrefresh(stdscr);
	//touchwin(Pad);
	pnoutrefresh(Pad, ScrollY, ScrollX, 1, 0, Height-2, Width-1);
	pnoutrefresh(TopPad, 0, ScrollX,0,0,1,Width-1);

	//Draw update to screen. Doing this after reduces screen flicker
	doupdate();
}