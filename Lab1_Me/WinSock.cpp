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
        cout << "[ERROR] " << socketName << " creation failed! Code: "
            << err << " [" << getErrorName(err) << "]" << endl;
    }
    else {
        cout << "[OK] " << socketName << " initialized successfully." << endl;
    }
}

void bindAndPrint(const char* socketName, SOCKET s, sockaddr_in& addr) {
    int result = bind(s, (sockaddr*)&addr, sizeof(addr));
    if (result == SOCKET_ERROR) {
        int err = WSAGetLastError();
        cout << "[BIND FAILED] " << socketName << " -> Code: "
            << err << " [" << getErrorName(err) << "]" << endl;
    }
    else {
        cout << "[BIND OK] " << socketName << " bound successfully." << endl;
    }
}

void printBindError(const char* testName, int result) {
    if (result == SOCKET_ERROR) {
        int err = WSAGetLastError();
        cout << " -> Error caught: " << err << " [" << getErrorName(err) << "]" << endl;
    }
    else {
        cout << " -> No errors detected (unexpected)" << endl;
    }
}

int main() {
    WSADATA wsaData;

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "Critical error: WSAStartup failed. Code: " << result << endl;
        return 1;
    }

    cout << ">>> STAGE 1: WinSock Setup <<<" << endl;
    cout << "Library loaded successfully!" << endl;
    cout << "Description: " << wsaData.szDescription << endl;
    cout << "System status: " << wsaData.szSystemStatus << endl;
    cout << "Current version: "
        << (int)LOBYTE(wsaData.wVersion) << "."
        << (int)HIBYTE(wsaData.wVersion) << endl;
    cout << "Max supported version: "
        << (int)LOBYTE(wsaData.wHighVersion) << "."
        << (int)HIBYTE(wsaData.wHighVersion) << endl;
    cout << "UDP datagram size limit (from OS): " << wsaData.iMaxUdpDg << " bytes" << endl;

    // Умова 
    int maxDatagramSize = 247;
    char datagramBuffer[247];
    cout << "\nCustom buffer size (by variant): " << maxDatagramSize << " bytes" << endl;
    cout << "Memory allocated for buffer: " << sizeof(datagramBuffer) << " bytes" << endl;

    cout << "\n>>> STAGE 2: Generating Sockets <<<" << endl;

    SOCKET sock1 = socket(AF_INET, SOCK_DGRAM, 0);
    SOCKET sock2 = socket(AF_INET, SOCK_DGRAM, 0);
    SOCKET sock3 = socket(AF_INET, SOCK_STREAM, 0);
    SOCKET sock4 = socket(AF_INET, SOCK_STREAM, 0);

    printSocketStatus("Socket #1 (UDP, broadcast)", sock1);
    printSocketStatus("Socket #2 (UDP, static IP)", sock2);
    printSocketStatus("Socket #3 (TCP, static IP)", sock3);
    printSocketStatus("Socket #4 (TCP, any local)", sock4);

    BOOL broadcastEnable = TRUE;
    if (setsockopt(sock1, SOL_SOCKET, SO_BROADCAST, (char*)&broadcastEnable, sizeof(broadcastEnable)) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        cout << "Failed to enable Broadcast for Socket #1. Code: "
            << err << " [" << getErrorName(err) << "]" << endl;
    }
    else {
        cout << "Broadcast mode for Socket #1 activated." << endl;
    }

    sockaddr_in addr1{}, addr2{}, addr3{}, addr4{};

    // 1) Broadcast
    addr1.sin_family = AF_INET;
    addr1.sin_port = htons(5001);
    addr1.sin_addr.s_addr = INADDR_BROADCAST;

    // 2) 10.1.2.1XX -> для XX=47 => 10.1.2.147 
    addr2.sin_family = AF_INET;
    addr2.sin_port = htons(5002);
    addr2.sin_addr.s_addr = inet_addr("10.1.2.147");

    // 3) 10.1.2.1(XX+1) -> для XX=47 => 10.1.2.148
    addr3.sin_family = AF_INET;
    addr3.sin_port = htons(5003);
    addr3.sin_addr.s_addr = inet_addr("10.1.2.148");

    // 4) Будь-яка локальна адреса
    addr4.sin_family = AF_INET;
    addr4.sin_port = htons(5004);
    addr4.sin_addr.s_addr = INADDR_ANY;

    cout << "\n>>> STAGE 3: Binding <<<" << endl;
    bindAndPrint("Socket #1", sock1, addr1);
    bindAndPrint("Socket #2", sock2, addr2);
    bindAndPrint("Socket #3", sock3, addr3);
    bindAndPrint("Socket #4", sock4, addr4);

    cout << "\n>>> STAGE 4: bind() Error Handling Tests <<<" << endl;

    // 1. WSAEADDRINUSE
    cout << "[Test 1] Address in use (WSAEADDRINUSE)";
    SOCKET sAddrUse1 = socket(AF_INET, SOCK_STREAM, 0);
    SOCKET sAddrUse2 = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in sameAddr{};
    sameAddr.sin_family = AF_INET;
    sameAddr.sin_port = htons(5050);
    sameAddr.sin_addr.s_addr = INADDR_ANY;
    bind(sAddrUse1, (sockaddr*)&sameAddr, sizeof(sameAddr));
    int rAddrInUse = bind(sAddrUse2, (sockaddr*)&sameAddr, sizeof(sameAddr));
    printBindError("", rAddrInUse);

    // 2. WSAEINVAL
    cout << "[Test 2] Invalid argument / Already bound (WSAEINVAL)";
    SOCKET sInvalidBind = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in invalAddr{};
    invalAddr.sin_family = AF_INET;
    invalAddr.sin_port = htons(5051);
    invalAddr.sin_addr.s_addr = INADDR_ANY;
    bind(sInvalidBind, (sockaddr*)&invalAddr, sizeof(invalAddr));
    int rInvalid = bind(sInvalidBind, (sockaddr*)&invalAddr, sizeof(invalAddr));
    printBindError("", rInvalid);

    // 3. WSAEFAULT
    cout << "[Test 3] Bad address / structure size (WSAEFAULT)";
    SOCKET sFault = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in faultAddr{};
    faultAddr.sin_family = AF_INET;
    faultAddr.sin_port = htons(5052);
    faultAddr.sin_addr.s_addr = INADDR_ANY;
    int rFault = bind(sFault, (sockaddr*)&faultAddr, 1); // Навмисно передаємо 1
    printBindError("", rFault);

    // 4. WSAENOTSOCK
    cout << "[Test 4] Invalid socket descriptor (WSAENOTSOCK)";
    sockaddr_in notSockAddr{};
    notSockAddr.sin_family = AF_INET;
    notSockAddr.sin_port = htons(5053);
    notSockAddr.sin_addr.s_addr = INADDR_ANY;
    int rNotSock = bind((SOCKET)99999, (sockaddr*)&notSockAddr, sizeof(notSockAddr));
    printBindError("", rNotSock);

    // 5. WSANOTINITIALISED
    cout << "[Test 5] WinSock not initialized (WSANOTINITIALISED)";
    SOCKET sNotInit = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in notInitAddr{};
    notInitAddr.sin_family = AF_INET;
    notInitAddr.sin_port = htons(5054);
    notInitAddr.sin_addr.s_addr = INADDR_ANY;

    WSACleanup(); // Вимикаємо підсистему
    int rNotInit = bind(sNotInit, (sockaddr*)&notInitAddr, sizeof(notInitAddr));
    printBindError("", rNotInit);

    // Повторна ініціалізація для безпечного очищення пам'яті
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "Error restarting WSAStartup." << endl;
        return 1;
    }

    // Закриваємо всі створені сокети
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

    cout << "\n>>> PROGRAM FINISHED <<<" << endl;
    return 0;
}