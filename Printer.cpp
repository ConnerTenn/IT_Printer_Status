
#include "Printer.h"

int PrinterHeight = 5;
int PrinterWidth = 50;
int PrinterCols = 1;
int DisplayStyle = 1;

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
	int s = 0, f = 0, r = -1;
	while (s < (int)str.size())
	{
		if (str[s] == find[f])
		{
			if (r<0) { r = s; }
			s++;
			f++;
		}
		else
		{
			if (f == 0) { s++; }
			else { f = 0; }
			r = -1;
		}
		
		if (f >= (int)find.size())
		{
			str.replace(r, s-r, replace);
			s -= (s-r);
			f = 0;
			r = -1;
		}
	}
}

bool First(std::string str, std::string first, std::string second, int offset, int *i)
{
	int s = 0, f1 = 0, f2 = 0;
	bool incr = true;
	
	while (s < (int)str.size())
	{
		incr = true;
		
		if (str[s] == first[f2])
		{
			//incr = true && incr;
			f1++;
		}
		else
		{
			if (f1 != 0) { f1 = 0; incr = false; }
		}
		
		if (str[s] == second[f2])
		{
			//incr = true && incr;
			f2++;
		}
		else
		{
			if (f2 != 0) { f2 = 0; incr = false; }
		}
		
		if (f1 >= (int)first.size())
		{
			*i = s;
			return true;
		}
		if (f2 >= (int)second.size())
		{
			*i = s;
			return true;
		}
		
		if (incr) { s++; }
	}
	
	return -1;
}


Printer::Printer() : Printer("") {}
Printer::Printer(std::string name) : Name(name) {}

Printer::~Printer()
{
	if (Mutex) { delete Mutex; Mutex = 0; }
	
	if (Pad) { delwin(Pad); }
}

void Printer::GetStatus()
{
	{
		Buffer = "\"";
		int offset = 0;
		Buffer += Search(HtmlTopBar, "<td*class=\"statusLine\"*>*>@<", offset, &offset);
		Buffer += ", ";
		Buffer += Search(HtmlTopBar, "<td*class=\"statusLine\"*>*>@<", offset);
		
		Replace(Buffer, "&#032;&#032;", "");
		Replace(Buffer, "&#032;", " ");
		Buffer += "\"";
		
		if (Search(HtmlTopBar, "<table*bgcolor=\"@\"") == "#FFFFFF")
		{
			StatusColour = 0b111000;
		}
		else if (Search(HtmlTopBar, "<table*bgcolor=\"@\"") == "#FFFF66")
		{
			StatusColour = 0b011000;
		}
	}
	
	Mutex->lock();
	Status = Buffer;
	
	{
		int offset = 0;
		
		
		Toner = stoi(Search(HtmlStatus, "Black*~@%"));
		
		TrayList.clear();
		std::string in;
		
		bool doTray = true;
		while (doTray)
		{
			Search(HtmlStatus, "<TR>*<TD>", offset, &offset);
			in = Search(HtmlStatus, "<P style=\"margin-left:5\">@<", offset, &offset);
			if (in!="-1" && in.size())
			{
				TrayList.push_back(Tray());
				//Replace(in, "Multi-Purpose ", "");
				TrayList.back().Name = in;
				
				
				in = Search(HtmlStatus, "<tr><td><b>@<", offset, &offset);
				TrayList.back().Status = in;
				
				
				if (First(HtmlStatus, "<TD>", "<TR>", offset) == true)
				{
					in = Search(HtmlStatus, "<TD><P style=\"margin-left:5\">@<", offset, &offset);
					TrayList.back().Capacity = stoi(in);
					
					if (First(HtmlStatus, "<TD>", "<TR>", offset) == true)
					{
						in = Search(HtmlStatus, "<TD><P style=\"margin-left:5\">@<", offset, &offset);
						in += std::string(MAX(6-(int)in.size(), 0), ' ');
						TrayList.back().PageSize = in;
						
						if (First(HtmlStatus, "<TD>", "<TR>", offset) == true)
						{
							in = Search(HtmlStatus, "<TD><P style=\"margin-left:5\">@<", offset, &offset);
							Replace(in, "&nbsp;", " ");
							TrayList.back().PageType = in;
						}
					}
				}
				
				
				
			}
			else
			{
				doTray = false;
			}
			
		}
		
		Mutex->unlock();
	}
}

