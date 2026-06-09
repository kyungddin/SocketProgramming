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
    // ── 0. 주소 / 버퍼 준비 ──────────────────────────────────────────
    const char* serverIpAddr = "127.0.0.1";
    char        buf[BUF_SIZE];
    std::string userMsg;

    // ── 1. WSAStartup ────────────────────────────────────────────────
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "Can't Init WinSock!" << endl;
        return -1;
    }

    // ── 2. socket() ──────────────────────────────────────────────────
    SOCKET ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ClientSocket == INVALID_SOCKET)
    {
        cerr << "Can't Create Socket" << endl;
        WSACleanup();
        return -1;
    }

    // ── 3. 서버 주소 설정 (hint) ─────────────────────────────────────
    sockaddr_in hint {};
    hint.sin_family = AF_INET;
    hint.sin_port   = htons(PORT);
    if (inet_pton(AF_INET, serverIpAddr, &hint.sin_addr) != 1)
    {
        cerr << "Can't Convert IP address" << endl;
        closesocket(ClientSocket);
        WSACleanup();
        return -1;
    }

    // ── 4. connect() ─────────────────────────────────────────────────
    // 3-way handshake 자동 수행 (UDP에는 없는 단계)
    if (connect(ClientSocket, reinterpret_cast<sockaddr*>(&hint), sizeof(hint)) == SOCKET_ERROR)
    {
        cerr << "Can't Connect To Server!" << endl;
        closesocket(ClientSocket);
        WSACleanup();
        return -1;
    }

    // ── 5. send / recv 루프 ──────────────────────────────────────────
    do
    {
        cout << "> ";
        getline(cin, userMsg);

        if (userMsg.size() == 0)
            break;

        // 5-1. send()
        // connect()로 이미 연결됐으므로 목적지 주소 지정 불필요
        int sendResult = send(ClientSocket, userMsg.c_str(), userMsg.size() + 1, 0);

        if (sendResult == SOCKET_ERROR)
            continue;

        // 5-2. recv()
        ZeroMemory(buf, BUF_SIZE);
        int bytesReceived = recv(ClientSocket, buf, BUF_SIZE, 0);

        if (bytesReceived > 0)
            cout << "SERVER> " << buf << endl;

    } while (true);

    // ── 6. closesocket / WSACleanup ──────────────────────────────────
    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}