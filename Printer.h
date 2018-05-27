
struct Printer;

#ifndef _PRINTER_H_
#define _PRINTER_H_

#include <string>
#include <curl/curl.h>
#include <ncurses.h>
#include "Screen.h"

std::string Search(std::string str, std::string delim, int offset = 0, int *i = 0);
void Replace(std::string &str, std::string find, std::string replace);

const int PrinterHeight = 5;
const int MinPrinterWidth = 77;
const int MaxPrinterWidth = 80;
extern int PrinterWidth;
extern int PrinterCols;


struct Tray
{
	std::string Name;
	std::string Status;
	int Capacity = 0;
	std::string PageSize;
	std::string PageType;
};

struct Printer
{
	std::string Name;
	std::string HtmlTopBar;
	std::string HtmlStatus;
	std::string Status;
	char StatusColour = 0;
	
	std::vector<Tray> TrayList;
	
	int Toner = 0;
	
	WINDOW *Pad = 0;
	
	Printer();
	Printer(std::string name);
	
	void GetStatus();
	
	std::string GetUrlTopbar();
	
	std::string GetUrlStatus();
	
	static size_t WriteCallback(void* buf, size_t size, size_t nmemb, void* userp);
	
	int Update();
	
	void Draw(Screen *screen);
};

#endif
