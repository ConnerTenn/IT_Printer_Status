

#ifndef _PRINTER_H_
#define _PRINTER_H_

#include <string>
#include <curl/curl.h>

std::string Search(std::string str, std::string delim, int offset = 0, int *i = 0);
void Replace(std::string &str, std::string find, std::string replace);

struct Printer
{
	std::string Name;
	std::string Html;
	std::string Status;
	char StatusColour = 0;
	
	Printer();
	Printer(std::string name);
	
	std::string GetStatus();
	
	std::string GetUrlTopbar();
	
	std::string GetUrlStatus();
	
	static size_t WriteCallback(void* buf, size_t size, size_t nmemb, void* userp);

	
	int Update();
};

#endif
