#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
#endif // !_EasyTcpClient_hpp_

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

class EasyTcpServer {
public:
	EasyTcpServer() {

	}
	virtual ~EasyTcpServer() {

	}
	//初始化socket
	void InitSocket() {
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
		SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	//绑定IP和端口号
	int Bind(const char* ip, unsigned short port) {
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
		_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret) {
			cout << "Error, fail to bind network port..." << endl;
		}
		else {
			cout << "成功绑定网络端口..." << endl;
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
	}
	int Accept() {
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
		if (INVALID_SOCKET == _cSock) {
			cout << "错误, 接收到无效客户端SOCKET..." << endl;
		}
		for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
			NewUserJoin userJoin;
			send(g_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
		}
		cout << "新客户端加入：socket = " << (int)_cSock << " IP = " << inet_ntoa(clientAddr.sin_addr) << endl;
		g_clients.push_back(_cSock);
		return _cSock;
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

			for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
				FD_SET(g_clients[n], &fdRead);
			}
			//select第一个参数nfds是一个整数值 是指fd_set集合中所有描述符（socket）的范围，而不是数量
			//即是所有文件描述符最大值+1 在Windows中这个参数则可以任意取值
			timeval t = { 0,0 };
			int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);//NULL表示永远阻塞直到有事件发生
			if (ret < 0) {
				cout << "select任务结束" << endl;
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdRead)) {
				FD_CLR(_sock, &fdRead);
				//4.accept

			}
			for (size_t n = 0; n < fdRead.fd_count; n++) {
				if (-1 == RecvData(fdRead.fd_array[n])) {
					auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
					if (iter != g_clients.end()) {
						g_clients.erase(iter);
					}
				}
			}
		}
		return isRun();
		
	}
	bool isRun() {
		return INVALID_SOCKET != _sock;
	}
	//
	int RecvData(SOCKET _cSock) {
		//缓冲区
		char szRecv[4096] = {};
		//5.接收客户端数据
		int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0) {
			printf("客户端<Socket = %d>已经退出，结束\n", _cSock);
			return -1;
		}
		//if (nLen >= sizeof(DataHeader)) {}

		switch (header->cmd) {
		case CMD_LOGIN: {
			recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			Login* login = (Login*)szRecv;
			printf("收到命令：CMD_LOGIN, 数据长度：%d, username = %s PassWord = %s\n", login->dataLength, login->userName, login->PassWord);
			LoginResult ret;
			send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
		}
					  break;
		case CMD_LOGOUT: {
			recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			Logout* logout = (Logout*)szRecv;
			printf("收到命令：CMD_LOGOUT, 数据长度：%d, username = %s\n", logout->dataLength, logout->userName);
			LogoutResult ret;
			send(_cSock, (const char*)&ret, sizeof(ret), 0);
		}
					   break;
		default:
			DataHeader header = { 0, CMD_ERROR };
			send(_cSock, (char*)&header, sizeof(header), 0);
			break;
		}
	}
	//关闭
	void Close() {
		for (size_t n = g_clients.size() - 1; n >= 0; n--) {
			closesocket(g_clients[n]);
		}
		//7关闭socket
		closesocket(_sock);
		WSACleanup();
	}
private:
	SOCKET  _sock;
	vector<SOCKET>g_clients;
};