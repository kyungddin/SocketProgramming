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
    SOCKET ServerSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (ServerSocket == INVALID_SOCKET)
    {
        cerr << "Can't Create Socket" << endl;
        WSACleanup();
        return -1;
    }

    // ── 3. bind() ────────────────────────────────────────────────────
    // 서버는 클라이언트와 달리 내 IP + Port를 OS에 등록해야 함
    sockaddr_in hint {};
    hint.sin_family      = AF_INET;
    hint.sin_port        = htons(PORT);
    hint.sin_addr.s_addr = INADDR_ANY; // 모든 NIC에서 수신 허용

    if (bind(ServerSocket, reinterpret_cast<sockaddr*>(&hint), sizeof(hint)) == SOCKET_ERROR)
    {
        cerr << "Can't Bind Socket" << endl;
        closesocket(ServerSocket);
        WSACleanup();
        return -1;
    }

    cout << "Server listening on port " << PORT << "..." << endl;

    // ── 4. recvfrom / sendto 루프 ────────────────────────────────────
    // 클라이언트 주소를 recvfrom()에서 받아와 sendto()에서 그대로 사용
    sockaddr_in clientAddr {};
    int         clientAddrSize = sizeof(clientAddr);

    do
    {
        // 4-1. recvfrom()
        ZeroMemory(buf, BUF_SIZE);
        int bytesReceived = recvfrom(
            ServerSocket,
            buf,
            BUF_SIZE,
            0,
            reinterpret_cast<sockaddr*>(&clientAddr),  // 송신측(클라이언트) 주소 저장
            &clientAddrSize
        );

        if (bytesReceived == SOCKET_ERROR)
            continue;

        if (bytesReceived == 0)
            break;

        cout << "CLIENT> " << buf << endl;

        // 4-2. sendto()
        // recvfrom()에서 받은 clientAddr로 그대로 응답
        string response = string(buf); // Echo
        int sendResult = sendto(
            ServerSocket,
            response.c_str(),
            response.size() + 1,
            0,
            reinterpret_cast<sockaddr*>(&clientAddr),  // 목적지 = 클라이언트
            clientAddrSize
        );

        if (sendResult == SOCKET_ERROR)
            cerr << "Send Failed" << endl;

    } while (true);

    // ── 5. closesocket / WSACleanup ──────────────────────────────────
    closesocket(ServerSocket);
    WSACleanup();
    return 0;
}