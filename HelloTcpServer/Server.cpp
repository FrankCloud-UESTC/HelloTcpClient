#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include<iostream>
#include<vector>
#include<thread>
#include"EasyTcpServer.hpp"
#pragma comment(lib, "ws2_32.lib")
using namespace std;

bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf_s("%s", cmdBuf, 128);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("退出cmdThread线程\n");
			break;
		}
		else {
			printf("不支持的命令。\n");
		}
	}
}

int main() {
	
	EasyTcpServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	//启动UI线程
	std::thread t1(cmdThread);
	t1.detach();
	while (g_bRun) {
		server.OnRun();
		
	}
	server.Close();
	getchar();
	return 0;
}
