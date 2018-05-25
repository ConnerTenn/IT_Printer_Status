
#include <iostream>
#include <ncurses.h>
#include <unistd.h>
#include "Printer.h"

int main()
{
	//std::cout << "Hello World\n";
	/*initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	start_color();
	//nodelay(stdscr, TRUE);
	init_pair(1, COLOR_BLACK, COLOR_RED);
    init_pair(2, COLOR_BLACK, COLOR_GREEN);
	
	//WINDOW * win = newwin(50, 300, 0, 0);
	refresh();
	
	for (int i = 0; i < 1000; i++)
	{
		addch(i); 
		refresh();
		attron(COLOR_PAIR(i%2+1));
	}
	
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
	
	addch(ACS_ULCORNER);
	
	int ch = getch();
	
	//delwin(win);
	
	endwin();*/
	
	Printer print;
	
	print.Name = "c238";
	std::cout << print.Update() << "\n";
	std::cout << print.Html << "\n";
	
	return 0;
}
