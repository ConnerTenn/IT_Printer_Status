

#include "Printer.h"


Printer::Printer() : Printer("") {}
Printer::Printer(std::string name) : Name(name) { Mutex = new std::mutex; }

Printer::~Printer()
{
	if (Mutex) { delete Mutex; Mutex = 0; }
	
	if (Pad) { delwin(Pad); }
}

void Printer::GetToner()
{
	int offset = 0;
	std::string in;
	
	TonerList.clear();
	
	Search(HtmlStatus, "<!-- Toner Level -->", offset, &offset);
	
	while (First(HtmlStatus, "><B>", "<hr", offset) == 1)
	{
		Toner toner; bool front = false;
		in = Search(HtmlStatus, "><B>@<", offset, &offset);
		
		if (in.find("Black") != std::string::npos) { toner.Colour = 0b111111; front = true; }
		else if (in.find("Cyan") != std::string::npos) { toner.Colour = 0b110110; }
		else if (in.find("Magenta") != std::string::npos) { toner.Colour = 0b101101; }
		else if (in.find("Yellow") != std::string::npos) { toner.Colour = 0b011011; }
		
		in = Search(HtmlStatus, "<TBODY>*<TR>*<TD width=\"@%", offset, &offset);
		toner.Percent = atoi(in.c_str());
		if (First(HtmlStatus, "bgColor=#ffffff", "&nbsp;", offset) == 1) { toner.Percent = 0; }
		
		if (front) { TonerList.insert(TonerList.begin(), toner); }
		else { TonerList.push_back(toner); }
		
		Search(HtmlStatus, "</table>", offset, &offset);
	}
}

void Printer::GetTrays()
{
	int offset = 0;
	std::string in;
	
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
}

void Printer::GetKits()
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

void Printer::GetStatus()
{
	//Read the topbar html
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
		else if (colour == "#FF3333")
		{
			StatusColour = 0b001000;
		}
		else if (colour == "#97B1FF")
		{
			StatusColour = 0b100000;
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
		//Toner = stoi(Search(HtmlStatus, "Black*~@%"));
		GetToner();
		
		GetTrays();
		
		GetKits();
		
		//if (Toner == 0 && StatusColour == 0b111000 ) { StatusColour = 0b011000; }
		
		Mutex->unlock();
		
		//MaxStatusLength = MAX(MaxStatusLength, (int)Status.size());
	}
}

std::string URLTopbar = ".internal/cgi-bin/dynamic/topbar.html";
std::string URLStatus = ".internal/cgi-bin/dynamic/printer/PrinterStatus.html";
std::mutex URLMutex;

std::string Printer::GetUrlTopbar()
{
	URLMutex.lock();
	std::string urlTopbar = URLTopbar;
	URLMutex.unlock();
	return Name + urlTopbar;
}

std::string Printer::GetUrlStatus()
{
	URLMutex.lock();
	std::string urlStatus = URLStatus;
	URLMutex.unlock();
	return Name + urlStatus;
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

	Mutex->lock();
	curl_easy_setopt(curl, CURLOPT_URL, GetUrlTopbar().c_str());
	Mutex->unlock();
	//curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, NetworkTimeout);
	Buffer.clear();
	res = curl_easy_perform(curl);
	HtmlTopBar = Buffer;
	if (res == CURLE_OK && HtmlTopBar.find("<html class=\"top_bar\">") == std::string::npos) { res = CURLE_RECV_ERROR; }
	if (res != CURLE_OK) { Status = "Network Error:" + std::to_string(res); StatusColour = 0b001000; }
	
	Mutex->lock();
	curl_easy_setopt(curl, CURLOPT_URL, GetUrlStatus().c_str());
	Mutex->unlock();
	//curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, NetworkTimeout);
	Buffer.clear();
	res = (res?res:curl_easy_perform(curl));
	if (res == CURLE_OK && HtmlTopBar.find("<html class=\"top_bar\">") == std::string::npos) { res = CURLE_RECV_ERROR; }
	if (res != CURLE_OK) { Status = "Network Error:" + std::to_string(res); StatusColour = 0b001000; }
	HtmlStatus = Buffer;

	curl_easy_cleanup(curl);
	
	if (res == CURLE_OK) { GetStatus(); }
	
	return res;

#else
	
#include <stdio.h>

	int res = 0;
	Mutex->lock(); FILE *file = fopen((Name + "-topbar.html").c_str(), "r"); Mutex->unlock();
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
	
	
	if (res) { Mutex->lock(); file = fopen((Name + "-status.html").c_str(), "r"); Mutex->unlock(); }
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

#ifdef LINUX
	sleep(2);
#elif WINDOWS
	Sleep(2000);
#endif
	
	return 0;
#endif
}

