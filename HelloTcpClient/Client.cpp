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
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
//数据包头
struct DataHeader {
	short dataLength;//数据长度
	short cmd;
};
//数据包体
struct Login : public DataHeader {
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};
struct LoginResult : public DataHeader {
	LoginResult() {
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

struct Logout : public DataHeader {
	Logout() {
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};
struct LogoutResult : public DataHeader {
	LogoutResult() {
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};
struct NewUserJoin : public DataHeader {
	NewUserJoin() {
		dataLength = sizeof(LogoutResult);
		cmd = CMD_NEW_USER_JOIN;
		scok = 0;
	}
	int scok;
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
			cout << "收到exit命令，结束服务端";
			break;
		}
		else if (0 == strcmp(cmdBuf, "login")) {
			Login login;
			strcpy_s(login.userName, 128, "ada");
			strcpy_s(login.PassWord, 128, "123243");
			send(_sock, (const char*)&login, sizeof(login), 0);
			//接收服务器返回数据
			LoginResult loginRet = {};
			recv(_sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LoginReuslt：%d \n", loginRet.result);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			Logout logout;
			strcpy_s(logout.userName, 128, "ada");
			send(_sock, (const char*)&logout, sizeof(logout), 0);
			LogoutResult logoutRet = {};
			recv(_sock, (char*)&logoutRet, sizeof(logoutRet), 0);
			printf("LogoutReuslt：%d \n", logoutRet.result);
		}
		else {
			cout << "输入存在问题...\n";
		}
	}
	//7
	closesocket(_sock);
	WSACleanup();
	getchar();
}
