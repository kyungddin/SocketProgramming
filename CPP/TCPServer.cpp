#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#pragma comment(lib, "ws2_32")
using namespace std;

enum ePort    { PORT     = 54000 };
enum eBufSize { BUF_SIZE = 4096  };

int main()
{
    // ── 0. 버퍼 준비 ─────────────────────────────────────────────────
    char buf[BUF_SIZE];

    // ── 1. WSAStartup ────────────────────────────────────────────────
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "Can't Init WinSock!" << endl;
        return -1;
    }

    // ── 2. socket() ──────────────────────────────────────────────────
    SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ServerSocket == INVALID_SOCKET)
    {
        cerr << "Can't Create Socket!" << endl;
        WSACleanup();
        return -1;
    }

    // ── 3. bind() ────────────────────────────────────────────────────
    sockaddr_in hint {};
    hint.sin_family           = AF_INET;
    hint.sin_port             = htons(PORT);
    hint.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // 모든 NIC에서 수신 허용

    if (bind(ServerSocket, reinterpret_cast<sockaddr*>(&hint), sizeof(hint)) == SOCKET_ERROR)
    {
        cerr << "Can't Bind Socket!" << endl;
        closesocket(ServerSocket);
        WSACleanup();
        return -1;
    }

    // ── 4. listen() ──────────────────────────────────────────────────
    // SOMAXCONN: accept() 전에 쌓아둘 연결 요청을 OS 최대치로 허용
    if (listen(ServerSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        cerr << "Can't Listen Socket!" << endl;
        closesocket(ServerSocket);
        WSACleanup();
        return -1;
    }

    cout << "Server listening on port " << PORT << "..." << endl;

    // ── 5. accept() ──────────────────────────────────────────────────
    // 클라이언트 연결 수락 → 통신 전용 새 소켓(ClientSocket) 반환
    sockaddr_in clientAddr {};
    int         clientAddrSize = sizeof(clientAddr);

    SOCKET ClientSocket = accept(ServerSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
    if (ClientSocket == INVALID_SOCKET)
    {
        cerr << "Can't Accept Socket!" << endl;
        closesocket(ServerSocket);
        WSACleanup();
        return -1;
    }

    // ── 6. 리스닝 소켓 닫기 ──────────────────────────────────────────
    // accept() 이후 ServerSocket은 역할 끝 → 닫아도 무방
    closesocket(ServerSocket);

    // ── 7. recv / send 루프 ──────────────────────────────────────────
    // 클라이언트 접속 정보 출력
    char host[NI_MAXHOST] {};
    char service[NI_MAXSERV] {};
    if (getnameinfo((sockaddr*)&clientAddr, clientAddrSize, host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
    {
        cout << host << " connected on port " << service << endl;
    }
    else
    {
        inet_ntop(AF_INET, &clientAddr.sin_addr, host, NI_MAXHOST);
        cout << host << " connected on port " << ntohs(clientAddr.sin_port) << endl;
    }

    while (true)
    {
        // 7-1. recv()
        ZeroMemory(buf, BUF_SIZE);
        int bytesReceived = recv(ClientSocket, buf, BUF_SIZE, 0);

        if (bytesReceived == SOCKET_ERROR)
        {
            cerr << "Error in recv(). Quitting" << endl;
            break;
        }

        if (bytesReceived == 0)
        {
            cout << "Client disconnected" << endl;
            break;
        }

        // 7-2. send()
        cout << "CLIENT> " << buf << endl;
        send(ClientSocket, buf, bytesReceived + 1, 0);
    }

    // ── 8. closesocket / WSACleanup ──────────────────────────────────
    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}