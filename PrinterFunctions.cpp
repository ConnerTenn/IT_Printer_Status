
#include "Printer.h"


std::vector<Printer *> PrinterList;
std::vector<Printer *> PrinterUpdateThreadList;
Printer *Selected = 0;
int SortOrder = 0;
int PrinterColumns[10];
int MinStatusLength = 0;
int MaxStatusLength = 50;
//std::mutex PrinterListGuard;

void InitPrinters()
{
	DestroyPrinters();
	
	//PrinterListGuard.lock();
	
	std::ifstream file("Printers.txt");
	std::string line;
	
	while(std::getline(file, line))
	{
		if (line.size())
		{
			PrinterList.push_back(new Printer(line));
			PrinterUpdateThreadList.push_back(PrinterList.back());
		}
	}
	
	//mutexes created after adding printers to list to handle how std::vector copying and deleting Printer object issues
	/*for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		PrinterList[i].Mutex = new std::mutex;
	}*/
	//PrinterListGuard.unlock();
	
	SortPrinters();
}

void DestroyPrinters()
{
	//PrinterListGuard.lock();
	
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		if (PrinterList[i]) { delete PrinterList[i]; }
	}
	
	PrinterList.clear();
	PrinterUpdateThreadList.clear();
	
	//PrinterListGuard.unlock();
}

//Sorts printers in PrinterList
void SortPrinters()
{
	//Sort fuction to define sort order
	auto sortFunc = [](Printer *a, Printer *b)->int
		{ 
			if (SortOrder == 0)
			{
				return a->Name.compare(b->Name);
			}
			else if (SortOrder == 1)
			{
				
				//Error status comes last
				if (a->Status.find("File Error") != std::string::npos || a->Status.find("Network Error") != std::string::npos) { return 1; }
				else if (b->Status.find("File Error") != std::string::npos || b->Status.find("Network Error") != std::string::npos) { return -1; }
				// Red, Yellow, Blue, Green
				if (a->StatusColour == 0b001000) { if (b->StatusColour == 0b001000) { a->Name.compare(b->Name); } else { return -1; } }
				else if (b->StatusColour == 0b001000) { return 1; }
				else if (a->StatusColour == 0b011000) { if (b->StatusColour == 0b011000) { a->Name.compare(b->Name); } else { return -1; } }
				else if (b->StatusColour == 0b011000) { return 1; }
				else if (a->StatusColour == 0b100000) { if (b->StatusColour == 0b100000) { a->Name.compare(b->Name); } else { return -1; } }
				else if (b->StatusColour == 0b100000) { return 1; }
				else if (a->StatusColour == 0b010000) { if (b->StatusColour == 0b010000) { a->Name.compare(b->Name); } else { return -1; } }
				else if (b->StatusColour == 0b010000) { return 1; }
			}
			
			return 0;
		};
	
	
	
	//simple selection sort algorithm
	int sorted = 0;
	while (sorted < (int)PrinterList.size())
	{
		int select = sorted;
		//find the item that comes next in the list
		for (int i = sorted+1; i < (int)PrinterList.size(); i++)
		{
			if (sortFunc(PrinterList[i], PrinterList[select]) < 0)
			{
				select = i;
			}
		}
		
		//moves selected elem to front of list
		Printer *temp = PrinterList[sorted];
		PrinterList[sorted] = PrinterList[select];
		PrinterList[select] = temp;
		
		sorted++;
	}
}




void GetPrinterDisplayHeight(int *maxY, int *indexMinY, int *indexMaxY, int index)
{
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		if (i <= index)
		{
			if (indexMaxY) { *indexMaxY += (PrinterList[i]->Expanded ? PrinterHeight : 1) + (i < (int)PrinterList.size() - 1 ? 1 : 0); }
			if (indexMinY && i-1 >= 0) { *indexMinY += (PrinterList[i-1]->Expanded ? PrinterHeight : 1) + (i < (int)PrinterList.size() - 1 ? 1 : 0); }
		}
		if (maxY) { *maxY += (PrinterList[i]->Expanded ? PrinterHeight : 1) + (i < (int)PrinterList.size() - 1 ? 1 : 0); }
	}
}

void UpdatePrinterColumns()
{
	const int begin = 18, tonerWidth = 25;
	int PrinterColumnWidths[10];
	
	for (int i = 0; i < 10; i++) { PrinterColumnWidths[i] = 0; }
	PrinterColumnWidths[1] = tonerWidth;
	
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		PrinterColumnWidths[0] = MAX(PrinterColumnWidths[0], (int)PrinterList[i]->Status.size());
		
		for (int j = 0; j < (int)PrinterList[i]->TrayList.size(); j++)
		{
			int k = j+2;
			if (PrinterList[i]->TrayList[j].Name == "Multi-Purpose Feeder") { k = 7; }
			if (PrinterList[i]->TrayList[j].Name == "Standard Bin") { k = 8; }
			if (PrinterList[i]->TrayList[j].Name == "Bin 1") { k = 9; }
			if (PrinterList[i]->TrayList.size() >= 0) { PrinterColumnWidths[k] = MAX(PrinterColumnWidths[k], (int)PrinterList[i]->TrayList[j].Name.size() + (int)PrinterList[i]->TrayList[j].Status.size() + 4); }
		}
	}
	
	PrinterColumnWidths[0] = MIN(MAX(PrinterColumnWidths[0], MinStatusLength), MaxStatusLength);
	
	PrinterColumns[0] = begin;
	
	for (int i = 1; i < 10; i++)
	{
		PrinterColumns[i] = PrinterColumns[i-1] + PrinterColumnWidths[i-1];
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
