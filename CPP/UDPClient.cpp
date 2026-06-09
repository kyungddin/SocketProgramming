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

    // ── 2. **socket()** ──────────────────────────────────────────────
    SOCKET ClientSocket = socket(AF_INET, SOCK_DGRAM, 0);
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

    // ── 4. **sendto / recvfrom 루프** ────────────────────────────────
    // recvfrom()에서 서버 주소를 받아올 변수
	// 원래는 필요 없지만 recvfrom()의 매개변수 채우기 위해 사용
	sockaddr_in serverAddr {};
    int         serverAddrSize = sizeof(serverAddr);	

    do
    {
        cout << "> ";
        getline(cin, userMsg);

        if (userMsg.size() == 0)
            break;

        // 4-1. sendto()
        int sendResult = sendto(
            ClientSocket,
            userMsg.c_str(),
            userMsg.size() + 1,
            0,
            reinterpret_cast<sockaddr*>(&hint),
            sizeof(hint)
        );

        if (sendResult == SOCKET_ERROR)
            continue;

        // 4-2. recvfrom()
        ZeroMemory(buf, BUF_SIZE);
        int bytesReceived = recvfrom(
            ClientSocket,
            buf,
            BUF_SIZE,
            0,
            reinterpret_cast<sockaddr*>(&serverAddr),
            &serverAddrSize
        );

        if (bytesReceived > 0)
            cout << "SERVER> " << buf << endl;

    } while (true);

    // ── 5. **closesocket / WSACleanup** ──────────────────────────────────
    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}
