
#include "Printer.h"

int PrinterWidth = 50;
int PrinterCols = 1;

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



Printer::Printer() {}
Printer::Printer(std::string name) : Name(name) {}

void Printer::GetStatus()
{
	{
		Status = "\"";
		int offset = 0;
		Status += Search(HtmlTopBar, "<td*class=\"statusLine\"*>*>@<", offset, &offset);
		Status += ", ";
		Status += Search(HtmlTopBar, "<td*class=\"statusLine\"*>*>@<", offset);
		
		Replace(Status, "&#032;&#032;", "");
		Replace(Status, "&#032;", " ");
		Status += "\"";
		
		if (Search(HtmlTopBar, "<table*bgcolor=\"@\"") == "#FFFF66")
		{
			StatusColour = 0b011000;
		}
	}
	
	{
		int offset = 0;
		
		
		Toner = stoi(Search(HtmlStatus, "Toner  ~@%"));
		
		TrayList.clear();
		std::string in;
		bool doTray = true;
		while (doTray)
		{
			in = Search(HtmlStatus, "<TD><P style=\"margin-left:5\">@<", offset, &offset);
			if (in!="-1" && in.size() && in != "Standard Bin")
			{
				TrayList.push_back(Tray());
				Replace(in, "Multi-Purpose ", "");
				TrayList.back().Name = in;
				
				in = Search(HtmlStatus, "height=\"1\"><tr><td><b>@<", offset, &offset);
				in += std::string(5-in.size(), ' ');
				TrayList.back().Status = in;
				
				in = Search(HtmlStatus, "<TD><P style=\"margin-left:5\">@<", offset, &offset);
				TrayList.back().Capacity = stoi(in);
				
				in = Search(HtmlStatus, "<TD><P style=\"margin-left:5\">@<", offset, &offset);
				in += std::string(6-in.size(), ' ');
				TrayList.back().PageSize = in;
				
				in = Search(HtmlStatus, "<TD><P style=\"margin-left:5\">@<", offset, &offset);
				Replace(in, "&nbsp;", " ");
				TrayList.back().PageType = in;
			}
			else if (in == "Standard Bin")
			{
				TrayList.push_back(Tray());
				TrayList.back().Name = "Bin   ";
				
				in = Search(HtmlStatus, "height=\"1\"><tr><td><b>@<", offset, &offset);
				in += std::string(5-in.size(), ' ');
				TrayList.back().Status = in;
				
				in = Search(HtmlStatus, "<TD><P style=\"margin-left:5\">@<", offset, &offset);
				TrayList.back().Capacity = stoi(in);
			}
			else
			{
				doTray = false;
			}
		}
	}
}

std::string Printer::GetUrlTopbar()
{
	return "httpconfig.co";
	return "br-" + Name + "-prn2.internal/cgi-bin/dynamic/topbar.html";
}

std::string Printer::GetUrlStatus()
{
	return "httponfig.co";
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
	CURL *curl = curl_easy_init();
	if(!curl) { return 0; }
	CURLcode res = CURLE_OK;
	
	curl_easy_setopt(curl, CURLOPT_URL, GetUrlTopbar().c_str());
	//curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &HtmlTopBar);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);
	HtmlTopBar.clear();
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) { Status = "Network Error"; }
	
	curl_easy_setopt(curl, CURLOPT_URL, GetUrlStatus().c_str());
	//curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &HtmlStatus);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);
	HtmlStatus.clear();
	res = (res?res:curl_easy_perform(curl));
	if (res != CURLE_OK) { Status = "Network Error"; }
	
	curl_easy_cleanup(curl);
	
	if (res == CURLE_OK) { GetStatus(); }
	
	return res;
	
#include <stdio.h>
	FILE *file = fopen("html1.html", "r");
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
	
	file = fopen("html2.html", "r");
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	buff = new char[size + 1];
	fread(buff, size, 1, file);
	fclose(file);
	buff[size-1] = 0;
	HtmlStatus = std::string(buff);
	//printf("%d\n", size);
	delete[] buff;
	
	
	
	GetStatus();
	
	
	return 0;
}

void Printer::Draw(Screen *screen)
{
	wclear(Pad);
	
	/*auto tab = [&](){ wattrset(pad, COLOR_PAIR(selected ? HIGHLIGHT : NORMAL)); waddstr(pad, "    "); wattrset(pad, COLOR_PAIR(NORMAL)); };
	//wattrset(pad, COLOR_PAIR(i==Cursor ? HIGHLIGHT : NORMAL));
	wattrset(pad, (selected ? A_BOLD : 0));
	wattrset(pad, (selected ? COLOR_PAIR(HIGHLIGHT) | A_BOLD | A_DIM : COLOR_PAIR(NORMAL)));
	waddstr(pad, Name.c_str());
	wattrset(pad, COLOR_PAIR(NORMAL));
	waddstr(pad, ("\t" + Status).c_str());  FillLine(pad, ' ');*/
	
	//auto begin = [&](){ if (selected) { wattrset(pad, COLOR_PAIR(selected?HIGHLIGHT:NORMAL)); } waddstr(pad, " "); wattrset(pad, COLOR_PAIR(NORMAL)); waddstr(pad, " "); };
	
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
		int len = TrayList.size() - 2;
		int x=0, y=0;
		if (TrayList[i].Name == "Feeder")
		{
			y = (len-len%2)/2;
		}
		else if (TrayList[i].Name == "Bin   ")
		{
			x = 42;
			y = (len-len%2)/2;
		}
		else
		{
			x = (i >= (len-len%2)/2) * 39;
			y = (i % ((len-len%2)/2));
		}
		//int x = ((i >= (len-len%2)/2 && i<len-1) || ((i==len-1) && (i%2))) * 42;
		//int y = i % ((len-len%2)/2) + (i==len-1 && !(i%2)) * (len-len%2)/2;
		//int x=0, y=i;
		wmove(Pad, y+2, x);
		
		waddstr(Pad, (TrayList[i].Name + "  ").c_str());
		
		if (TrayList[i].Status == "OK   ") { wattrset(Pad, COLOR_PAIR(0b010000)); }
		if (TrayList[i].Status == "Low  ") { wattrset(Pad, A_BOLD | COLOR_PAIR(0b011000)); }
		if (TrayList[i].Status == "Empty") { wattrset(Pad, A_BOLD | COLOR_PAIR(0b001000)); }
		waddstr(Pad, TrayList[i].Status.c_str());
		wattrset(Pad, COLOR_PAIR(NORMAL));
		waddstr(Pad, ("," + std::to_string(TrayList[i].Capacity) + "," + TrayList[i].PageSize + "," + TrayList[i].PageType).c_str()); FillLine(Pad, ' ');
	}
	//waddstr(Pad, "Tray 1  OK  500  Letter  Plain Paper"); FillLine(Pad, ' ');
	//waddstr(Pad, "Tray 2  OK  500  Letter  Plain Paper"); FillLine(Pad, ' ');
	//waddstr(Pad, "Tray 3  OK  500  Letter  Plain Paper"); FillLine(Pad, ' ');
	//waddstr(Pad, "Tray 4  OK  500  Letter  Plain Paper"); FillLine(Pad, ' ');
	//FillLine(Pad, '-');
}
	
	