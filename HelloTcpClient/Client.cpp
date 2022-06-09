#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "EasyTcpClient.hpp"
#include<thread>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

bool g_bRun = true;
void cmdThread() {
	while (true) {
		char cmdBuf[256] = {};
		scanf_s("%s", cmdBuf, 128);
		if (0 == strcmp(cmdBuf, "exit")) {
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
	const int cCount = FD_SETSIZE - 1;
	EasyTcpClient* client[cCount];
	//client1.InitSocket();
	for (int n = 0; n < cCount; n++) {
		client[n] = new EasyTcpClient();
	}
	for (int n = 0; n < cCount; n++) {
		client[n]->Connect("127.0.0.1", 4567);
	}

	//启动UI线程
	std::thread t1(cmdThread);
	t1.detach();
	Login login;
	strcpy_s(login.userName, "Chris");
	strcpy_s(login.PassWord, "123456");
	while (g_bRun) {
		//client1.OnRun();
		for (int n = 0; n < cCount; n++) {
			
			client[n]->SendData(&login);
			client[n]->OnRun();
		}
	}
	for (int n = 0; n < cCount; n++) {
		client[n]->Close();
	}
	//7
	getchar();
	return 0;
}
