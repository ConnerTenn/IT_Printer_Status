
#include "Screen.h"

void Screen::Draw()
{
	SortPrinters();
	
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
	UpdatePrinterColumns();
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
	if (RefreshingPrinters) { waddstr(stdscr, "     Refreshing Printers"); }
	
	wattrset(stdscr, COLOR_PAIR(0b111100));
	FillLine(stdscr, ' ');
	
	wmove(stdscr, Height-1, Width - 31);
	waddstr(stdscr, "Press H or I for Help and Info");
	
	wattrset(stdscr, COLOR_PAIR(NORMAL));
	
	
	
	if (Popup)
	{
		DrawInfoMenu();	
	}
	
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
	
	
	//Refresh display elements
	wnoutrefresh(stdscr);
	//touchwin(Pad);

	pnoutrefresh(Pad, ScrollY, ScrollX, 1, 0, Height-2, Width-2);
	pnoutrefresh(TopPad, 0, ScrollX,0,0,1,Width-1);
	
	if (Popup) { wnoutrefresh(PopupBorder); wnoutrefresh(Popup); }

	//Draw update to screen. Doing this after reduces screen flicker
	doupdate();
	
}

void Screen::DrawInfoMenu()
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