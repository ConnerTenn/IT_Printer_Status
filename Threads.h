
#include <thread>
#include "Printer.h"


extern bool Run;
time_t Timer = 0;
int ThreadNum = 10;
std::thread **PrinterThreads = 0;
bool *PrinterComplete = 0;
int UpdateIndex = 0;
std::mutex UpdateIndexMutex;
int RefreshDelay = 5;

void UpdatePrinters(int index)
{
	bool loop = true;
	while (loop)
	{
		int updateI = 0;
		UpdateIndexMutex.lock();
		loop = UpdateIndex < (int)PrinterUpdateThreadList.size() && Run;
		updateI = UpdateIndex;
		UpdateIndex++;
		UpdateIndexMutex.unlock();
			
		if (loop) { PrinterUpdateThreadList[updateI]->Update(); }
	}
		

	PrinterComplete[index] = true;
}



void InitThreads()
{
	RefreshingPrinters = true;
	PrinterThreads = new std::thread *[ThreadNum];
	PrinterComplete = new bool[ThreadNum];
	
	UpdateIndex = 0;
	Timer = 0;
	for (int i = 0; i < ThreadNum; i++)
	{
		PrinterComplete[i] = false;
		PrinterThreads[i] = new std::thread(UpdatePrinters, i);
	}
}

void JoinThreads()
{
	Run = false;

	for (int i = 0; i < ThreadNum && PrinterThreads; i++)
	{
		if (PrinterThreads[i])
		{
			PrinterThreads[i]->join();
			delete PrinterThreads[i];
			PrinterThreads[i] = 0;
		}
	}

	if (PrinterThreads) { delete[] PrinterThreads; PrinterThreads=0; }
	if (PrinterComplete) { delete[] PrinterComplete; PrinterComplete=0; }
	
	Run = true;
	
	RefreshingPrinters = false;
}

void UpdateThreadTimer()
{
	if (Timer && time(0) - Timer > RefreshDelay)
	{
		UpdateIndex = 0;
		InitThreads();
		Timer = 0;
		
	}
	else if (!Timer && UpdateIndex >= (int)PrinterUpdateThreadList.size())
	{
		bool complete = true;
		for (int i = 0; i < ThreadNum; i++) { complete = complete && PrinterComplete[i]; }

		if (complete)
		{
			JoinThreads();
			Timer = time(0);
		}
	}
}

