#pragma once
/**	
	Klasa odpowiadająca za logowanie informacji, ostrzerzeń i błędów
	z silnika gry do pliku/konsoli Windows lub innych
**/
#pragma once
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <deque>
#include <string>
#include <SDL.h>
/*
Logger::Open( const char* file );
Logger::Info( const char* format, ... );
Logger::Success( const char* format, ... );
Logger::Fail( const char* format, ... );
Logger::Error( const char* format, ... );
Logger::Debug( const char* format, ... );
*/

#define logger Logger::getInstance()

enum LogType
{
	LOG_INFO = 0,
	LOG_SUCCESS = 1,
	LOG_ERROR = 2,
	LOG_FATAL = 3,
	LOG_DEBUG = 4
};

struct Message
{
	LogType type;
	std::string message;
};

class Logger
{
private:
	FILE* file;
	bool initialized = false;
	bool verbose = false;

	std::deque<Message> messages;

	void Log(int type, const char* format, va_list list)
	{
		char buffer[2048];

		vsprintf(buffer, format, list);

		char _logtype[64];

		switch (type)
		{
		case LOG_INFO:
			strcpy(_logtype, "INFO");
			break;

		case LOG_SUCCESS:
			strcpy(_logtype, "SUCCESS");
			break;

		case LOG_ERROR:
			strcpy(_logtype, "ERROR");
			break;

		case LOG_FATAL:
			strcpy(_logtype, "FATAL");
			break;

		case LOG_DEBUG:
			strcpy(_logtype, "DEBUG");
			break;

		default:
			strcpy(_logtype, "UNKNOWN");
			break;
		}

		char _time[64];
		time_t secs = time(0);
		strftime(_time, 64, "%X", localtime(&secs));

		/*if (type != LOG_DEBUG) */fprintf(file, "[%s][%s]\t%s\n", _time, _logtype, buffer);
		printf("[%s][%s]\t%s\n", _time, _logtype, buffer);
		if (type == LOG_FATAL)
		{
			char message_buffer[4096];
			sprintf(message_buffer, "FATAL ERROR: %s", buffer);
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", message_buffer, NULL);
		}

		fflush(file);

		char bigbuffer[4096];
		sprintf(bigbuffer, "[%s][%s]\t%s\n", _time, _logtype, buffer);

		Message m;
		m.type = (LogType)type;
		m.message = std::string(bigbuffer);
		messages.push_back(m);
		if (messages.size() > 10) messages.pop_front();
		//OutputDebugString(bigbuffer);
	}
	void Open(const char* filename)
	{
		file = fopen(filename, "w");
		initialized = true;
		Info("=== Log opended ===");
	}
	void Close()
	{
		Info("=== Log closed ===");
		fclose(file);
		initialized = false;
	}
public:
	Logger()
	{
		Open("log.txt");
	}
	Logger(const char* filename)
	{
		Open(filename);
	}
	~Logger()
	{
		Close();
	}

	static Logger& getInstance()
	{
		static Logger instance;
		return instance;
	}

	// Only Errors and fatals are logged
	void Quiet()
	{
		verbose = false;
	}
	// Everyting including success and info are logged
	void Verbose()
	{
		verbose = true;
	}
	void Info(const char* format, ...)
	{
		va_list list;
		va_start(list, format);
		Log(LOG_INFO, format, list);
		va_end(list);
	}
	void Success(const char* format, ...)
	{
		va_list list;
		va_start(list, format);
		Log(LOG_SUCCESS, format, list);
		va_end(list);
	}
	void Error(const char* format, ...)
	{
		va_list list;
		va_start(list, format);
		Log(LOG_ERROR, format, list);
		va_end(list);
	}
	void Fatal(const char* format, ...)
	{
		va_list list;
		va_start(list, format);
		Log(LOG_FATAL, format, list);
		va_end(list);
	}
	void Debug(const char* format, ...)
	{
		if (!verbose) return;
		va_list list;
		va_start(list, format);
		Log(LOG_DEBUG, format, list);
		va_end(list);
	}

	std::deque<Message>& getMessages() {
		return messages;
	}
};