std::string Printer::GetUrlTopbar()
{
	//return "http://v4.ifconfig.co";
	return "br-" + Name + ".internal/cgi-bin/dynamic/topbar.html";
}

std::string Printer::GetUrlStatus()
{
	//return "http://v4.ifconfig.co";
	return "br-" + Name + ".internal/cgi-bin/dynamic/printer/PrinterStatus.html";
}

size_t Printer::WriteCallback(void* buf, size_t size, size_t nmemb, void* userp)
{
	if(userp)
	{
		((Printer *)userp)->Mutex->lock();
		((Printer *)userp)->Buffer.append((char *)buf, size * nmemb);
		((Printer *)userp)->Mutex->unlock();
		
		return size * nmemb;
	}

	return 0;
}


int Printer::Update()
{
#ifndef DEBUG
	CURL *curl = curl_easy_init();
	if(!curl) { return 0; }
	CURLcode res = CURLE_OK;
	
	curl_easy_setopt(curl, CURLOPT_URL, GetUrlTopbar().c_str());
	//curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);
	Buffer.clear();
	//if (Search(HtmlTopBar, "<html class=\"top_bar\">") == "-1") { res = CURLE_RECV_ERROR; }
	res = curl_easy_perform(curl);
	HtmlTopBar = Buffer;
	
	if (res != CURLE_OK) { Status = "Network Error:" + std::to_string(res); StatusColour = 0b001000; }
	
	curl_easy_setopt(curl, CURLOPT_URL, GetUrlStatus().c_str());
	//curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);
	Buffer.clear();
	res = (res?res:curl_easy_perform(curl));
	//if (Search(HtmlStatus, "<title>Printer Status</title>") == "-1") { res = CURLE_RECV_ERROR; }
	if (res != CURLE_OK) { Status = "Network Error:" + std::to_string(res); StatusColour = 0b001000; }
	HtmlStatus = Buffer;

	
	curl_easy_cleanup(curl);
	
	if (res == CURLE_OK) { GetStatus(); }
	
	return res;

#else
	
#include <stdio.h>

	int res = 0;
	
	FILE *file = fopen((Name + "-topbar.html").c_str(), "r");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		long size = ftell(file);
		fseek(file, 0, SEEK_SET);
		
		char *buff = new char[size + 1];
		fread(buff, size, 1, file);
		fclose(file);
		buff[size-1] = 0;
		HtmlTopBar = std::string(buff);
		//printf("%d\n", size);
		delete[] buff;
		
		res = 1;
	}
	
	
	if (res) { file = fopen((Name + "-status.html").c_str(), "r"); }
	if (file)
	{
		fseek(file, 0, SEEK_END);
		long size = ftell(file);
		fseek(file, 0, SEEK_SET);
		
		char *buff = new char[size + 1];
		fread(buff, size, 1, file);
		fclose(file);
		buff[size-1] = 0;
		HtmlStatus = std::string(buff);
		//printf("%d\n", size);
		delete[] buff;
	}
		
	if (!res)
	{
		Status = "File Error"; StatusColour = 0b001000;
	}
	else
	{
		GetStatus();
	}
	
	return 0;
#endif
}

void Printer::Draw(Screen *screen)
{
	wclear(Pad);
	
	if (DisplayStyle == 1)
	{
		Draw1(screen);
	}
	else if (DisplayStyle == 2)
	{
		Draw2(screen);
	}
}

