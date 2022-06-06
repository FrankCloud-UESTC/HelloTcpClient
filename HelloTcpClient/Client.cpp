#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "EasyTcpClient.hpp"
#include<thread>
#pragma comment(lib, "ws2_32.lib")
using namespace std;


void cmdThread(EasyTcpClient* client)
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf_s("%s", cmdBuf, 128);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			client->Close();
			printf("退出cmdThread线程\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy_s(login.userName, 128, "Chris");
			strcpy_s(login.PassWord, 128, "123456");
			client->SendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy_s(logout.userName, 128, "Chris");
			client->SendData(&logout);
		}
		else {
			printf("不支持的命令。\n");
		}
	}
}

int main() {
	EasyTcpClient client;
	client.InitSocket();
	client.Connect("127.0.0.1", 4567);
	//启动UI线程
	std::thread t1(cmdThread, &client);
	t1.detach();

	while (client.isRun()) {
		client.OnRun();
	}
	//7
	client.Close();
	getchar();
	return 0;
}
