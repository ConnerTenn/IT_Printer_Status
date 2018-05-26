
#include "Printer.h"

std::string Search(std::string str, std::string delim, int offset, int *i)
{
	std::string found;
	
	int s = offset, d = 0;
	bool any = false;
	bool copy = false;
	while (s < (int)str.size() && d < (int)delim.size())
	{
		if (delim[d] == '*')
		{
			any = true;
			d++;
		}
		else if (delim[d] == '@')
		{
			copy = true;
			d++;
		}
		else if (any)
		{
			if (str[s] == delim[d])
			{
				any = false;
			}
			else
			{
				s++;	
			}
		}
		else if (copy)
		{
			if (str[s] == delim[d])
			{
				copy = false;
			}
			else
			{
				found += str[s];
				s++;
			}
		}
		else if (str[s] == delim[d])
		{
			s++;
			d++;
		}
		else
		{
			if (d == 0) { s++; }
			else { d = 0; }
		}
	}
	
	if (d < (int)delim.size()) { return "-1"; }
	if (i) { *i = s; }
	return found; 
}

void Replace(std::string &str, std::string find, std::string replace)
{
	int s = 0, f = 0, r = 0;
	while (s < (int)str.size())
	{
		if (str[s] == find[f])
		{
			if (!r) { r = s; }
			s++;
			f++;
		}
		else
		{
			if (f == 0) { s++; }
			else { f = 0; }
			r = 0;
		}
		
		if (f >= (int)find.size())
		{
			str.replace(r, s-r, replace);
			s -= (s-r);
			f = 0;
			r = 0;
		}
	}
}



Printer::Printer() {}
Printer::Printer(std::string name) : Name(name) {}

std::string Printer::GetStatus()
{
	Status = "\"";
	int offset = 0;
	Status += Search(Html, "<td*class=\"statusLine\"*>*>@<", offset, &offset);
	Status += ", ";
	Status += Search(Html, "<td*class=\"statusLine\"*>*>@<", offset);
	
	Replace(Status, "&#032;&#032;", "");
	Replace(Status, "&#032;", " ");
	Status += "\"";
	
	
	if (Search(Html, "<table*bgcolor=\"@\"") == "#FFFF66")
	{
		StatusColour = 1;
	}
	
	return Status;
}

std::string Printer::GetUrlTopbar()
{
	return "br-" + Name + "-prn2.internal/cgi-bin/dynamic/topbar.html";
}

std::string Printer::GetUrlStatus()
{
	return "http://v4.ifconfig.co";
	return std::string("br-") + Name + "-prn2.internal/cgi-bin/printer/PrinterStatus.html";
}

size_t Printer::WriteCallback(void* buf, size_t size, size_t nmemb, void* userp)
{
	if(userp)
	{
		((std::string *)userp)->append((char *)buf, size * nmemb);
		return size * nmemb;
	}

	return 0;
}


int Printer::Update()
{
	/*CURL *curl = curl_easy_init();
	if(!curl) { return 0; }
	
	CURLcode res;
	curl_easy_setopt(curl, CURLOPT_URL, GetUrlStatus().c_str());
	//curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &Html);
	Html.clear();
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	
	return res;*/
	
#include <stdio.h>
	FILE *file = fopen("html1.html", "r");
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char *buff = new char[size + 1];
	fread(buff, size, 1, file);
	fclose(file);
	buff[size-1] = 0;
	Html = std::string(buff);
	//printf("%d\n", size);
	delete[] buff;
	
	
	GetStatus();
	
	
	return 0;
}

void Printer::Draw(Screen *screen, WINDOW *pad, bool selected)
{
	auto tab = [&](){ wattrset(pad, COLOR_PAIR(selected ? HIGHLIGHT : NORMAL)); waddstr(pad, "    "); wattrset(pad, COLOR_PAIR(NORMAL)); };
	//wattrset(pad, COLOR_PAIR(i==Cursor ? HIGHLIGHT : NORMAL));
	wattrset(pad, (selected ? A_BOLD : 0));
	wattrset(pad, (selected ? COLOR_PAIR(HIGHLIGHT) | A_BOLD | A_DIM : COLOR_PAIR(NORMAL)));
	waddstr(pad, Name.c_str());
	wattrset(pad, COLOR_PAIR(NORMAL));
	waddstr(pad, ("\t" + Status).c_str());  FillLine(pad, ' ');

	tab(); waddstr(pad, "Toner ["); for (int i=0;i<10;i++) {waddch(pad, i<5?ACS_CKBOARD:' ');} waddstr(pad, "]"); FillLine(pad, ' ');
	tab(); waddstr(pad, "Tray 1\tOK\t500\tLetter\tPlain Paper"); FillLine(pad, ' ');
	tab(); waddstr(pad, "Tray 2\tOK\t500\tLetter\tPlain Paper"); FillLine(pad, ' ');
	tab(); waddstr(pad, "Tray 3\tOK\t500\tLetter\tPlain Paper"); FillLine(pad, ' ');
	tab(); waddstr(pad, "Tray 4\tOK\t500\tLetter\tPlain Paper"); FillLine(pad, ' ');
}
	
	