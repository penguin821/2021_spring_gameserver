#include <iostream>
#include <WS2tcpip.h>

using namespace std;

#pragma comment(lib,"WS2_32.lib")

int main()
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 0), &wsadata); // 얘를 선언해야 소켓 프로그래밍 하겠다는걸 컴퓨터에 알림

	SOCKET s_sock = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, 0);
	SOCKADDR_IN svr_addr;
	memset(&svr_addr, 0, sizeof(svr_addr));
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons(9000); // 네트웍 관련해서 숫자 넣을땐 네트워크 포멧으로
	svr_addr.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(s_sock, reinterpret_cast<SOCKADDR*>(&svr_addr), sizeof(svr_addr));
	listen(s_sock, SOMAXCONN);

	SOCKADDR_IN cl_addr;
	memset(&cl_addr, 0, sizeof(cl_addr));
	cl_addr.sin_family = AF_INET;
	cl_addr.sin_port = htons(9000); // 네트웍 관련해서 숫자 넣을땐 네트워크 포멧으로
	inet_pton(AF_INET, "127.0.0.1", &cl_addr.sin_addr);
	int addr_l = sizeof(cl_addr);


	SOCKET c_sock = WSAAccept(s_sock, reinterpret_cast<SOCKADDR*>(&cl_addr), &addr_l, NULL, NULL);

	while (true)
	{
		char r_mess[200];
		WSABUF r_wsabuf[1];
		r_wsabuf[0].buf = r_mess;
		r_wsabuf[0].len = 200;
		DWORD recv_bytes;
		DWORD r_flag = 0;
		WSARecv(c_sock, r_wsabuf, 1, &recv_bytes, &r_flag, 0, 0);

		cout << "client sent : " << r_mess << endl;

		r_wsabuf[0].len = recv_bytes; // 받은 만큼만 돌려줘야함
		DWORD sent_bytes;
		WSASend(c_sock, r_wsabuf, 1, &sent_bytes, 0, 0, 0);
	}
	closesocket(c_sock);
	closesocket(s_sock);
	WSACleanup();
}