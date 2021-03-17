#include <iostream>
#include <WS2tcpip.h>

using namespace std;

#pragma comment(lib,"WS2_32.lib")

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

int main()
{
	wcout.imbue(std::locale("korean"));
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 0), &wsadata);

	SOCKET s_sock = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, 0);
	SOCKADDR_IN svr_addr;
	memset(&svr_addr, 0, sizeof(svr_addr));
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons(9000); // 네트웍 관련해서 숫자 넣을땐 네트워크 포멧으로
	inet_pton(AF_INET, "127.0.0.1", &svr_addr.sin_addr);

	WSAConnect(s_sock, reinterpret_cast<SOCKADDR*>(&svr_addr), sizeof(svr_addr), NULL, NULL, NULL, NULL);
;	while (true)
	{
		char s_mess[200];
		cout << "enter message : ";
		cin.getline(s_mess, 199);
		WSABUF s_wsabuf[1];
		s_wsabuf[0].buf = s_mess;
		s_wsabuf[0].len = static_cast<int>(strlen(s_mess) + 1); // get 199는 맨 끝에 0 붙일수 있게 여분 둔것, 보낼땐 전부다 보내야함
		DWORD sent_bytes;
		WSASend(s_sock, s_wsabuf, 1, &sent_bytes, 0, 0, 0);
		
		char r_mess[200];
		WSABUF r_wsabuf[1];
		r_wsabuf[0].buf = r_mess;
		r_wsabuf[0].len = 200;
		DWORD recv_bytes;
		DWORD r_flag = 0;
		int retval = WSARecv(s_sock, r_wsabuf, 1, &recv_bytes, &r_flag, 0, 0);
		if (SOCKET_ERROR == retval)
		{
			error_display("recv from server", WSAGetLastError());
			exit(-1);
		}
		cout << "server sent : " << r_mess << endl;
	}
	closesocket(s_sock);
	WSACleanup();
}