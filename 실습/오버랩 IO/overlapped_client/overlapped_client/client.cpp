/*
## ���� ���� : 1 v n - overlapped callback
1. socket()            : ���ϻ���
2. connect()        : �����û
3. read()&write()
    WIN recv()&send    : ������ �а���
4. close()
    WIN closesocket    : ��������
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
        // 3-2. ������ �б�
        int receiveBytes = recv(clientSocket, messageBuffer, MAX_BUFFER, 0);
        if (receiveBytes > 0)
        {
            printf("TRACE - Receive message : %s (%d bytes)\n* Enter Message\n->", messageBuffer, receiveBytes);
        }
    }
}

int main()
{
    // Winsock Start - winsock.dll �ε�
    WSADATA WSAData;
    if (WSAStartup(MAKEWORD(2, 0), &WSAData) != 0)
    {
        printf("Error - Can not load 'winsock.dll' file\n");
        return 1;
    }

    // 1. ���ϻ���
    = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (clientSocket == INVALID_SOCKET)
    {
        printf("Error - Invalid socket\n");
        return 1;
    }

    // �������� ��ü����
    SOCKADDR_IN serverAddr;
    memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP); // ������ ���� ���ɼ� ����

    // 2. �����û
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        printf("Error - Fail to connect\n");
        // 4. ��������
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
        // �޽��� �Է�
        std::cin.getline(messageBuffer, MAX_BUFFER - 1);
        int bufferLen = strlen(messageBuffer)+1; // strlen�� \0�� ���� ���ϰ� ī��Ʈ�ϱ� ������ +1 �������

        socketInfo = (struct SOCKETINFO*)malloc(sizeof(struct SOCKETINFO));
        memset((void*)socketInfo, 0x00, sizeof(struct SOCKETINFO));
        socketInfo->dataBuffer.len = bufferLen;
        socketInfo->dataBuffer.buf = messageBuffer;

        // 3-1. ������ ����
        int sendBytes = WSASend(clientSocket, &socketInfo->dataBuffer, 1, NULL,0, &socketInfo->overlapped,recv_cb);
    }
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}