void Printer::Draw1(Screen *screen)
{
	wattrset(Pad, A_BOLD | COLOR_PAIR(0b110000));
	waddstr(Pad, " [");
	waddstr(Pad, (Name).c_str());
	waddstr(Pad, "]");
	wattrset(Pad, COLOR_PAIR(NORMAL));
	waddstr(Pad, "  ");
	
	wattrset(Pad, A_BOLD | COLOR_PAIR(StatusColour));
	waddstr(Pad, (Status).c_str()); 
	wattrset(Pad, COLOR_PAIR(NORMAL));
	FillLine(Pad, ' '); 
	//wattrset(pad, COLOR_PAIR(NORMAL));
	
	if (Status.size() && Status != "Network Error")
	{
		waddstr(Pad, "Toner ["); 
		if (Toner <= 20) { wattrset(Pad, A_BOLD | COLOR_PAIR(0b001000)); } else { wattrset(Pad, A_BOLD | COLOR_PAIR(0b111000)); }
		for (int i=0;i<10;i++) { waddch(Pad, i<Toner/10?ACS_CKBOARD:' '); }
		wattrset(Pad, COLOR_PAIR(NORMAL));
		waddstr(Pad, "]"); 
		if (Toner <= 20) { wattrset(Pad, A_BOLD | COLOR_PAIR(0b001000)); } else { wattrset(Pad, A_BOLD | COLOR_PAIR(0b111000)); }
		waddstr(Pad, (std::to_string(Toner) + "%").c_str()); 
		wattrset(Pad, COLOR_PAIR(NORMAL));
		FillLine(Pad, ' ');
		
		for (int i = 0; i < (int)TrayList.size(); i++)
		{
			const int columnOffset = 39;
			int len = TrayList.size() - 2;
			int x=0, y=0;
			if (TrayList[i].Name == "Feeder")
			{
				y = (len-len%2)/2;
			}
			else if (TrayList[i].Name == "Bin   ")
			{
				x = columnOffset;
				y = (len-len%2)/2;
			}
			else
			{
				x = (i >= (len-len%2)/2) * columnOffset;
				y = (i % ((len-len%2)/2));
			}
			//int x = ((i >= (len-len%2)/2 && i<len-1) || ((i==len-1) && (i%2))) * 42;
			//int y = i % ((len-len%2)/2) + (i==len-1 && !(i%2)) * (len-len%2)/2;
			//int x=0, y=i;
			wmove(Pad, y+2, x);
			
			waddstr(Pad, (TrayList[i].Name + "  ").c_str());
			
			if (TrayList[i].Status == "OK") { wattrset(Pad, COLOR_PAIR(0b010000)); }
			if (TrayList[i].Status == "Low") { wattrset(Pad, A_BOLD | COLOR_PAIR(0b011000)); }
			if (TrayList[i].Status == "Empty") { wattrset(Pad, A_BOLD | COLOR_PAIR(0b001000)); }
			waddstr(Pad, (TrayList[i].Status + std::string(5-TrayList[i].Status.size(), ' ')).c_str());
			wattrset(Pad, COLOR_PAIR(NORMAL));
			waddstr(Pad, ("," + std::to_string(TrayList[i].Capacity) + "," + TrayList[i].PageSize + "," + TrayList[i].PageType).c_str()); FillLine(Pad, ' ');
		}
	}
}

void Printer::Draw2(Screen *screen)
{
	wattrset(Pad, A_BOLD | COLOR_PAIR(0b110000));
	waddstr(Pad, " [");
	waddstr(Pad, (Name).c_str());
	waddstr(Pad, "]");
	wattrset(Pad, COLOR_PAIR(NORMAL));
	waddstr(Pad, "  ");
	
	wattrset(Pad, A_BOLD | COLOR_PAIR(StatusColour));
	waddstr(Pad, (Status).c_str()); 
	wattrset(Pad, COLOR_PAIR(NORMAL));
	waddstr(Pad, "  "); 
	
	wmove(Pad, 0, 50);
	for (int i = 0; i < (int)TrayList.size(); i++)
	{
		waddstr(Pad, (TrayList[i].Name + "  ").c_str());
			
			if (TrayList[i].Status == "OK") { wattrset(Pad, COLOR_PAIR(0b010000)); }
			if (TrayList[i].Status == "Low") { wattrset(Pad, A_BOLD | COLOR_PAIR(0b011000)); }
			if (TrayList[i].Status == "Empty") { wattrset(Pad, A_BOLD | COLOR_PAIR(0b001000)); }
			waddstr(Pad, (TrayList[i].Status + std::string(6-TrayList[i].Status.size(), ' ')).c_str());
			wattrset(Pad, COLOR_PAIR(NORMAL));
	}
}	
	