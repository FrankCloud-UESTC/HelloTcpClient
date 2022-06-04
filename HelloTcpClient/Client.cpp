#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include<iostream>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

enum CMD {
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_ERROR
};
//数据包头
struct DataHeader {
	short dataLength;//数据长度
	short cmd;
};
//数据包体
struct Login {
	char userName[32];
	char PassWord[32];
};
struct LoginResult {
	int result;

};

struct Logout {
	char userName[32];
};
struct LogoutResult {
	int result;
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
		else if(0 == strcmp(cmdBuf, "login")){
			Login login = { "Ada","123456" };
			DataHeader dh = { sizeof(login),CMD_LOGIN };
			send(_sock, (const char*)&dh, sizeof(dh), 0);
			send(_sock, (const char*)&login, sizeof(login), 0);
			//接收服务器返回数据
			DataHeader retHeader = {};
			LoginResult loginRet = {};
			recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
			recv(_sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LoginReuslt：%d \n", loginRet.result);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			Logout logout = {};
			DataHeader dh = { sizeof(logout), CMD_LOGOUT };
			send(_sock, (const char*)&dh, sizeof(dh), 0);
			send(_sock, (const char*)&logout, sizeof(logout), 0);
			DataHeader retHeader = {};
			LogoutResult logoutRet = {};
			recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
			recv(_sock, (char*)&logoutRet, sizeof(logoutRet), 0);
			printf("LogoutReuslt：%d \n", logoutRet.result);
		}
		else {
			printf("输入有问题");
		}
	}
	//7
	closesocket(_sock);
	WSACleanup();
	getchar();
}
