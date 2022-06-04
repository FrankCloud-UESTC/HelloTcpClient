#define WIN32_LEAN_AND_MEAN
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
	//2.bind 绑定用于接受客户端连接的网络端口
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
		cout << "Error, fail to bind network port..." << endl;
	}
	else {
		cout << "成功绑定网络端口..." << endl;
	}
	//3.listen 监听网络端口
	if (SOCKET_ERROR == listen(_sock, 5)) {
		cout << "Fail to listen network port..." << endl;
	}
	else {
		cout << "成功监听网络端口..." << endl;
	}
	//4.accept
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;

	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock) {
		cout << "错误, 接收到无效客户端SOCKET..." << endl;
	}
	else {
		cout << "新客户端加入：socket = " << (int)_cSock <<endl;
	}
	//char _recvBuf[128] = {};
	while (true) {
		DataHeader header = {};
		//5.接收客户端数据
		int nLen = recv(_cSock, (char*)&header, sizeof(header), 0);
		if (nLen <= 0) {
			cout << "客户端已经退出，结束";
			break;
		}
		cout << "收到命令：" << header.cmd << " 命令长度：" << header.dataLength << endl;
		switch (header.cmd) {
			case CMD_LOGIN: {
				Login login = {};
				recv(_cSock, (char*)&header, sizeof(Login), 0);
				LoginResult ret = { 0 };
				send(_cSock, (char*)&header, sizeof(DataHeader), 0);
				send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
			}
							break;
			case CMD_LOGOUT: {
				Logout logout = {};
				recv(_cSock, (char*)&header, sizeof(logout), 0);
				LogoutResult ret = { 0 };
				send(_cSock, (char*)&header, sizeof(header), 0);
				send(_cSock, (char*)&ret, sizeof(ret), 0);
			}
							 break;
			default:
				header.cmd = CMD_ERROR;
				header.dataLength = 0;
				send(_cSock, (char*)&header, sizeof(header), 0);
			break;
		}
		
	}

	//7关闭socket
	closesocket(_sock);
	WSACleanup();
	getchar();
}
