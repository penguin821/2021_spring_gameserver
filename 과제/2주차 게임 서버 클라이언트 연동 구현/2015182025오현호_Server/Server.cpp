#include <iostream>
#include <WS2tcpip.h>
#include <ctime>

#pragma comment(lib,"WS2_32.lib")

using namespace std;

constexpr short PORT_NUM = 9000;

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
	wcout << L"에러 " << lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
}

struct Vec2
{
	short x;
	short y;
};

int main()
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 0), &wsadata);
	wcout.imbue(std::locale("korean"));
	srand(time(NULL));

	SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, NULL, NULL, NULL, NULL);
	SOCKADDR_IN svr_addr;
	ZeroMemory(&svr_addr, sizeof(svr_addr));
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons(PORT_NUM);
	svr_addr.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(s_socket, reinterpret_cast<SOCKADDR*>(&svr_addr), sizeof(svr_addr));
	listen(s_socket, SOMAXCONN);

	SOCKADDR_IN c_addr;
	ZeroMemory(&c_addr, sizeof(c_addr));
	c_addr.sin_family = AF_INET;
	c_addr.sin_port = htons(PORT_NUM);
	int addr_l = sizeof(c_addr);

	SOCKET c_socket = WSAAccept(s_socket, reinterpret_cast<SOCKADDR*>(&c_addr), &addr_l, NULL, NULL);
	if (SOCKET_ERROR == c_socket)
	{
		error_display("accept from server", WSAGetLastError());
		exit(-1);
	}
	else
	{
		cout << "성공\n";
	}

	Vec2 pos;
	pos.x = (rand() % 8) * 60;
	pos.y = (rand() % 8) * 60;

	while (true)
	{
		WSABUF s_wsabuf;
		s_wsabuf.buf = reinterpret_cast<char*>(&pos);
		s_wsabuf.len = sizeof(pos);

		DWORD s_bytes;
		int retval = WSASend(c_socket, &s_wsabuf, 1, &s_bytes, NULL, NULL, NULL);
		if (SOCKET_ERROR == retval)
		{
			error_display("send from server", WSAGetLastError());
			exit(-1);
		}

		char r_key;
		WSABUF r_wsabuf;
		r_wsabuf.buf = reinterpret_cast<char*>(&r_key);
		r_wsabuf.len = 1;

		DWORD r_bytes;
		DWORD r_flag = 0;

		retval = WSARecv(c_socket, &r_wsabuf, 1, &r_bytes, &r_flag, NULL, NULL);
		if (SOCKET_ERROR == retval)
		{
			error_display("recv from server", WSAGetLastError());
			exit(-1);
		}

		switch (r_key)
		{
		case 'w':
		case 'W':
			if (0 != pos.y)
				pos.y -= 60;
			break;
		case 's':
		case 'S':
			if (7 != pos.y)
				pos.y += 60;
			break;
		case 'a':
		case 'A':
			if (0 != pos.x)
				pos.x -= 60;
			break;
		case 'd':
		case 'D':
			if (7 != pos.x)
				pos.x += 60;
			break;
		}
	}
	closesocket(c_socket);
	closesocket(s_socket);
	WSACleanup();
}