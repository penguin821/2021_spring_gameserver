// 2015182025오현호_Client.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "2015182025오현호_Client.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
string client_ip;
SOCKET s_socket;

constexpr int MAX_BUFFER = 1024;
constexpr short SERVER_PORT = 3500;
bool isLogin = false;
short my_id;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
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

struct all_player_info
{
    Vec2 pos;
};
all_player_info players[10];

struct Key
{
    char key;
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.
    GdiplusStartupInput         m_GdiplusStartupInput;
    ULONG_PTR                   m_GdiplusToken;
    GdiplusStartup(&m_GdiplusToken, &m_GdiplusStartupInput, NULL);

    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 0), &wsadata);
    s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, 0);
    wcout.imbue(std::locale("korean"));

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY2015182025CLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY2015182025CLIENT));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY2015182025CLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MY2015182025CLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 495, 539, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Vec2 pos;
    static Key input_key{};
    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, 0, 0, NULL);
        break;
    case WM_CHAR:
        if (wParam == VK_RETURN && !client_ip.empty())
        {
            SOCKADDR_IN server_a;
            ZeroMemory(&server_a, sizeof(server_a));
            server_a.sin_family = AF_INET;
            server_a.sin_port = htons(SERVER_PORT);
            inet_pton(AF_INET, client_ip.c_str(), &server_a.sin_addr);

            int retval = WSAConnect(s_socket, reinterpret_cast<SOCKADDR*>(&server_a), sizeof(server_a), 0, 0, 0, 0);
            if (SOCKET_ERROR == retval)
            {
                error_display("connect from cilent", WSAGetLastError());
                exit(-1);
            }
            else
            {
                WSABUF r_wsabuf;
                r_wsabuf.buf = reinterpret_cast<char*>(&players);
                r_wsabuf.len = sizeof(players);
                DWORD recv_bytes;
                DWORD r_flag = 0;
                int retval = WSARecv(s_socket, &r_wsabuf, 1, &recv_bytes, &r_flag, 0, 0);
                if (SOCKET_ERROR == retval)
                {
                    error_display("recv from client", WSAGetLastError());
                    exit(-1);
                }
                isLogin = true;
            }
        }
        else if (wParam == VK_BACK)
        {
            if (!client_ip.empty())
                client_ip.pop_back();
        }
        else
        {
            client_ip.push_back(wParam);
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_TIMER:
    {
        switch (input_key.key)
        {
            WSABUF s_wsabuf;
            DWORD send_bytes;
        case 'w':
        {
            s_wsabuf.buf = reinterpret_cast<char*>(&input_key.key);
            s_wsabuf.len = sizeof(input_key);
            int retval = WSASend(s_socket, &s_wsabuf, 1, &send_bytes, 0, 0, 0);
            if (SOCKET_ERROR == retval)
            {
                error_display("send from client", WSAGetLastError());
                exit(-1);
            }


            WSABUF r_wsabuf;
            r_wsabuf.buf = reinterpret_cast<char*>(&players);
            r_wsabuf.len = sizeof(players);
            DWORD recv_bytes;
            DWORD r_flag = 0;
            retval = WSARecv(s_socket, &r_wsabuf, 1, &recv_bytes, &r_flag, 0, 0);
            if (SOCKET_ERROR == retval)
            {
                error_display("recv from client", WSAGetLastError());
                exit(-1);
            }
            input_key.key = 'g';
        }
        break;
        case 's':
        {
            s_wsabuf.buf = reinterpret_cast<char*>(&input_key.key);
            s_wsabuf.len = sizeof(input_key);
            int retval = WSASend(s_socket, &s_wsabuf, 1, &send_bytes, 0, 0, 0);
            if (SOCKET_ERROR == retval)
            {
                error_display("send from client", WSAGetLastError());
                exit(-1);
            }
            WSABUF r_wsabuf;
            r_wsabuf.buf = reinterpret_cast<char*>(&players);
            r_wsabuf.len = sizeof(players);
            DWORD recv_bytes;
            DWORD r_flag = 0;
            retval = WSARecv(s_socket, &r_wsabuf, 1, &recv_bytes, &r_flag, 0, 0);
            if (SOCKET_ERROR == retval)
            {
                error_display("recv from client", WSAGetLastError());
                exit(-1);
            }
            input_key.key = 'g';
        }
        break;
        case 'a':
        {
            s_wsabuf.buf = reinterpret_cast<char*>(&input_key.key);
            s_wsabuf.len = sizeof(input_key);
            int retval = WSASend(s_socket, &s_wsabuf, 1, &send_bytes, 0, 0, 0);
            if (SOCKET_ERROR == retval)
            {
                error_display("send from client", WSAGetLastError());
                exit(-1);
            }


            WSABUF r_wsabuf;
            r_wsabuf.buf = reinterpret_cast<char*>(&players);
            r_wsabuf.len = sizeof(players);
            DWORD recv_bytes;
            DWORD r_flag = 0;
            retval = WSARecv(s_socket, &r_wsabuf, 1, &recv_bytes, &r_flag, 0, 0);
            if (SOCKET_ERROR == retval)
            {
                error_display("recv from client", WSAGetLastError());
                exit(-1);
            }
            input_key.key = 'g';
        }
        break;
        case 'd':
        {
            s_wsabuf.buf = reinterpret_cast<char*>(&input_key.key);
            s_wsabuf.len = sizeof(input_key);
            int retval = WSASend(s_socket, &s_wsabuf, 1, &send_bytes, 0, 0, 0);
            if (SOCKET_ERROR == retval)
            {
                error_display("send from client", WSAGetLastError());
                exit(-1);
            }


            WSABUF r_wsabuf;
            r_wsabuf.buf = reinterpret_cast<char*>(&players);
            r_wsabuf.len = sizeof(players);
            DWORD recv_bytes;
            DWORD r_flag = 0;
            retval = WSARecv(s_socket, &r_wsabuf, 1, &recv_bytes, &r_flag, 0, 0);
            if (SOCKET_ERROR == retval)
            {
                error_display("recv from client", WSAGetLastError());
                exit(-1);
            }
            input_key.key = 'g';
        }
        break;
        default:
        {
            if (true == isLogin)
            {
                input_key.key = 'g';
                s_wsabuf.buf = reinterpret_cast<char*>(&input_key.key);
                s_wsabuf.len = sizeof(input_key);
                int retval = WSASend(s_socket, &s_wsabuf, 1, &send_bytes, 0, 0, 0);
                if (SOCKET_ERROR == retval)
                {
                    error_display("send from client", WSAGetLastError());
                    exit(-1);
                }

                WSABUF r_wsabuf;
                r_wsabuf.buf = reinterpret_cast<char*>(&players);
                r_wsabuf.len = sizeof(players);
                DWORD recv_bytes;
                DWORD r_flag = 0;
                retval = WSARecv(s_socket, &r_wsabuf, 1, &recv_bytes, &r_flag, 0, 0);
                if (SOCKET_ERROR == retval)
                {
                    error_display("recv from client", WSAGetLastError());
                    exit(-1);
                }
                break;
            }
        }
        }
    }
    InvalidateRect(hWnd, NULL, FALSE);
    break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...

            wstring input_ip = L"Server IP : ";
            wstring inpuy_ip2{ client_ip.begin(),client_ip.end() };
            wstring textBox = input_ip + inpuy_ip2;

            Graphics graphics(hdc);
            Image image(L"chessboard.bmp");

            graphics.DrawImage(&image, 0, 0);

            for (int i = 0; i < 10; i++)
            {
                Ellipse(hdc, 0 + players[i].pos.x, 0 + players[i].pos.y, 60 + players[i].pos.x, 60 + players[i].pos.y);
            }

            RECT rt = { 100,100,400,300 };
            DrawText(hdc, textBox.c_str(), -1, &rt, DT_CENTER | DT_WORDBREAK);
            //InvalidateRect(hWnd, NULL, FALSE);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_UP:
        {
            input_key.key = 'w';
        }
        break;
        case VK_DOWN:
        {
            input_key.key = 's';
        }
        break;
        case VK_LEFT:
        {
            input_key.key = 'a';
        }
        break;
        case VK_RIGHT:
        {
            input_key.key = 'd';
        }
        break;
        }
        break;
    case WM_DESTROY:
        closesocket(s_socket);
        WSACleanup();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
