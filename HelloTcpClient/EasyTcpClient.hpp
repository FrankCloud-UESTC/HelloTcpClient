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
	SOCKET InitSocket() {
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
			printf("建立socket<socket = %d>成功\n", _sock);
		}
		return _sock;
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
	
	//处理网络信息
	bool OnRun() {
		if (isRun()) {
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 0,0 };
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
	//缓冲区最小单元大小
#ifndef  RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240
#endif // ! RECV_BUFF_SZIE

	//接受缓冲区
	char _szRecv[RECV_BUFF_SZIE] = {};
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SZIE * 10] = {};
	//消息缓冲区尾部位置
	int _lastPos = 0;
	//接收数据 处理粘包 拆分包
	int RecvData(SOCKET _cSock) {
		// 5 接收客户端数据
		int nLen = (int)recv(_cSock, _szRecv, RECV_BUFF_SZIE, 0);
		if (nLen <= 0) {
			printf("客户端<socket=%d>已退出，任务结束。\n", _cSock);
			return -1;
		}
		memcpy(_szMsgBuf + _lastPos, _szRecv, nLen);
		//消息缓冲区的数据尾部位置后移
		_lastPos += nLen;
		//判断消息缓冲区的长度是否大于消息头长度
		
		while (_lastPos >= sizeof(DataHeader)) {
			//这时就可知道消息体长度
			DataHeader* header = (DataHeader*)_szMsgBuf;
			if (_lastPos > header->dataLength) {
				//剩余未处理消息缓冲区数据的长度
				int nSize = _lastPos - header->dataLength;
				//处理网络消息
				OnNetMsg(header);
				//将消息缓冲区剩余未处理数据前移
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, _lastPos - header->dataLength);
				//消息缓冲区的数据尾部位置后移
				_lastPos = nSize;
			}
			else {
				//剩余数据不够一条完整消息
				break;
			}
		}
		/*
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0){
			printf("与服务器断开连接，任务结束。\n");
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(header);*/
		return 0;
	}
	//响应网络消息
	virtual void OnNetMsg(DataHeader* header) {
		switch (header->cmd) {
		case CMD_LOGIN_RESULT: {
			LoginResult* login = (LoginResult*)header;
			//printf("<socket=%d>收到服务端消息：CMD_LOGIN_RESULT,数据长度：%d\n", _sock, login->dataLength);
		}
		break;
		case CMD_LOGOUT_RESULT: {
			LogoutResult* logout = (LogoutResult*)header;
			//printf("<socket=%d>收到服务端消息：CMD_LOGOUT_RESULT,数据长度：%d\n", _sock, logout->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN: {
			NewUserJoin* userJoin = (NewUserJoin*)header;
			//printf("<socket=%d>收到服务端消息：CMD_NEW_USER_JOIN,数据长度：%d\n", _sock, userJoin->dataLength);
		}
		break;
		case CMD_ERROR: {
			//printf("<socket=%d>收到服务端消息：CMD_NEW_USER_JOIN,数据长度：%d\n", _sock, header->dataLength);
		}
		break;
		default:
			printf("<socket=%d>收到未定义消息,数据长度：%d\n", _sock, header->dataLength);
		}
		
	}
	//发送数据
	int SendData(DataHeader* header) {
		if (isRun() && header) {
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
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
private:

};
#endif // !_EasyTcpClient_hpp_