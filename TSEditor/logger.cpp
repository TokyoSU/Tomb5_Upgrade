#include "framework.h"
#include "logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <tinyfiledialogs/tinyfiledialogs.h>

static std::shared_ptr<spdlog::logger> fileLog = spdlog::basic_logger_mt("file", "logs/debug.txt", true);
static std::shared_ptr<spdlog::logger> consoleLog = spdlog::stdout_color_mt("console");
static std::string prevMsg = "";

void TraceLogInit()
{
	fileLog->set_level(spdlog::level::trace);
	consoleLog->set_level(spdlog::level::trace);
}

void TraceLog(LogType type, const char* message, ...)
{
	char buffer[512] = {};
	va_list args;
	va_start(args, message);
	vsprintf_s(buffer, message, args);
	va_end(args);

	if (prevMsg == buffer)
		return;

	switch (type)
	{
	case LogType::Trace:
		fileLog->trace(buffer);
		consoleLog->trace(buffer);
		break;
	default:
	case LogType::Info:
		fileLog->info(buffer);
		consoleLog->info(buffer);
		break;
	case LogType::Debug:
		fileLog->debug(buffer);
		consoleLog->debug(buffer);
		break;
	case LogType::Warn:
		fileLog->warn(buffer);
		consoleLog->warn(buffer);
		break;
	case LogType::Error:
		fileLog->error(buffer);
		consoleLog->error(buffer);
		break;
	case LogType::Critical:
		fileLog->critical(buffer);
		consoleLog->critical(buffer);
		break;
	}

	prevMsg = buffer;
}

void TraceLogSDL(LogType type, const char* message, ...)
{
	char buffer[512] = {};
	va_list args;
	va_start(args, message);
	vsprintf_s(buffer, message, args);
	va_end(args);

	if (prevMsg == buffer)
		return;

	std::string result = buffer;
	result.append(", SDL Error: " + std::string(SDL_GetError()));

	switch (type)
	{
	case LogType::Trace:
		fileLog->trace(result);
		consoleLog->trace(result);
		break;
	default:
	case LogType::Info:
		fileLog->info(result);
		consoleLog->info(result);
		break;
	case LogType::Debug:
		fileLog->debug(result);
		consoleLog->debug(result);
		break;
	case LogType::Warn:
		fileLog->warn(result);
		consoleLog->warn(result);
		break;
	case LogType::Error:
		fileLog->error(result);
		consoleLog->error(result);
		break;
	case LogType::Critical:
		fileLog->critical(result);
		consoleLog->critical(result);
		break;
	}

	prevMsg = result;
}

int DrawMessageBox(MsgType type, const char* name, const char* message, ...)
{
	char buffer[512] = {};
	va_list args;
	va_start(args, message);
	vsprintf_s(buffer, message, args);
	va_end(args);

	char nameBuffer[16] = {};
	switch (type)
	{
	default:
	case MsgType::Info:
		fileLog->info(buffer);
		consoleLog->info(buffer);
		sprintf_s(nameBuffer, "info");
		break;
	case MsgType::Warn:
		fileLog->warn(buffer);
		consoleLog->warn(buffer);
		sprintf_s(nameBuffer, "warn");
		break;
	case MsgType::Error:
		fileLog->error(buffer);
		consoleLog->error(buffer);
		sprintf_s(nameBuffer, "error");
		break;
	}

	return tinyfd_messageBox(name, buffer, "okcancel", nameBuffer, 1);
}
