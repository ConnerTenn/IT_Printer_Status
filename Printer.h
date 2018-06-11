
struct Printer;

#ifndef _PRINTER_H_
#define _PRINTER_H_


#ifdef WINDOWS
#include <windows.h>
#elif LINUX
#include <unistd.h>
#endif

#include <fstream>
#include <string>
#include <curl/curl.h>
#ifdef WINDOWS
#define NCURSES_MOUSE_VERSION
#undef MOUSE_MOVED
#include <curses.h>
#elif LINUX
#include <ncurses.h>
#endif
#include <mutex>
#include "Screen.h"

const int PrinterHeight = 5; 
const int PrinterWidth = 300; 

extern std::vector<Printer *> PrinterList; 
extern std::vector<Printer *> PrinterUpdateThreadList; 
extern Printer *Selected; 
extern int SortOrder; 

extern int PrinterColumns[10];
extern bool RefreshingPrinters;

extern int MinStatusLength; 
extern int MaxStatusLength; 
extern int NetworkTimeout;

extern std::string URLTopbar;
extern std::string URLStatus;
extern std::mutex URLMutex;

void InitPrinters();
void DestroyPrinters();
void SortPrinters();

//Get the height of the printers based on if they're expanded or not
void GetPrinterDisplayHeight(int *maxY, int *indexMinY = 0, int *indexMaxY = 0, int index = -1);
//Printer *GetPrinterAtHeight()
void UpdatePrinterColumns();

std::string Search(std::string str, std::string delim, int offset = 0, int *i = 0);
void Replace(std::string &str, std::string find, std::string replace);
int First(std::string str, std::string first, std::string second, int offset = 0, int *i = 0);
std::string MinSize(std::string str, int size, char fill = ' ');
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

struct Toner
{
	int Percent = 0;
	short Colour = 0;
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
	
	std::vector<Toner> TonerList;
	std::vector<Tray> TrayList;
	std::vector<Kit> KitList;
	
	WINDOW *Pad = 0;
	
	std::mutex *Mutex = 0;
	
	Printer();
	Printer(std::string name);
	~Printer();
	
	void GetToner();
	void GetTrays();
	void GetKits();
	void GetStatus();
	
	std::string GetUrlTopbar();
	
	std::string GetUrlStatus();
	
	static size_t WriteCallback(void* buf, size_t size, size_t nmemb, void* userp);
	
	int Update();
	
	void Draw(Screen *screen);
};

#endif
