

#ifndef _PRINTER_H_
#define _PRINTER_H_

#include <string>
#include <curl/curl.h>

std::string Search(std::string delim);

struct Printer
{
	std::string Name;
	std::string Html;
	
	Printer();
	Printer(std::string name);
	
	std::string GetStatus();
	
	std::string GetUrlTopbar();
	
	std::string GetUrlStatus();
	
	static size_t WriteCallback(void* buf, size_t size, size_t nmemb, void* userp);

	
	int Update();
};

#endif
