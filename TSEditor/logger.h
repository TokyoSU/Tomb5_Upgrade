#pragma once

enum class LogType
{
	Trace,
	Info,
	Debug,
	Warn,
	Error,
	Critical
};

enum class MsgType
{
	Info,
	Warn,
	Error
};

extern void TraceLogInit();
extern void TraceLog(LogType type, const char* message, ...);
extern void TraceLogSDL(LogType type, const char* message, ...);

extern int DrawMessageBox(MsgType type, const char* name, const char* message, ...);
