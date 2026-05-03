#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>


#pragma comment(lib, "Ws2_32.lib")

using namespace std;

const char* getErrorName(int errorCode) {
    switch (errorCode) {
    case WSANOTINITIALISED: return "WSANOTINITIALISED";
    case WSAENETDOWN:       return "WSAENETDOWN";
    case WSAEADDRINUSE:     return "WSAEADDRINUSE";
    case WSAEFAULT:         return "WSAEFAULT";
    case WSAEINPROGRESS:    return "WSAEINPROGRESS";
    case WSAEAFNOSUPPORT:   return "WSAEAFNOSUPPORT";
    case WSAEINVAL:         return "WSAEINVAL";
    case WSAENOBUFS:        return "WSAENOBUFS";
    case WSAENOTSOCK:       return "WSAENOTSOCK";
    case WSAEADDRNOTAVAIL:  return "WSAEADDRNOTAVAIL";
    default:                return "UNKNOWN_ERROR";
    }
}

void printSocketStatus(const char* socketName, SOCKET s) {
    if (s == INVALID_SOCKET) {
        int err = WSAGetLastError();
        cout << socketName << " creation failed. Error: "
            << err << " (" << getErrorName(err) << ")" << endl;
    }
    else {
        cout << socketName << " created successfully." << endl;
    }
}

void bindAndPrint(const char* socketName, SOCKET s, sockaddr_in& addr) {
    int result = bind(s, (sockaddr*)&addr, sizeof(addr));
    if (result == SOCKET_ERROR) {
        int err = WSAGetLastError();
        cout << socketName << " bind failed. Error: "
            << err << " (" << getErrorName(err) << ")" << endl;
    }
    else {
        cout << socketName << " bind successful." << endl;
    }
}

void printBindError(const char* testName, int result) {
    if (result == SOCKET_ERROR) {
        int err = WSAGetLastError();
        cout << testName << " -> Error: "
            << err << " (" << getErrorName(err) << ")" << endl;
    }
    else {
        cout << testName << " -> No error" << endl;
    }
}

