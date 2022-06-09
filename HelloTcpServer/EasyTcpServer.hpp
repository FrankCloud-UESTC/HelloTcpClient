#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
#endif // !_EasyTcpClient_hpp_
#ifndef  RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240
#endif // ! RECV_BUFF_SZIE
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include<iostream>
#include<vector>
#include"MessageHeader.hpp"
#pragma comment(lib, "ws2_32.lib")
using namespace std;
class ClientSocket {
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET) {
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
	}
	SOCKET sockfd() {
		return _sockfd;
	}
	char* msgBuf() {
		return _szMsgBuf;
	}
	int getLastPos() {
		return _lastPos;
	}
	void setLastPos(int pos) {
		_lastPos = pos;
	}
private:
	//socket fd_set file desc set
	SOCKET _sockfd;
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SZIE * 10] = {};
	//消息缓冲区尾部位置
	int _lastPos = 0;

};

//new 堆内存
class EasyTcpServer {
public:
	EasyTcpServer() {
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer() {
		Close();
	}
	//初始化socket
	SOCKET InitSocket() {
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
		
		if (INVALID_SOCKET != _sock) {
			printf("<socket=%d>关闭旧连接...\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			printf("socket启动失败\n");
		}
		printf("<SOCKET = %d>socket建立成功\n", _sock);
		return _sock;
	}
	//绑定IP和端口号
	int Bind(const char* ip, unsigned short port) {
		if (INVALID_SOCKET == _sock) {
			InitSocket();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
		if (ip) {
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret) {
			cout << "Error, fail to bind network port..." << endl;
			InitSocket();
		}
		else {
			printf("<SOCKET = %d>成功绑定网络端口...\n", _sock);
		}
		return ret;
	}
	int Listen(int n) {
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			cout << "Fail to listen network port..." << endl;
		}
		else {
			printf("<Socket = %d>成功监听网络端口...\n", _sock);
		}
		return ret;
	}
	int Accept() {
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
		if (INVALID_SOCKET == cSock) {
			cout << "错误, 接收到无效客户端SOCKET..." << endl;
		}
		else {
			NewUserJoin uesrJoin;
			SendDataToAll(&uesrJoin);
			cout << "新客户端加入：socket = " << (int)cSock << " IP = " << inet_ntoa(clientAddr.sin_addr) << endl;
			_clients.push_back(new ClientSocket(cSock));
		}
		
		return cSock;
	}
	bool OnRun() {
		if (isRun()) {
			//伯克利 socket
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExp;

			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);
			SOCKET maxSock = _sock;
			for (int n = (int)_clients.size() - 1; n >= 0; n--) {
				FD_SET(_clients[n]->sockfd(), &fdRead);
				if (maxSock < _clients[n]->sockfd()) {
					maxSock = _clients[n]->sockfd();
				}
			}
			//select第一个参数nfds是一个整数值 是指fd_set集合中所有描述符（socket）的范围，而不是数量
			//即是所有文件描述符最大值+1 在Windows中这个参数则可以任意取值
			timeval t = { 1,0 };
			int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, nullptr);//NULL表示永远阻塞直到有事件发生
			if (ret < 0) {
				cout << "select任务结束" << endl;
				Close();
				return false;
			}
			//判断描述符（socket）是否在集合中
			if (FD_ISSET(_sock, &fdRead)) {
				FD_CLR(_sock, &fdRead);
				Accept();
			}
			for (int n = (int)_clients.size() - 1; n >= 0; n--) {
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead)) {
					if (-1 == RecvData(_clients[n])) {
						auto iter = _clients.begin() + n;
						if (iter != _clients.end()) {
							delete _clients[n];
							_clients.erase(iter);
						}
					}
				}
			}
		}
		return isRun();
		
	}
	bool isRun() {
		return INVALID_SOCKET != _sock;
	}
	//缓冲区
	char _szRecv[RECV_BUFF_SZIE] = {};
	int RecvData(ClientSocket* pClient) {
		//
		
		//5.接收客户端数据
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SZIE, 0);
		if (nLen <= 0) {
			printf("客户端<socket=%d>已退出，任务结束。\n", pClient->sockfd());
			return -1;
		}
		memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);
		//消息缓冲区的数据尾部位置后移
		pClient->setLastPos(pClient->getLastPos() + nLen);
		//判断消息缓冲区的时间长度大于消息头DataHeader长度
		while (pClient->getLastPos() >= sizeof(DataHeader)) {
			//这时就可知道消息体长度
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			if (pClient->getLastPos() > header->dataLength) {
				//剩余未处理消息缓冲区数据的长度
				int nSize = pClient->getLastPos() - header->dataLength;
				//处理网络消息
				OnNetMsg(pClient->sockfd(), header);
				//将消息缓冲区剩余未处理数据前移
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//消息缓冲区的数据尾部位置后移
				pClient->setLastPos(nSize);
			}
			else {
				//剩余数据不够一条完整消息
				break;
			}
		}
		return 0;
		
	}
	//
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header) {
		switch (header->cmd) {
		case CMD_LOGIN: {
			
			Login* login = (Login*)header;
			//printf("收到命令：CMD_LOGIN, 数据长度：%d, username = %s PassWord = %s\n", login->dataLength, login->userName, login->PassWord);
			LoginResult ret;
			SendData(cSock, &ret);
		}
					  break;
		case CMD_LOGOUT: {
			
			Logout* logout = (Logout*)header;
			//printf("收到命令：CMD_LOGOUT, 数据长度：%d, username = %s\n", logout->dataLength, logout->userName);
			LogoutResult ret;
			SendData(cSock, &ret);
		}
					   break;
		default: {
			printf("<socket=%d>收到未定义消息,数据长度：%d\n", cSock, header->dataLength);
			//DataHeader ret;
			//SendData(cSock, &ret);
		}
			break;
		}
	}
	//发送指定socket数据
	int SendData(SOCKET _cSock, DataHeader* header) {
		if (isRun() && header) {
			return send(_cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	void SendDataToAll(DataHeader* header) {

		for (int n = (int)_clients.size() - 1; n >= 0; n--) {
			SendData(_clients[n]->sockfd(), header);
		}
		
	}
	//关闭
	void Close() {
		for (size_t n = _clients.size() - 1; n >= 0; n--) {
			closesocket(_clients[n]->sockfd());
			delete _clients[n];
		}
		_clients.clear();
		//7关闭socket
		closesocket(_sock);
		WSACleanup();
	}
private:
	SOCKET _sock;
	vector<ClientSocket*>_clients;
};

