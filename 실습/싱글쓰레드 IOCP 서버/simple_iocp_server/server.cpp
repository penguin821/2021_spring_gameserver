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

	bool			m_in_game;
	char			m_name[MAX_NAME];
	short			m_x, m_y;
};

map <int, SESSION> players;

void error_display(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	cout << msg;
	wcout << L"���� " << lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
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
	{
		c2s_packet_move* p = reinterpret_cast<c2s_packet_move*>(packet);
		player_move(p_id, p->dir);
	}
	break;
	default:
		cout << "unknown packet type! [" << p->type << "] Error!\n";
	}
}

void do_recv(int p_id)
{
	SESSION& p = players[p_id];
	EX_OVER& p_ro = p.m_recv_over;
	//p_ro.m_op = OP_RECV;
	memset(&p_ro.m_over, 0, sizeof(p_ro.m_over));
	p_ro.m_wsabuf[0].buf = reinterpret_cast<CHAR*>(p_ro.m_netbuf) + p.m_prev_recv;
	p_ro.m_wsabuf[0].len = MAX_BUFFER - p.m_prev_recv;
	DWORD r_flag = 0;
	WSARecv(p.m_s, p_ro.m_wsabuf, 1, 0, &r_flag, &p_ro.m_over, 0);
}

int get_new_player_id()
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (0 == players.count(i))
			return i;
	}
	return -1;
}

void do_accept(SOCKET s_sock, SOCKET* c_sock, EX_OVER* a_over)
{
	*c_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&a_over->m_over, 0, sizeof(a_over->m_over));
	DWORD num_byte;
	int ret = AcceptEx(s_sock, *c_sock, a_over->m_netbuf, 0, 16, 16, &num_byte, &a_over->m_over);
	if (FALSE == ret)
	{
		int err = WSAGetLastError();
		error_display("Accept EX : ", err);
		exit(-1);
	}
}

void disconnect(int p_id)
{
	players[p_id].m_in_game = false;
	closesocket(players[p_id].m_s);
	players.erase(p_id);
}

int main()
{
	wcout.imbue(locale("korean"));
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

	SOCKET c_sock;
	EX_OVER a_over;
	do_accept(listenSocket, &c_sock, &a_over);

	while (true)
	{
		DWORD num_byte;
		ULONG_PTR i_key;
		WSAOVERLAPPED* over;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_byte, &i_key, &over, INFINITE);
		int key = static_cast<int>(i_key);
		if (FALSE == ret) // GetQueuedCompletionStatus�� ������ �Լ���
		{
			disconnect(key);
			continue;
		}
		EX_OVER* ex_over = reinterpret_cast<EX_OVER*>(over);
		switch (ex_over->m_op)
		{
		case OP_RECV:
		{
			unsigned char* ps = ex_over->m_netbuf;
			int remain_data = num_byte + players[key].m_prev_recv;
			while (remain_data > 0)
			{
				int packet_size = ps[0];
				if (packet_size > remain_data)
					break;
				process_packet(key, ps);
				remain_data -= packet_size;
				ps += packet_size;
			}
			if (remain_data > 0) // 0�̵� 0 �ƴϵ� ������ �������
				memcpy(ex_over->m_netbuf, ps, remain_data);
			players[key].m_prev_recv = remain_data;
			do_recv(key);
		}
		break;
		case OP_SEND:
		{
			if (num_byte != ex_over->m_wsabuf[0].len)
				disconnect(key);
			delete ex_over;
			break;
		}
		break;
		case OP_ACCEPT:
		{
			int p_id = get_new_player_id();
			if (-1 == p_id)
			{
				closesocket(c_sock);
				do_accept(listenSocket, &c_sock, &a_over);
				continue;
			}
			SESSION t; //
			t.m_in_game = false; //
			players[p_id] = t; // Ŭ�� ��ü Ŭ������ �����
			SESSION& n_s = players[p_id];
			n_s.m_id = p_id;
			n_s.m_prev_recv = 0;
			n_s.m_recv_over.m_op = OP_RECV;
			n_s.m_s = c_sock;

			CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_sock), h_iocp, p_id, 0);
			do_recv(p_id);
			do_accept(listenSocket, &c_sock, &a_over);
			cout << "New Client [" << p_id << "] connected!\n";
		}
		break;
		default:
			cout << "Unknown GQCS Error!\n";
			exit;
		}
	}
	closesocket(listenSocket);
	WSACleanup();
}