int main() {
    WSADATA wsaData;

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed. Error: " << result << endl;
        return 1;
    }

    cout << "=== WinSock initialization ===" << endl;
    cout << "WinSock initialized successfully!" << endl;
    cout << "Description: " << wsaData.szDescription << endl;
    cout << "System status: " << wsaData.szSystemStatus << endl;
    cout << "Version: "
        << (int)LOBYTE(wsaData.wVersion) << "."
        << (int)HIBYTE(wsaData.wVersion) << endl;
    cout << "High version: "
        << (int)LOBYTE(wsaData.wHighVersion) << "."
        << (int)HIBYTE(wsaData.wHighVersion) << endl;
    cout << "Max UDP Datagram size reported by WinSock: " << wsaData.iMaxUdpDg << endl;

    // Умовні останні 3 цифри залікової книжки
    int maxDatagramSize = 247;
    char datagramBuffer[247];
    cout << "Custom max datagram size (from student ID): " << maxDatagramSize << endl;
    cout << "Allocated buffer size: " << sizeof(datagramBuffer) << endl;

    cout << "\n=== Creating 4 sockets ===" << endl;

    SOCKET sock1 = socket(AF_INET, SOCK_DGRAM, 0);
    SOCKET sock2 = socket(AF_INET, SOCK_DGRAM, 0);
    SOCKET sock3 = socket(AF_INET, SOCK_STREAM, 0);
    SOCKET sock4 = socket(AF_INET, SOCK_STREAM, 0);

    printSocketStatus("Socket 1 (UDP, broadcast)", sock1);
    printSocketStatus("Socket 2 (UDP, fixed IP)", sock2);
    printSocketStatus("Socket 3 (TCP, fixed IP)", sock3);
    printSocketStatus("Socket 4 (TCP, any local)", sock4);

    BOOL broadcastEnable = TRUE;
    if (setsockopt(sock1, SOL_SOCKET, SO_BROADCAST, (char*)&broadcastEnable, sizeof(broadcastEnable)) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        cout << "Socket 1 broadcast setup failed. Error: "
            << err << " (" << getErrorName(err) << ")" << endl;
    }
    else {
        cout << "Socket 1 broadcast mode enabled." << endl;
    }

    sockaddr_in addr1{}, addr2{}, addr3{}, addr4{};

    // 1) Broadcast
    addr1.sin_family = AF_INET;
    addr1.sin_port = htons(5001);
    addr1.sin_addr.s_addr = INADDR_BROADCAST;

    // 2) 10.1.2.1XX -> для XX=25 => 10.1.2.125
    addr2.sin_family = AF_INET;
    addr2.sin_port = htons(5002);
    addr2.sin_addr.s_addr = inet_addr("10.1.2.125");

    // 3) 10.1.2.1(XX+1) -> для XX=25 => 10.1.2.126
    addr3.sin_family = AF_INET;
    addr3.sin_port = htons(5003);
    addr3.sin_addr.s_addr = inet_addr("10.1.2.126");

    // 4) Будь-яка локальна адреса
    addr4.sin_family = AF_INET;
    addr4.sin_port = htons(5004);
    addr4.sin_addr.s_addr = INADDR_ANY;

    cout << "\n=== Binding sockets ===" << endl;
    bindAndPrint("Socket 1 (UDP, broadcast)", sock1, addr1);
    bindAndPrint("Socket 2 (UDP, fixed IP)", sock2, addr2);
    bindAndPrint("Socket 3 (TCP, fixed IP)", sock3, addr3);
    bindAndPrint("Socket 4 (TCP, any local)", sock4, addr4);

    cout << "\n=== Artificial bind() error simulation ===" << endl;

    // 1. WSAEADDRINUSE - адреса вже використовується
    SOCKET sAddrUse1 = socket(AF_INET, SOCK_STREAM, 0);
    SOCKET sAddrUse2 = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in sameAddr{};
    sameAddr.sin_family = AF_INET;
    sameAddr.sin_port = htons(5050);
    sameAddr.sin_addr.s_addr = INADDR_ANY;

    bind(sAddrUse1, (sockaddr*)&sameAddr, sizeof(sameAddr));
    int rAddrInUse = bind(sAddrUse2, (sockaddr*)&sameAddr, sizeof(sameAddr));
    printBindError("WSAEADDRINUSE test", rAddrInUse);

    // 2. WSAEINVAL - сокет уже прив'язаний
    SOCKET sInvalidBind = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in invalAddr{};
    invalAddr.sin_family = AF_INET;
    invalAddr.sin_port = htons(5051);
    invalAddr.sin_addr.s_addr = INADDR_ANY;

    bind(sInvalidBind, (sockaddr*)&invalAddr, sizeof(invalAddr));
    int rInvalid = bind(sInvalidBind, (sockaddr*)&invalAddr, sizeof(invalAddr));
    printBindError("WSAEINVAL test", rInvalid);

    // 3. WSAEFAULT - некоректний namelen
    SOCKET sFault = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in faultAddr{};
    faultAddr.sin_family = AF_INET;
    faultAddr.sin_port = htons(5052);
    faultAddr.sin_addr.s_addr = INADDR_ANY;

    int rFault = bind(sFault, (sockaddr*)&faultAddr, 1);
    printBindError("WSAEFAULT test", rFault);

    // 4. WSAENOTSOCK - дескриптор не є сокетом
    sockaddr_in notSockAddr{};
    notSockAddr.sin_family = AF_INET;
    notSockAddr.sin_port = htons(5053);
    notSockAddr.sin_addr.s_addr = INADDR_ANY;

    int rNotSock = bind((SOCKET)12345, (sockaddr*)&notSockAddr, sizeof(notSockAddr));
    printBindError("WSAENOTSOCK test", rNotSock);

    // 5. WSANOTINITIALISED - WinSock не ініціалізований
    SOCKET sNotInit = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in notInitAddr{};
    notInitAddr.sin_family = AF_INET;
    notInitAddr.sin_port = htons(5054);
    notInitAddr.sin_addr.s_addr = INADDR_ANY;

    WSACleanup(); // спеціально вимикаємо WinSock
    int rNotInit = bind(sNotInit, (sockaddr*)&notInitAddr, sizeof(notInitAddr));
    printBindError("WSANOTINITIALISED test", rNotInit);

    // Знову ініціалізуємо, щоб коректно закрити сокети
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "Second WSAStartup failed. Error: " << result << endl;
        return 1;
    }

    // Закриття всіх сокетів
    if (sock1 != INVALID_SOCKET) closesocket(sock1);
    if (sock2 != INVALID_SOCKET) closesocket(sock2);
    if (sock3 != INVALID_SOCKET) closesocket(sock3);
    if (sock4 != INVALID_SOCKET) closesocket(sock4);

    if (sAddrUse1 != INVALID_SOCKET) closesocket(sAddrUse1);
    if (sAddrUse2 != INVALID_SOCKET) closesocket(sAddrUse2);
    if (sInvalidBind != INVALID_SOCKET) closesocket(sInvalidBind);
    if (sFault != INVALID_SOCKET) closesocket(sFault);
    if (sNotInit != INVALID_SOCKET) closesocket(sNotInit);

    WSACleanup();

    cout << "\n=== Program finished successfully ===" << endl;
    return 0;
}