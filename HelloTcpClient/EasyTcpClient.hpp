#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_


#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include<iostream>
#include "MessageHeader.hpp"
#pragma comment(lib, "ws2_32.lib")
using namespace std;

class EasyTcpClient {
public:
	SOCKET _sock;
	EasyTcpClient() {
		_sock = INVALID_SOCKET;
	}

	virtual ~EasyTcpClient() {
		Close();
	}
	//初始化socket
	void InitSocket() {
		//启动Win Sock 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);

		if (INVALID_SOCKET != _sock) {
			printf("关闭旧socket = %d\n", _sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET != _sock) {
			printf("建立socket成功\n");
		}
	}
	//连接服务器
	int Connect(const char* ip, unsigned short port) {
		if (INVALID_SOCKET == _sock) {
			InitSocket();
		}
		//2.connect
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret) {
			cout << "Error, 连接服务器失败..." << endl;
		}
		else {
			cout << "Success 连接服务器成功..." << endl;
		}
		return ret;
	}
	//关闭socket
	void Close() {
		//关闭Win Sock 2.x环境
		if (_sock != INVALID_SOCKET) {
			closesocket(_sock);
			WSACleanup();
			_sock = INVALID_SOCKET;
		}
		
	}
	
	//处理网络信息
	bool OnRun() {
		if (isRun()) {
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 1,0 };
			int ret = select(_sock, &fdReads, 0, 0, &t);
			if (ret < 0) {
				printf("<socket = %d>select任务结束1\n", _sock);
				return false;
			}
			if (FD_ISSET(_sock, &fdReads)) {
				FD_CLR(_sock, &fdReads);

				if (-1 == RecvData(_sock)) {
					printf("<socket = %d>select任务结束2\n", _sock);
					return false;
				}
			}
			return true;
		}
		return false;
	}
	//是否工作中
	bool isRun() {
		return _sock != INVALID_SOCKET;
	}
	//接受数据 处理粘包 拆封包
	int RecvData(SOCKET _cSock) {
		//缓冲区
		char szRecv[4096] = {};
		// 5 接收客户端数据
		int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0){
			printf("与服务器断开连接，任务结束。\n");
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(header);
		return 0;
	}
	//响应网络消息
	void OnNetMsg(DataHeader* header) {
		switch (header->cmd) {
		case CMD_LOGIN_RESULT: {
			LoginResult* login = (LoginResult*)header;
			printf("收到服务端消息：CMD_LOGIN_RESULT,数据长度：%d\n", login->dataLength);
		}
							 break;
		case CMD_LOGOUT_RESULT: {
			LogoutResult* logout = (LogoutResult*)header;
			printf("收到服务端消息：CMD_LOGOUT_RESULT,数据长度：%d\n", logout->dataLength);
		}
							  break;
		case CMD_NEW_USER_JOIN: {
			NewUserJoin* userJoin = (NewUserJoin*)header;
			printf("收到服务端消息：CMD_NEW_USER_JOIN,数据长度：%d\n", userJoin->dataLength);
		}
							  break;
		}
	}
	//发送数据
	int SendData(DataHeader* header) {
		if (isRun() && header) {
			send(_sock, (const char*)&header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
private:

};
#endif // !_EasyTcpClient_hpp_