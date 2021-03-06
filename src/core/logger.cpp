#include"core\logger.h"
#include"common\version.h"

#include<fstream>
#include<chrono>

namespace Logger {
	namespace {
		const string errorLogFileName  = "error.txt";
		const string normalLogFileName = "logger.txt";
		std::ofstream errorFile;
	
		std::ofstream& GetErrorFile()
		{
			if (!errorFile.is_open())
				errorFile.open(errorLogFileName);
			return errorFile;
		}

		std::ofstream& GetLoggerFile()
		{
			if (!errorFile.is_open())
				errorFile.open(normalLogFileName);
			return errorFile;
		}


		// 以字符串形式获取系统当前时间
		std::string GetCurSystemTimeStr()
		{
			auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			tm ptm;
			localtime_s(&ptm, &tt);
			char date[60] = { 0 };
			sprintf_s(date, "%02d:%02d:%02d ",
				(int)ptm.tm_hour, (int)ptm.tm_min, (int)ptm.tm_sec);
			return std::string(date);
		}

		// 设置当前控制台字体颜色
		const int CONSOLE_FONT_WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
		const int CONSOLE_FONT_BLUE = FOREGROUND_BLUE;
		const int CONSOLE_FONT_YELLOW = FOREGROUND_RED | FOREGROUND_GREEN;
		const int CONSOLE_FONT_GREEN = FOREGROUND_GREEN;
		const int CONSOLE_FONT_RED = FOREGROUND_RED;
		void SetLoggerConsoleFontColor(int fontColor)
		{
			HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | fontColor);
		}
	}
	
	void Logger::Print(const string & msg, std::ostream & out)
	{
		auto timeStr = GetCurSystemTimeStr();
		out << timeStr << "  " << msg << std::endl;

		std::ostream& logger = GetLoggerFile();
		logger << timeStr << "  " << msg << std::endl;
	}

	void Debug(const string & msg)
	{
		SetLoggerConsoleFontColor(CONSOLE_FONT_YELLOW);
		//Print("[Debug] " + msg);
		Print(msg);
		SetLoggerConsoleFontColor(CONSOLE_FONT_WHITE);
	} 

	void Logger::Info(const string & msg)
	{
		SetLoggerConsoleFontColor(CONSOLE_FONT_GREEN);
		Print("[Info]  " + msg);
		SetLoggerConsoleFontColor(CONSOLE_FONT_WHITE);
	}

	void Logger::Warning(const string & msg)
	{
		SetLoggerConsoleFontColor(CONSOLE_FONT_YELLOW);
		string warningMsg = "[Warning]  " + msg;
		Print(warningMsg);
		Print(warningMsg, GetErrorFile());
		SetLoggerConsoleFontColor(CONSOLE_FONT_WHITE);
	}

	void Logger::Error(const string & msg)
	{
		SetLoggerConsoleFontColor(CONSOLE_FONT_RED);
		string warningMsg = "[Error]  " + msg;
		Print(warningMsg);
		Print(warningMsg, GetErrorFile());
		SetLoggerConsoleFontColor(CONSOLE_FONT_WHITE);
	}

	void Fatal(const string& msg)
	{
		SetLoggerConsoleFontColor(CONSOLE_FONT_WHITE);
		string warningMsg = "[Fatal]  " + msg;
		Print(warningMsg);
		Print(warningMsg, GetErrorFile());
	}

	void PrintConsoleHeader()
	{
		std::cout << "Cjing-2d Version " << 
			CJING_VERSION_MAJOR << "." <<
			CJING_VERSION_MINOR << "." << 
			CJING_VERSION_PATCH << std::endl;
		std::cout << "Copyright (c) 2016-2018 ZZZY." << std::endl;
		std::cout << std::endl;
	}
}
