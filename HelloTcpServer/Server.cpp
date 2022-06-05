#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include<iostream>
#include<vector>

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
//���ݰ�ͷ
struct DataHeader {
	short dataLength;//���ݳ���
	//short MaxLength;
	short cmd;
};
//���ݰ���
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
vector<SOCKET> g_clients;

int processor(SOCKET _cSock) {
	//������
	char szRecv[4096] = {};
	//5.���տͻ�������
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0) {
		cout << "�ͻ���<Socket = %d>�Ѿ��˳�������" << endl;
		return -1;
	}
	if (nLen >= sizeof(DataHeader)) {}
	switch (header->cmd) {
	case CMD_LOGIN: {
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		printf("�յ����CMD_LOGIN, ���ݳ��ȣ�%d, username = %s PassWord = %s\n", login->dataLength, login->userName, login->PassWord);
		LoginResult ret;
		send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
	}
				  break;
	case CMD_LOGOUT: {
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Logout* logout = (Logout*)szRecv;
		printf("�յ����CMD_LOGOUT, ���ݳ��ȣ�%d, username = %s\n", logout->dataLength, logout->userName);
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
int main() {
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	//1.build a socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2.bind �����ڽ��ܿͻ������ӵ�����˿�
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
		cout << "Error, fail to bind network port..." << endl;
	}
	else {
		cout << "�ɹ�������˿�..." << endl;
	}
	//3.listen ��������˿�
	if (SOCKET_ERROR == listen(_sock, 5)) {
		cout << "Fail to listen network port..." << endl;
	}
	else {
		cout << "�ɹ���������˿�..." << endl;
	}

	//char _recvBuf[128] = {};
	while (true) {
		//������ socket
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
		//select��һ������nfds��һ������ֵ ��ָfd_set������������������socket���ķ�Χ������������
		//���������ļ����������ֵ+1 ��Windows������������������ȡֵ
		timeval t = { 0,0 };
		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);//NULL��ʾ��Զ����ֱ�����¼�����
		if (ret < 0) {
			cout << "select�������" << endl;
			break;
		}
		if (FD_ISSET(_sock, &fdRead)) {
			FD_CLR(_sock, &fdRead);
			//4.accept
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _cSock = INVALID_SOCKET;
			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _cSock) {
				cout << "����, ���յ���Ч�ͻ���SOCKET..." << endl;
			}
			for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
				NewUserJoin userJoin;
				send(g_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
			}
			cout << "�¿ͻ��˼��룺socket = " << (int)_cSock <<  " IP = " << inet_ntoa(clientAddr.sin_addr) << endl;
			g_clients.push_back(_cSock);
		}
		for (size_t n = 0; n < fdRead.fd_count; n++) {
			if (-1 == processor(fdRead.fd_array[n])) {
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end()) {
					g_clients.erase(iter);
				}
			}
		}
		

	}

	for (size_t n = g_clients.size() - 1; n >= 0; n--) {
		closesocket(g_clients[n]);
	}
	//7�ر�socket
	closesocket(_sock);
	WSACleanup();
	getchar();
	return 0;
}
