
#include "Screen.h"

Screen *Screen::This = 0;

void FillLine(int chr)
{
	addstr(std::string(getmaxx(stdscr)-getcurx(stdscr), chr).c_str());
}

void FillLine(WINDOW *win, int chr)
{
	waddstr(win, std::string(getmaxx(win)-getcurx(win), chr).c_str());
}

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
	mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
	mouseinterval(10);

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
	resize_term(50, 200);
	Resize();

#ifdef WINDOWS
	//signal(SIGWINCH, Resize);
#endif
}

Screen::~Screen()
{	
	delwin(TopPad); TopPad = 0;
	delwin(Pad); Pad = 0;
	if (Popup) { delwin(Popup); Popup = 0; delwin(PopupBorder); PopupBorder = 0;}
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
	

	//Recreate Pad
	if (Pad) { delwin(Pad); }
	Pad = newpad(MAX(((int)PrinterList.size() + 1)*(PrinterHeight+1) + 1, Height-2), MAX(PrinterWidth+2, Width+2));
	
	//Recreate Pads for each printer
	for (int i = 0; Pad && i < (int)PrinterList.size(); i++)
	{
		if (PrinterList[i]->Pad) { delwin(PrinterList[i]->Pad); }
		PrinterList[i]->Pad = subpad(Pad, PrinterHeight, PrinterWidth, 0, 0);
	}
	
	if (Popup)
	{
		delwin(Popup);
		delwin(PopupBorder);
		PopupBorder = subwin(stdscr, Height / 2 + 2, Width / 2 + 2, Height / 4 - 1, Width / 4 - 1);
		Popup = subwin(stdscr, Height / 2, Width / 2, Height / 4, Width / 4);
	}
	
	UpdateScroll();
}



void Screen::UpdateScroll()
{
	int maxY = 0;
	int SelectMaxY = 0;
	int SelectMinY = 0;
	
	GetPrinterDisplayHeight(&maxY, &SelectMinY, &SelectMaxY, Cursor);
	SelectMaxY -= ScrollY;
	SelectMinY -= ScrollY;
	
	if (SelectMaxY > Height-2) { ScrollY += SelectMaxY - (Height-2); }
	if (SelectMinY < 0) { ScrollY += SelectMinY; }
	if (maxY < Height-2) { ScrollY = 0; }
	if (ScrollY > maxY - (Height-2)) { ScrollY = MAX(maxY - (Height-2), 0); }
}

