#include <M3D/m3d_log.h>
#include <iostream>

void LogMan(int logLevel, const char* msg)
{
	std::cout << "[" << logLevel << "]::" << msg << std::endl;
}