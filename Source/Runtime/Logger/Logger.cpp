#include "Logger.h"
#include <iostream>
#include <fstream>
#include <chrono>

namespace Oksijen
{
	static unsigned long long GetTimeUs()
	{
		auto duration = std::chrono::system_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
	}
	static unsigned long long GetTimeMs()
	{
		auto duration = std::chrono::system_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	}

	Logger::Logger(const std::string logFile) : mLogFile(logFile)
	{

	}

	Logger::~Logger()
	{

	}

	void Logger::AddEvent(const std::string& title)
	{
		const unsigned long long timeInUs = GetTimeUs();

		mEntries.push_back({ title,timeInUs });
	}

	void Logger::DumpLogs()
	{
		std::ofstream outputFile(mLogFile);

		if (outputFile.is_open())
		{
			for (const Entry& entry : mEntries)
			{
				outputFile << entry.Title + " " + std::to_string(entry.Time) + "\n";
			}

			outputFile.close(); // close the file when done
			std::cout << "Logger dumped\n";
		}
		else
		{
			std::cerr << "Error opening log file\n";
		}
	}

}