/*
## 소켓 서버 : 1 v n - overlapped callback
1. socket()            : 소켓생성
2. connect()        : 연결요청
3. read()&write()
    WIN recv()&send    : 데이터 읽고쓰기
4. close()
    WIN closesocket    : 소켓종료
*/

#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER        1024
#define SERVER_IP        "127.0.0.1"
#define SERVER_PORT        3500

struct SOCKETINFO
{
    WSAOVERLAPPED overlapped;
    WSABUF dataBuffer;
    int receiveBytes;
    int sendBytes;
};

SOCKET clientSocket;
SOCKETINFO* socketInfo;
char messageBuffer[MAX_BUFFER];

void CALLBACK send_cb(DWORD err, DWORD num_byte, LPOVERLAPPED over, DWORD recv_flag)
{
    if (num_byte > 0)
    {
        printf("TRACE - Send message : %s (%d bytes)\n", messageBuffer, sendBytes);
        // 3-2. 데이터 읽기
        int receiveBytes = recv(clientSocket, messageBuffer, MAX_BUFFER, 0);
        if (receiveBytes > 0)
        {
            printf("TRACE - Receive message : %s (%d bytes)\n* Enter Message\n->", messageBuffer, receiveBytes);
        }
    }
}

int main()
{
    // Winsock Start - winsock.dll 로드
    WSADATA WSAData;
    if (WSAStartup(MAKEWORD(2, 0), &WSAData) != 0)
    {
        printf("Error - Can not load 'winsock.dll' file\n");
        return 1;
    }

    // 1. 소켓생성
    = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (clientSocket == INVALID_SOCKET)
    {
        printf("Error - Invalid socket\n");
        return 1;
    }

    // 서버정보 객체설정
    SOCKADDR_IN serverAddr;
    memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP); // 컴파일 에러 가능성 있음

    // 2. 연결요청
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        printf("Error - Fail to connect\n");
        // 4. 소켓종료
        closesocket(clientSocket);
        // Winsock End
        WSACleanup();
        return 1;
    }
    else
    {
        printf("Server Connected\n* Enter Message\n->");
    }

    DWORD sendBytes;
    DWORD receiveBytes;
    DWORD flags;

    while (1)
    {
        // 메시지 입력
        std::cin.getline(messageBuffer, MAX_BUFFER - 1);
        int bufferLen = strlen(messageBuffer)+1; // strlen는 \0을 포함 안하고 카운트하기 때문에 +1 해줘야함

        socketInfo = (struct SOCKETINFO*)malloc(sizeof(struct SOCKETINFO));
        memset((void*)socketInfo, 0x00, sizeof(struct SOCKETINFO));
        socketInfo->dataBuffer.len = bufferLen;
        socketInfo->dataBuffer.buf = messageBuffer;

        // 3-1. 데이터 쓰기
        int sendBytes = WSASend(clientSocket, &socketInfo->dataBuffer, 1, NULL,0, &socketInfo->overlapped,recv_cb);
    }
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}