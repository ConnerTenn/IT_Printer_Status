
struct Printer;

#ifndef _PRINTER_H_
#define _PRINTER_H_


#ifdef WINDOWS
#include <windows.h>
#undef MOUSE_MOVED
#elif LINUX
#include <unistd.h>
#endif

#include <fstream>
#include <string>
#include <curl/curl.h>
#ifdef WINDOWS
#include <curses.h>
#elif LINUX
#include <ncurses.h>
#endif
#include <thread>
#include <mutex>
#include "Screen.h"

extern std::vector<Printer> PrinterList; //Defined in Printer.cpp
const int PrinterHeight = 5; 
const int PrinterWidth = 300; 
extern Printer *Selected; //Defined in Printer.cpp
extern int MaxStatusLength; //Defined in Printer.cpp


void InitPrinters();
void SortPrinters();
std::string Search(std::string str, std::string delim, int offset = 0, int *i = 0);
void Replace(std::string &str, std::string find, std::string replace);
int First(std::string str, std::string first, std::string second, int offset = 0, int *i = 0);
std::string MinSize(std::string str, int size);
std::string MaxSize(std::string str, int size);


struct Tray
{
	std::string Name;
	std::string Status;
	int Capacity = 0;
	std::string PageSize;
	std::string PageType;
};

struct Kit
{
	std::string Name;
	int LifeRemaining = 0;
};

struct Printer
{
	std::string Name;
	std::string Model;
	std::string Address;
	std::string ContactName;;
	
	std::string MaintenanceKit;
	std::string PCKit;
	
	std::string Buffer;
	std::string HtmlTopBar;
	std::string HtmlStatus;
	std::string Status;
	
	char StatusColour = 0;
	bool Expanded = false;
	
	std::vector<Tray> TrayList;
	std::vector<Kit> KitList;
	
	int Toner = 0;
	
	WINDOW *Pad = 0;
	
	std::mutex *Mutex = 0;
	
	Printer();
	Printer(std::string name);
	~Printer();
	
	void GetStatus();
	
	std::string GetUrlTopbar();
	
	std::string GetUrlStatus();
	
	static size_t WriteCallback(void* buf, size_t size, size_t nmemb, void* userp);
	
	int Update();
	
	void Draw(Screen *screen);
};

#endif
