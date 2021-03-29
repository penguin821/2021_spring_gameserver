#include <iostream>
#include <map>
using namespace std;
#include <WS2tcpip.h>
#include <ctime>
#include <vector>
#pragma comment(lib, "Ws2_32.lib")

constexpr int MAX_BUFFER = 1024;
constexpr short SERVER_PORT = 3500;
constexpr short MAX_PLAYER = 10;

struct Vec2
{
	short x;
	short y;
};

struct all_player_info
{
	Vec2 pos;
};
all_player_info players[10];

struct SESSION // 세션
{
	WSAOVERLAPPED overlapped; // 얘를 맨 위로하면 세션의 주소랑 같아짐
	// 구조체의 첫 맴버 변수와 구조체 변수의 주소값은 같아 = C 표준
	WSABUF dataBuffer;
	SOCKET socket;

	Vec2 pos;
	char dir;
	short count;
};

map <SOCKET, SESSION> clients;

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	SOCKET client_s = reinterpret_cast<SESSION*>(overlapped)->socket; // 이렇게 소켓 정보 읽어옴

	if (dataBytes == 0) // 읽은 데이터 량
	{
		closesocket(clients[client_s].socket);
		players[clients[client_s].count].pos.x = -600;
		players[clients[client_s].count].pos.y = -600;
		clients.erase(client_s);
		return;
	}  // 클라이언트가 closesocket을 했을 경우
	cout << "From client [" << client_s << "] direction = " << clients[client_s].dir << endl;
	clients[client_s].dataBuffer.len = dataBytes;

	switch (clients[client_s].dir)
	{
	case 'w':
	case 'W':
		if (0 != clients[client_s].pos.y)
			clients[client_s].pos.y -= 60;
		break;
	case 's':
	case 'S':
		if (420 != clients[client_s].pos.y)
			clients[client_s].pos.y += 60;
		break;
	case 'a':
	case 'A':
		if (0 != clients[client_s].pos.x)
			clients[client_s].pos.x -= 60;
		break;
	case 'd':
	case 'D':
		if (420 != clients[client_s].pos.x)
			clients[client_s].pos.x += 60;
		break;
	}

	memset(&(clients[client_s].overlapped), 0, sizeof(WSAOVERLAPPED));
	Vec2 n_pos = clients[client_s].pos;
	players[clients[client_s].count].pos.x = clients[client_s].pos.x;
	players[clients[client_s].count].pos.y = clients[client_s].pos.y;

	clients[client_s].dataBuffer.buf = reinterpret_cast<char*>(&players); // 한 세션 생성 끝
	clients[client_s].dataBuffer.len = sizeof(players);
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
	}  // 클라이언트가 closesocket을 했을 경우

	// WSASend(응답에 대한)의 콜백일 경우
	cout << "TRACE - Send to " << client_s << "/ position : " << clients[client_s].pos.x << " , " << clients[client_s].pos.y << endl;

	memset(&(clients[client_s].overlapped), 0, sizeof(WSAOVERLAPPED));
	clients[client_s].dataBuffer.buf = reinterpret_cast<char*>(&clients[client_s].dir);
	clients[client_s].dataBuffer.len = 1;
	DWORD flags = 0; // 리시브는 무조건 플레그 필요 꼭 기억!
	WSARecv(client_s, &clients[client_s].dataBuffer, 1, 0, &flags, overlapped, recv_callback);
}

int main()
{
	srand(time(NULL));
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)); // c++11에 bind 키워드가 있음
	// 키워드가 아니라 함수임을 알리는 용도
	listen(listenSocket, 10);
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);

	for (int i = 0; i < 10; i++)
	{
		players[i].pos.x = -600;
		players[i].pos.y = -600;
	}

	static int count = 0;
	while (true)
	{
		if (clients.size() <= 10)
		{
			SOCKET clientSocket = WSAAccept(listenSocket, (struct sockaddr*)&clientAddr, &addrLen, NULL, NULL);
			clients[clientSocket] = SESSION{};
			clients[clientSocket].socket = clientSocket;
			clients[clientSocket].pos.x = (rand() % 8) * 60;
			clients[clientSocket].pos.y = (rand() % 8) * 60;
			clients[clientSocket].count = count;
			clients[clientSocket].dataBuffer.len = sizeof(clients[clientSocket].pos);
			Vec2 n_pos;
			n_pos.x = clients[clientSocket].pos.x;
			n_pos.y = clients[clientSocket].pos.y;

			players[clients[clientSocket].count].pos.x = clients[clientSocket].pos.x;
			players[clients[clientSocket].count].pos.y = clients[clientSocket].pos.y;

			clients[clientSocket].dataBuffer.buf = reinterpret_cast<char*>(&players); // 한 세션 생성 끝
			clients[clientSocket].dataBuffer.len = sizeof(players); // 한 세션 생성 끝
			memset(&clients[clientSocket].overlapped, 0, sizeof(WSAOVERLAPPED));

			cout << "New Client [" << clientSocket << "] connected!\n";
			WSASend(clients[clientSocket].socket, &clients[clientSocket].dataBuffer, 1, NULL,
				0, &(clients[clientSocket].overlapped), send_callback);

			count++;
		}
	}
	closesocket(listenSocket);
	WSACleanup();
}

