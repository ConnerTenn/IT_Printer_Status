
#include "Printer.h"


std::vector<Printer *> PrinterList;
Printer *Selected = 0;
int MaxStatusLength = 0;

void InitPrinters()
{
	DestroyPrinters();
	
	std::ifstream file("Printers.txt");
	std::string line;
	
	while(std::getline(file, line))
	{
		if (line.size())
		{
			PrinterList.push_back(new Printer(line));
		}
	}
	
	//mutexes created after adding printers to list to handle how std::vector copying and deleting Printer object issues
	/*for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		PrinterList[i].Mutex = new std::mutex;
	}*/
}

void DestroyPrinters()
{
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		if (PrinterList[i]) { delete PrinterList[i]; }
	}
	
	PrinterList.clear();
}

void SortPrinters()
{
	
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		PrinterList[i]->Mutex->lock();
	}
	
	auto sortFunc = [](Printer *a, Printer *b)->int{ return 1; };
	
	int sorted = 0;
	
	while (sorted < (int)PrinterList.size() - 1)
	{
		int maxima = 0;
		for (int i = sorted; i < (int)PrinterList.size(); i++)
		{
			if (sortFunc(PrinterList[i], PrinterList[maxima]) > 0)
			{
				maxima = i;
			}
		}
		
		Printer *temp = PrinterList[sorted];
		PrinterList[sorted] = PrinterList[maxima];
		PrinterList[maxima] = temp;
		
		sorted++;
	}
	
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		PrinterList[i]->Mutex->unlock();
	}
}

void GetPrinterDisplayHeight(int *maxY, int *cursorMinY, int *cursorMaxY, int index)
{
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		if (i <= index)
		{
			if (cursorMaxY) { *cursorMaxY += (PrinterList[i]->Expanded ? PrinterHeight : 1) + (i < (int)PrinterList.size() - 1 ? 1 : 0); }
			if (cursorMinY && i-1 >= 0) { *cursorMinY += (PrinterList[i-1]->Expanded ? PrinterHeight : 1) + (i < (int)PrinterList.size() - 1 ? 1 : 0); }
		}
		if (maxY) { *maxY += (PrinterList[i]->Expanded ? PrinterHeight : 1) + (i < (int)PrinterList.size() - 1 ? 1 : 0); }
	}
}

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

int First(std::string str, std::string first, std::string second, int offset, int *i)
{	
	size_t s1 = str.find(first, offset);
	size_t s2 = str.find(second, offset);
	
	if (s1 == std::string::npos && s2 == std::string::npos) { return -1; }
	
	if (i) { *i = MIN(s1, s2); }
	return (s1 <= s2 ? 1 : 0);
}

std::string MinSize(std::string str, int size)
{
	return str + std::string(MAX(size-(int)str.size(), 0), ' ');
}

std::string MaxSize(std::string str, int size)
{
	return str.substr(0, MIN(size, (int)str.size()));
}




Printer::Printer() : Printer("") {}
Printer::Printer(std::string name) : Name(name) { Mutex = new std::mutex; }

Printer::~Printer()
{
	if (Mutex) { delete Mutex; Mutex = 0; }
	
	if (Pad) { delwin(Pad); }
}

