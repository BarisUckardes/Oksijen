#pragma once
#include <string>
#include <map>
#include <vector>

namespace Oksijen
{
	class Logger final
	{
		struct Entry
		{
			std::string Title;
			unsigned long long Time;
		};
	public:
		Logger(const std::string logFile);
		~Logger();

		void AddEvent(const std::string& title);
		void DumpLogs();
	private:
		const std::string mLogFile;
		std::vector<Entry> mEntries;
	};
}