
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
	mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);

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
	
	//Get Nuber of columns and width of the printer tiles
	//PrinterCols = MAX(1, Width / (MinPrinterWidth+2));
	//PrinterWidth = MAX(Width / PrinterCols - 2, MinPrinterWidth);
	

	//FIX: This implementation works but is very slow

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
	
	Scroll();
}

void Screen::Draw()
{
	//Clear Pads
	clear();
	wclear(TopPad);
	wclear(Pad);
	
	//Top Text Panel
	wattrset(TopPad, A_BOLD | COLOR_PAIR(0b111010));
	waddstr(TopPad, " Name ");
	if (SortOrder == 0) { waddch(TopPad, ACS_UARROW); } else { waddch(TopPad, ' '); }
	waddstr(TopPad, "          Status ");
	if (SortOrder == 1) { waddch(TopPad, ACS_UARROW); } else { waddch(TopPad, ' '); }
	FillLine(TopPad, ' ');
	wattrset(TopPad, COLOR_PAIR(NORMAL));
	
	//Do drawing for printer grid
	int printerPos = 0;
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		//wattrset(Pad, COLOR_PAIR(GREY));
		//Border(Pad, 0, printerPos-1, PrinterWidth+1, printerPos+PrinterHeight);
		//wattrset(Pad, COLOR_PAIR(NORMAL));
		
		mvderwin(PrinterList[i]->Pad, printerPos, 0);
		
		PrinterList[i]->Mutex->lock();
		PrinterList[i]->Draw(this);
		PrinterList[i]->Mutex->unlock();
		//wborder(PrinterList[i].Pad, '1','2','3','4','5','6','7','8');
		
		printerPos+=(PrinterList[i]->Expanded ? PrinterHeight : 1) + 1;
		//if (PrinterList[i].Expanded) { exit(0); }
		
		if (i != (int)PrinterList.size() - 1)
		{
			wmove(Pad, printerPos - 1, 0);
			whline(Pad, ACS_HLINE, getmaxx(Pad));
		}
		
	}
	
	
	//Bottom Text Panel
	wattrset(stdscr, COLOR_PAIR(0b111100));
	wmove(stdscr, Height-1, 0);
	
	waddstr(stdscr, "Auto Scroll:");
	if (AutoScroll) { wattrset(stdscr, A_BOLD |  COLOR_PAIR(0b010100)); waddstr(stdscr, "ON "); }
	else { wattrset(stdscr, A_BOLD |  COLOR_PAIR(0b001100)); waddstr(stdscr, "OFF"); }
	
	wattrset(stdscr, COLOR_PAIR(0b111100));
	FillLine(stdscr, ' ');
	
	wmove(stdscr, Height-1, Width - 31);
	waddstr(stdscr, "Press H or I for Help and Info");
	
	wattrset(stdscr, COLOR_PAIR(NORMAL));
	
	
	//Scroll Bar
	{
		int maxY = 0;
		GetPrinterDisplayHeight(&maxY);
		
		int screenMin = ScrollY;
		int screenMax = ScrollY + Height-2;
		
		int barMin = (Height-2)*screenMin/MAX(maxY, Height-2);
		int barMax = (Height-2)*screenMax/MAX(maxY, Height-2);
		
		wmove(stdscr, 1, Width-1);
		wvline(stdscr, ACS_VLINE, Height-2);
		wmove(stdscr, barMin + 1, Width-1);
		wattrset(stdscr, COLOR_PAIR(0b111111));
		wvline(stdscr, ACS_CKBOARD, barMax - barMin);
		wattrset(stdscr, COLOR_PAIR(0b111111));
	}
	
	if (Popup)
	{
		wclear(PopupBorder);
		wclear(Popup);
		
		wattrset(PopupBorder, A_BOLD | COLOR_PAIR(0b111111));
		wborder(PopupBorder, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);
		
		wattrset(Popup, A_BOLD | A_UNDERLINE | COLOR_PAIR(NORMAL));
		FillLine(Popup, ' ');
		wmove(Popup, 0, getmaxx(Popup)/2-2);
		
		waddstr(Popup, "Info"); 
		
		wmove(Popup, 1, 0);
		wattrset(Popup, A_BOLD | COLOR_PAIR(NORMAL));
		waddstr(Popup, "Created By: "); 
		wattrset(Popup, COLOR_PAIR(NORMAL));
		waddstr(Popup, "Conner Tenn"); FillLine(Popup, ' ');
		
		wattrset(Popup, A_BOLD | COLOR_PAIR(NORMAL));
		waddstr(Popup, "Source Code availaible at:"); FillLine(Popup, ' ');
		wattrset(Popup, A_BOLD | COLOR_PAIR(0b110000));
		waddstr(Popup, "https://github.com/ConnerTenn/IT_Printer_Status"); FillLine(Popup, ' ');
		
		
		wmove(Popup, 7, 0);
		wattrset(Popup, A_BOLD | A_UNDERLINE | COLOR_PAIR(NORMAL));
		FillLine(Popup, ' ');
		wmove(Popup, 7, getmaxx(Popup)/2-5);
		waddstr(Popup, "Help Menu"); 
		
		wattrset(Popup, A_BOLD | COLOR_PAIR(NORMAL));
		wmove(Popup, 8, 0);
		waddstr(Popup, "Key Bindings:"); FillLine(Popup, ' ');
		waddstr(Popup, "A: ");
		wattrset(Popup, COLOR_PAIR(NORMAL));
		waddstr(Popup, "Toggle Auto Scroll\n");
		
		wattrset(Popup, A_BOLD | COLOR_PAIR(NORMAL));
		waddstr(Popup, "R: ");
		wattrset(Popup, COLOR_PAIR(NORMAL));
		waddstr(Popup, "Reload Printers From File and Reload\n");
		
		wattrset(Popup, A_BOLD | COLOR_PAIR(NORMAL));
		waddstr(Popup, "S: ");
		wattrset(Popup, COLOR_PAIR(NORMAL));
		waddstr(Popup, "Toggle Sort Order\n");

#ifdef WINDOWS
		wattrset(Popup, A_BOLD | COLOR_PAIR(NORMAL));
		waddstr(Popup, "M: ");
		wattrset(Popup, COLOR_PAIR(NORMAL));
		waddstr(Popup, "Maximize Screen Size\n");
#endif

		wattrset(Popup, A_BOLD | COLOR_PAIR(NORMAL));
		waddstr(Popup, "E: ");
		wattrset(Popup, COLOR_PAIR(NORMAL));
		waddstr(Popup, "Expand/Contract All\n");
		
		wattrset(Popup, A_BOLD | COLOR_PAIR(NORMAL));
		waddstr(Popup, "Enter: ");
		wattrset(Popup, COLOR_PAIR(NORMAL));
		waddstr(Popup, "Expand/Contract Selected Printer\n");
		
		wattrset(Popup, A_BOLD | COLOR_PAIR(NORMAL));
		waddstr(Popup, "Arrow Keys: ");
		wattrset(Popup, COLOR_PAIR(NORMAL));
		waddstr(Popup, "Move Cursor and Scroll Left/Right\n");
		
		wattrset(Popup, A_BOLD | COLOR_PAIR(NORMAL));
		waddstr(Popup, "Escape: ");
		wattrset(Popup, COLOR_PAIR(NORMAL));
		waddstr(Popup, "Close Program\n");
		
		wattrset(Popup, A_BOLD | COLOR_PAIR(NORMAL));
		waddstr(Popup, "H/I: ");
		wattrset(Popup, COLOR_PAIR(NORMAL));
		waddstr(Popup, "Toggle Help and Info Menu");
		
		
		
	}
	
	
	//Refresh display elements
	wnoutrefresh(stdscr);
	//touchwin(Pad);
	pnoutrefresh(Pad, ScrollY, ScrollX, 1, 0, Height-2, Width-2);
	pnoutrefresh(TopPad, 0, ScrollX,0,0,1,Width-1);
	
	if (Popup) { wnoutrefresh(PopupBorder); wnoutrefresh(Popup); }

	//Draw update to screen. Doing this after reduces screen flicker
	doupdate();
	
	if (AutoScroll)
	{
		if (AutoScrollDelay++ >= 10)
		{
			int maxY = 0;
			GetPrinterDisplayHeight(&maxY);
			
			if (ScrollY + Height - 2 >= maxY)
			{
				ScrollY = 0;
			}
			else
			{
				ScrollY += MIN((int)(Height * 0.75), maxY-(ScrollY+Height-2));
			}
			AutoScrollDelay = 0;
		}
	}
	else 
	{
		AutoScrollDelay = 0;
	}
}

void Screen::Scroll()
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
}
