
#include "Printer.h"

std::string Search(std::string delim)
{
	
}



Printer::Printer() {}
Printer::Printer(std::string name) : Name(name) {}

std::string Printer::GetStatus()
{
	std::string status;
	
	
	
	return "";
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
	CURL *curl = curl_easy_init();
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
	
	return res;
}
	
	