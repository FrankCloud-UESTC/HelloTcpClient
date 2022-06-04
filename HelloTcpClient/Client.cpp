#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include<iostream>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

struct DataPackage {
	int age;
	char name[32];
};

int main() {
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	//1.build a socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2.连接服务器
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret) {
		cout << "Error, 连接服务器失败..." << endl;
	}
	else {
		cout << "Success 连接服务器成功..." << endl;
	}

	char _recvBuf[128] = {};
	while (true) {
		//3.
		char cmdBuf[128] = {};
		scanf_s("%s", cmdBuf, 128);
		//4
		if (0 == strcmp(cmdBuf, "exit")) {
			cout << "收到exit命令，结束";
			break;
		}
		else {
			send(_sock, cmdBuf, strlen(cmdBuf) + 1, 0);
		}
		//5.处理请求
		char recvBuf[128] = {};
		int nlen = recv(_sock, recvBuf, 128, 0);
		if (nlen > 0) {
			DataPackage* dp = (DataPackage*)recvBuf;
			cout << "年龄 = " << dp->age << " 姓名 = " << dp->name << endl;
		}
	}
	//7
	closesocket(_sock);
	WSACleanup();
	getchar();
}