void Printer::GetStatus()
{
	{
		Buffer = "";
		int offset = 0;
		Buffer += Search(HtmlTopBar, "<td*class=\"statusLine\"*>*>@<", offset, &offset);
		Replace(Buffer, "&#032;&#032;", "");
		Replace(Buffer, "&#032;", " ");
		Buffer = MinSize(Buffer, 13);
		
		Buffer += Search(HtmlTopBar, "<td*class=\"statusLine\"*>*>@<", offset);
		Replace(Buffer, "&#032;&#032;", "");
		Replace(Buffer, "&#032;", " ");
		//Buffer += "";
		
		std::string colour = Search(HtmlTopBar, "<table*bgcolor=\"@\"");
		if (colour == "#F8F8F8")
		{
			StatusColour = 0b010000;
		}
		else if (colour == "#FFFF66")
		{
			StatusColour = 0b011000;
		}
	}
	
	{
		int offset = 0;
		Model = Search(HtmlTopBar, "<span class=\"top_prodname\">@<", offset, &offset);
		Address = Search(HtmlTopBar, "Address: @<", offset, &offset);
		ContactName = Search(HtmlTopBar, "Contact Name: @<", offset, &offset);
	}
	
	Mutex->lock();
	Status = Buffer;
	
	{
		int offset = 0;
		std::string in;
		
		Toner = stoi(Search(HtmlStatus, "Black*~@%"));
		
		TrayList.clear();
		
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
				
				
				if (First(HtmlStatus, "<TD>", "<TR>", offset) == 1)
				{
					in = Search(HtmlStatus, "<TD><P style=\"margin-left:5\">@<", offset, &offset);
					TrayList.back().Capacity = stoi(in);
					
					if (First(HtmlStatus, "<TD>", "<TR>", offset) == 1)
					{
						in = Search(HtmlStatus, "<TD><P style=\"margin-left:5\">@<", offset, &offset);
						in = MinSize(in, 6);
						TrayList.back().PageSize = in;
						
						if (First(HtmlStatus, "<TD>", "<TR>", offset) == 1)
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
		
		{
			int offset = 0;
			std::string in;
			
			KitList.clear();
			
			Search(HtmlStatus, "Toner Cartridge Capacity", offset, &offset);
			
			bool doKit = true;
			while (doKit)
			{
				if (First(HtmlStatus, "<TD><B>", "</table>", offset) == 1)
				{
					in = Search(HtmlStatus, "<TD><B>@<", offset, &offset);
					Replace(in, " Life Remaining", "");
					KitList.push_back(Kit());
					
					KitList.back().Name = in;
					
					in = Search(HtmlStatus, "</TD><TD>@%", offset, &offset);
					KitList.back().LifeRemaining = stoi(in);
				}
				else
				{
					doKit = false;
				}
			}
		}
		
		if (Toner == 0 && StatusColour == 0b111000 ) { StatusColour = 0b011000; }
		
		Mutex->unlock();
		
		MaxStatusLength = MAX(MaxStatusLength, (int)Status.size());
	}
}

std::string Printer::GetUrlTopbar()
{
	//return "http://v4.ifconfig.co";
	return Name + ".internal/cgi-bin/dynamic/topbar.html";
}

std::string Printer::GetUrlStatus()
{
	//return "http://v4.ifconfig.co";
	return Name + ".internal/cgi-bin/dynamic/printer/PrinterStatus.html";
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
	res = curl_easy_perform(curl);
	HtmlTopBar = Buffer;
	if (HtmlTopBar.find("<html class=\"top_bar\">") == std::string::npos) { res = CURLE_RECV_ERROR; }
	if (res != CURLE_OK) { Status = "Network Error:" + std::to_string(res); StatusColour = 0b001000; }
	
	curl_easy_setopt(curl, CURLOPT_URL, GetUrlStatus().c_str());
	//curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);
	Buffer.clear();
	res = (res?res:curl_easy_perform(curl));
	if (HtmlTopBar.find("<html class=\"top_bar\">") == std::string::npos) { res = CURLE_RECV_ERROR; }
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
		
		if (Search(HtmlTopBar, "<html class=\"top_bar\">") == "-1") { res = 0; }
		
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
		
		if (Search(HtmlStatus, "<title>Printer Status</title>") == "-1") { res = 0; }
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
	int TonerStart = MaxStatusLength + 19;
	int TrayStart = TonerStart + 26;
	
	wclear(Pad);
	
	if (Selected == this) { wattrset(Pad, A_BOLD | A_REVERSE | COLOR_PAIR(0b110000)); } else { wattrset(Pad, COLOR_PAIR(NORMAL)); }
	if (Expanded) { waddch(Pad, ACS_ULCORNER); } else { waddch(Pad, ACS_HLINE ); }
	
	if (Selected == this) { wattrset(Pad, A_BOLD | A_REVERSE | COLOR_PAIR(0b110000)); } else { wattrset(Pad, A_BOLD | COLOR_PAIR(0b110000)); }
	waddstr(Pad, ("[" + MinSize(Name, 12) + "]").c_str());
	waddstr(Pad, "  ");
	

	//if (Status.size() > StatusLen) { Status = MaxSize(Status, StatusLen-3) + "..."; }
	wattrset(Pad, A_BOLD | COLOR_PAIR(StatusColour));
	waddstr(Pad, (Status).c_str()); 
	//wattrset(Pad, COLOR_PAIR(NORMAL));
	waddstr(Pad, "  "); 
	wattrset(Pad, COLOR_PAIR(NORMAL));
	
	wmove(Pad, 0, TonerStart);
	
	bool noError = Status.size() && Status.find("File Error") == std::string::npos && Status.find("Network Error") == std::string::npos;
	
	if (noError)
	{
		if (Toner == 0) { wattrset(Pad, A_BOLD | COLOR_PAIR(0b001000)); } else if (Toner <= 20) { wattrset(Pad, A_BOLD | COLOR_PAIR(0b011000)); } else { wattrset(Pad, A_BOLD | COLOR_PAIR(0b111000)); }
		waddstr(Pad, "Toner ["); 
		for (int i=0;i<10;i++) { waddch(Pad, i<Toner/10?ACS_CKBOARD:' '); }
		waddstr(Pad, "]");
		waddstr(Pad, MinSize("~" + std::to_string(Toner) + "%", 6).c_str()); 
		
		
		wattrset(Pad, (Selected == this ? A_BOLD : 0) | COLOR_PAIR(NORMAL));
			
		wmove(Pad, 0, TrayStart);
		for (int i = 0; i < (int)TrayList.size(); i++)
		{
			
			waddstr(Pad, (TrayList[i].Name + "  ").c_str());
				
			if (TrayList[i].Status == "OK") { wattrset(Pad, COLOR_PAIR(0b010000)); }
			else if (TrayList[i].Status == "Low") { wattrset(Pad, A_BOLD | COLOR_PAIR(0b011000)); }
			else /*if (TrayList[i].Status == "Empty")*/ { wattrset(Pad, A_BOLD | COLOR_PAIR(0b001000)); }
			waddstr(Pad, MinSize(TrayList[i].Status, 6).c_str());
			wattrset(Pad, (Selected == this ? A_BOLD : 0) | COLOR_PAIR(NORMAL));
		}
	}
	
	
	if (Expanded)
	{
		wmove(Pad, 1, 0);
		if (Selected == this) { wattrset(Pad, A_BOLD | A_REVERSE | COLOR_PAIR(0b110000)); }
		wvline(Pad, ACS_VLINE, 4);
		wattrset(Pad, (Selected == this ? A_BOLD : 0) | COLOR_PAIR(NORMAL));
			
		if (noError)
		{
			wmove(Pad, 1, 1);
			waddstr(Pad, ("Model:       "+Model).c_str());
			
			wmove(Pad, 2, 1);
			waddstr(Pad, ("Address:     "+Address).c_str());
			
			wmove(Pad, 3, 1);
			waddstr(Pad, ("ContactName: "+ContactName).c_str());
			
			for (int i = 0; i < (int)KitList.size(); i++)
			{
				wmove(Pad, i+1, 34);
				waddstr(Pad, (MinSize(KitList[i].Name, 17) + std::to_string( KitList[i].LifeRemaining) + "%").c_str());
			}
			
			
			for (int i = 0, x = 0; i < (int)TrayList.size(); i++)
			{
				wmove(Pad, 1, TrayStart + x);
				waddstr(Pad, std::to_string(TrayList[i].Capacity).c_str());
				
				wmove(Pad, 2, TrayStart + x);
				waddstr(Pad, TrayList[i].PageSize.c_str());
				
				wmove(Pad, 3, TrayStart + x);
				waddstr(Pad, TrayList[i].PageType.c_str());
				
				x+= TrayList[i].Name.size() + 6 + 2;
			}
		}
	}
}	
	