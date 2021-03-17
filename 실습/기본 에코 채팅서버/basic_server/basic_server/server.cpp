#include <iostream>
#include <WS2tcpip.h>

using namespace std;

#pragma comment(lib,"WS2_32.lib")

int main()
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 0), &wsadata); // �긦 �����ؾ� ���� ���α׷��� �ϰڴٴ°� ��ǻ�Ϳ� �˸�

	SOCKET s_sock = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, 0);
	SOCKADDR_IN svr_addr;
	memset(&svr_addr, 0, sizeof(svr_addr));
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons(9000); // ��Ʈ�� �����ؼ� ���� ������ ��Ʈ��ũ ��������
	svr_addr.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(s_sock, reinterpret_cast<SOCKADDR*>(&svr_addr), sizeof(svr_addr));
	listen(s_sock, SOMAXCONN);

	SOCKADDR_IN cl_addr;
	memset(&cl_addr, 0, sizeof(cl_addr));
	cl_addr.sin_family = AF_INET;
	cl_addr.sin_port = htons(9000); // ��Ʈ�� �����ؼ� ���� ������ ��Ʈ��ũ ��������
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

		r_wsabuf[0].len = recv_bytes; // ���� ��ŭ�� ���������
		DWORD sent_bytes;
		WSASend(c_sock, r_wsabuf, 1, &sent_bytes, 0, 0, 0);
	}
	closesocket(c_sock);
	closesocket(s_sock);
	WSACleanup();
}