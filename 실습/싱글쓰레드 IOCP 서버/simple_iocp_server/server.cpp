#include <iostream>
#include <map>
using namespace std;
#include <WS2tcpip.h>
#include <MSWSock.h>
#include "protocol.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "MSWSock.lib")

enum OP_TYPE { OP_RECV, OP_SEND, OP_ACCEPT, OP_END };

struct EX_OVER 
{
	WSAOVERLAPPED	m_over; // �긦 �� �����ϸ� ������ �ּҶ� ������
// ����ü�� ù �ɹ� ������ ����ü ������ �ּҰ��� ���� = C ǥ��
	WSABUF			m_wsabuf[1];
	unsigned char	m_netbuf[MAX_BUFFER]; // �ִ� 225����Ʈ���� �ް� �ϱ�����
	OP_TYPE			m_op;
};

struct SESSION // ����
{
	int				m_id;
	EX_OVER			m_recv_over;
	unsigned char	m_prev_recv;
	SOCKET			m_s;

	char			m_name[MAX_NAME];
	short			m_x, m_y;
};

map <int, SESSION> players;

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	SOCKET client_s = reinterpret_cast<SESSION*>(overlapped)->socket; // �̷��� ���� ���� �о��

	if (dataBytes == 0) // ���� ������ ��
	{
		closesocket(clients[client_s].socket);
		clients.erase(client_s);
		return;
	}  // Ŭ���̾�Ʈ�� closesocket�� ���� ���
	cout << "From client [" << client_s << "]: " << clients[client_s].messageBuffer << " (" << dataBytes << ") bytes)\n";
	clients[client_s].dataBuffer.len = dataBytes;
	memset(&(clients[client_s].overlapped), 0, sizeof(WSAOVERLAPPED));
	WSASend(client_s, &(clients[client_s].dataBuffer), 1, NULL, 0, overlapped, send_callback);
}
void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	SOCKET client_s = reinterpret_cast<SESSION*>(overlapped)->socket;

	if (dataBytes == 0)
	{
		closesocket(clients[client_s].socket);
		clients.erase(client_s);
		return;
	}  // Ŭ���̾�Ʈ�� closesocket�� ���� ���

	// WSASend(���信 ����)�� �ݹ��� ���
	cout << "TRACE - Send message : " << clients[client_s].messageBuffer << " (" << dataBytes << " bytes)\n";
	memset(&(clients[client_s].overlapped), 0, sizeof(WSAOVERLAPPED));
	clients[client_s].dataBuffer.len = MAX_BUFFER;
	DWORD flags = 0; // ���ú�� ������ �÷��� �ʿ� �� ���!
	WSARecv(client_s, &clients[client_s].dataBuffer, 1, 0, &flags, overlapped, recv_callback);
}

void send_packet(int p_id, void* buf)
{
	// send �ϱ� ���� wsabuf�� �Ҵ� �޾ƾ��ϴµ� ��� ������ ������ Ȯ�忡 ������ ������ Ȯ���� new
	EX_OVER* s_over = new EX_OVER;

	unsigned char p_size = reinterpret_cast<unsigned char*>(buf)[0];
	s_over->m_op = OP_SEND;
	memset(&s_over->m_over, 0, sizeof(s_over->m_over));
	memcpy(s_over->m_netbuf, buf, p_size);

	s_over->m_wsabuf[0].buf = reinterpret_cast<char*>(s_over->m_netbuf);
	s_over->m_wsabuf[0].len = p_size;

	WSASend(players[p_id].m_s, s_over->m_wsabuf, 1, 0, 0, &s_over->m_over, 0);
}

void send_login_info(int p_id)
{
	s2c_packet_login_info p;
	p.hp = 100;
	p.id = p_id;
	p.level = 1;
	p.size = sizeof(p);
	p.type = S2C_PACKET_LOGIN_INFO;
	p.x = players[p_id].m_x;
	p.y = players[p_id].m_y;

	send_packet(p_id, &p);
}

void send_move_packet(int p_id)
{
	s2c_packet_pc_move p;
	p.id = p_id;
	p.size = sizeof(p);
	p.type = S2C_PACKET_PC_MOVE;
	p.x = players[p_id].m_x;
	p.y = players[p_id].m_y;

	send_packet(p_id, &p);
}

void player_move(int p_id, char dir)
{
	short x = players[p_id].m_x;
	short y = players[p_id].m_y;

	switch (dir)
	{
	case 0:
		if (y > 0)
			y--;
		break;
	case 1:
		if (x < BOARD_WIDTH - 1)
			x++;
		break;
	case 2:
		if (y < BOARD_HEIGHT - 1)
			y++;
		break;
	case 3:
		if (x > 0)
			x--;
		break;
	}

	players[p_id].m_x = x;
	players[p_id].m_y = y;

	send_move_packet(p_id);
}

void process_packet(int p_id, unsigned char* packet)
{
	c2s_packet_login* p = reinterpret_cast<c2s_packet_login*>(packet);

	switch (p->type)
	{
	case C2S_PACKET_LOGIN:
		strcpy_s(players[p_id].m_name, p->name);
		send_login_info(p_id);
		break;
	case C2S_PACKET_MOVE:
		c2s_packet_move* p = reinterpret_cast<c2s_packet_move*>(packet);
		player_move(p_id, p->dir);
		break;
	default:
		cout << "unknown packet type! [" << p->type << "] Error!\n";
	}
}

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)); // c++11�� bind Ű���尡 ����
	// Ű���尡 �ƴ϶� �Լ����� �˸��� �뵵
	listen(listenSocket, SOMAXCONN);

	HANDLE h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0); // iocp ��ü ����
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket), h_iocp, 100'000, 0);

	AcceptEx();

	while (true) {
		SOCKET clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &addrLen);
		clients[clientSocket] = SESSION{};
		clients[clientSocket].socket = clientSocket;
		clients[clientSocket].dataBuffer.len = MAX_BUFFER;
		clients[clientSocket].dataBuffer.buf = clients[clientSocket].messageBuffer; // �� ���� ���� ��
		memset(&clients[clientSocket].overlapped, 0, sizeof(WSAOVERLAPPED));
		DWORD flags = 0;
		WSARecv(clients[clientSocket].socket, &clients[clientSocket].dataBuffer, 1, NULL,
			&flags, &(clients[clientSocket].overlapped), recv_callback);
		cout << "New Client [" << clientSocket << "] connected!\n";
	}
	closesocket(listenSocket);
	WSACleanup();
}