void Printer::Draw(Screen *screen)
{
	//int TonerStart = MaxStatusLength + 19;
	//int TrayStart = TonerStart + 26;
	
	wclear(Pad);
	
	if (Selected == this) { wattrset(Pad, A_BOLD | A_REVERSE | COLOR_PAIR(0b110000)); } else { wattrset(Pad, COLOR_PAIR(NORMAL)); }
	if (Expanded) { waddch(Pad, ACS_ULCORNER); } else { waddch(Pad, ACS_HLINE ); }
	
	if (Selected == this) { wattrset(Pad, A_BOLD | A_REVERSE | COLOR_PAIR(0b110000)); } else { wattrset(Pad, A_BOLD | COLOR_PAIR(0b110000)); }
	waddstr(Pad, ("[" + MinSize(Name, 12) + "]").c_str());
	wattrset(Pad, COLOR_PAIR(NORMAL));
	waddstr(Pad, "  ");
	

	wattrset(Pad, A_BOLD | COLOR_PAIR(StatusColour));
	std::string status = Status;
	if ((int)Status.size() > MaxStatusLength) { status = MaxSize(status, MaxStatusLength-3); status += "..."; }
	else { status = MinSize(status, MinStatusLength); }
	waddstr(Pad, status.c_str()); 
	waddstr(Pad, "  "); 
	wattrset(Pad, COLOR_PAIR(NORMAL));
	
	
	
	bool noError = Status.size() && Status.find("File Error") == std::string::npos && Status.find("Network Error") == std::string::npos;
	
	if (noError)
	{
		wmove(Pad, 0, PrinterColumns[1]);
		//if (TonerList[0].Percent == 0) { wattrset(Pad, A_BOLD | COLOR_PAIR(0b001000)); } else if (TonerList[0].Percent <= 20) { wattrset(Pad, A_BOLD | COLOR_PAIR(0b011000)); } else { wattrset(Pad, A_BOLD | COLOR_PAIR(0b010000)); }
		short TonerStatusColour = 0b010010;
		for (int i = 0; i < (int)TonerList.size(); i++) { if (TonerList[i].Percent <= 10) { TonerStatusColour = 0b011011; } }
		for (int i = 0; i < (int)TonerList.size(); i++) { if (TonerList[i].Percent == 0) { TonerStatusColour = 0b001001; } }
		wattrset(Pad, A_BOLD | COLOR_PAIR(TonerStatusColour & 0b111000));
		waddstr(Pad, "Toner ");
		
		if ((int)TonerList.size() > 1) 
		{
			if (!Expanded)
			{
				for (int i = 0; i < (int)TonerList.size(); i++)
				{
					wattrset(Pad, A_BOLD | A_REVERSE | COLOR_PAIR(TonerList[i].Colour));
					waddch(Pad, ' ');
					if (!TonerList[i].Percent) { wattrset(Pad, A_BOLD | COLOR_PAIR(0b001000)); }
					else if (TonerList[i].Percent <= 10) { wattrset(Pad, A_BOLD | COLOR_PAIR(0b011000)); }
					else { wattrset(Pad, A_BOLD | COLOR_PAIR(0b010000)); }
					waddstr(Pad, MinSize("~" + std::to_string(TonerList[i].Percent) + "%", 6).c_str());
				}
			}
			else
			{
				for (int i = 0; i < (int)TonerList.size(); i++)
				{
					wmove(Pad, i, PrinterColumns[1]+6);
					wattrset(Pad, A_BOLD | COLOR_PAIR(TonerList[i].Colour & 0b111000));
					waddstr(Pad, "["); 
					wattrset(Pad, A_BOLD | A_REVERSE | COLOR_PAIR(TonerList[i].Colour & 0b111111));
					waddstr(Pad, std::string(TonerList[i].Percent/10, ' ').c_str());
					wattrset(Pad, A_BOLD | COLOR_PAIR(TonerList[i].Colour & 0b111000));
					waddstr(Pad, std::string(10 - TonerList[i].Percent/10, '_').c_str());
					waddstr(Pad, "]");
					
					if (!TonerList[i].Percent) { wattrset(Pad, A_BOLD | COLOR_PAIR(0b001000)); }
					else if (TonerList[i].Percent <= 10) { wattrset(Pad, A_BOLD | COLOR_PAIR(0b011000)); }
					else { wattrset(Pad, A_BOLD | COLOR_PAIR(0b010000)); }
					waddstr(Pad, MinSize("~" + std::to_string(TonerList[i].Percent) + "%", 6).c_str());
				}
			}
		}
		else
		{
			wattrset(Pad, A_BOLD | COLOR_PAIR(0b111000));
			waddstr(Pad, "["); 
			wattrset(Pad, A_BOLD | COLOR_PAIR(0b111111));
			waddstr(Pad, std::string(TonerList[0].Percent/10, ' ').c_str());
			wattrset(Pad, A_BOLD | COLOR_PAIR(0b111000));
			waddstr(Pad, std::string(10 - TonerList[0].Percent/10, '_').c_str());
			waddstr(Pad, "]");
			wattrset(Pad, A_BOLD | COLOR_PAIR(TonerStatusColour & 0b111000));
			waddstr(Pad, MinSize("~" + std::to_string(TonerList[0].Percent) + "%", 6).c_str());
		}
		
			
		//wmove(Pad, 0, TrayStart);
		for (int i = 0; i < (int)TrayList.size(); i++)
		{
			wattrset(Pad, (Selected == this ? A_BOLD : 0) | COLOR_PAIR(NORMAL));
			
			int j = i+2;
			if (TrayList[i].Name == "Multi-Purpose Feeder") { j = 7; }
			if (TrayList[i].Name == "Standard Bin") { j = 8; }
			if (TrayList[i].Name == "Bin 1") { j = 9; }
			wmove(Pad, 0, PrinterColumns[j]);
			
			waddstr(Pad, (TrayList[i].Name + "  ").c_str());
				
			if (TrayList[i].Status == "OK") { wattrset(Pad, COLOR_PAIR(0b010000)); }
			else if (TrayList[i].Status == "Low") { wattrset(Pad, A_BOLD | COLOR_PAIR(0b011000)); }
			else /*if (TrayList[i].Status == "Empty")*/ { wattrset(Pad, A_BOLD | COLOR_PAIR(0b001000)); }
			waddstr(Pad, MinSize(TrayList[i].Status + " ", 6).c_str());
		}
	}
	
	
	//if (Expanded)
	{
		wmove(Pad, 1, 0);
		if (Selected == this) { wattrset(Pad, A_BOLD | A_REVERSE | COLOR_PAIR(0b110000)); }
		else { wattrset(Pad, COLOR_PAIR(NORMAL)); }
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
			
			
			for (int i = 0; i < (int)TrayList.size(); i++)
			{
				int j = i+2;
				if (TrayList[i].Name == "Multi-Purpose Feeder") { j = 7; }
				if (TrayList[i].Name == "Standard Bin") { j = 8; }
				if (TrayList[i].Name == "Bin 1") { j = 9; }
				wmove(Pad, 1, PrinterColumns[j]);
				waddstr(Pad, std::to_string(TrayList[i].Capacity).c_str());
				
				wmove(Pad, 2, PrinterColumns[j]);
				waddstr(Pad, TrayList[i].PageSize.c_str());
				
				wmove(Pad, 3, PrinterColumns[j]);
				waddstr(Pad, TrayList[i].PageType.c_str());
			}
		}
	}
}	
	