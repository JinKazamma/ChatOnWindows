#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <mutex>
#include <string>
using namespace std;
mutex mtx;
int SendMsg(SOCKET ClientSocket, PADDRINFOA addrResult)
{ 
	string massage;
	cout << "ВВедите сообщение: "; getline(cin, massage);
	int result;
	mtx.lock(); //блокировка мьютекса
	result = send(ClientSocket, massage.c_str(), (int)strlen(massage.c_str()), 0);
	if (result == SOCKET_ERROR)
	{
		cout << "failed to send data back " << endl;
		closesocket(ClientSocket);
		freeaddrinfo(addrResult);
		WSACleanup();
		return 1;
	}
	mtx.unlock(); //разблокировка мьютекса
}
int main()
{
	setlocale(LC_ALL, "RU");
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	int result;
	ADDRINFOA hints;
	PADDRINFOA addrResult = NULL;
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		cout << "wsastartup Faild, result =" << result << endl;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	const char* sendbuffer = "vistrel!";
	char recvbuffer[512];
	result = getaddrinfo("localhost", "666", &hints, &addrResult);
	if (result != 0)
	{
		cout << "getadrinfo failed with error:" << result << endl;
		WSACleanup();
		return 1;
	}
	ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET)
	{
		cout << "socket creation failed with error:"<< endl;
		freeaddrinfo(addrResult);
		WSACleanup();
		return 1;
	}
	result = connect(ConnectSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		cout << "Unable connect server" << endl;
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		freeaddrinfo(addrResult);
		WSACleanup();
		return 1;
	}
	do
	{
		thread t(SendMsg,ConnectSocket, addrResult);
		t.join();
		ZeroMemory(recvbuffer, 512);
		mtx.lock();
		result = recv(ConnectSocket, recvbuffer, 512, 0);
		if (result > 0)
		{
			//cout << "Received " << result << "Bytes" << endl;
			cout << "Полеученное сообщение: " << recvbuffer << endl;
		}
		mtx.unlock();
		
	} while (true);
	result = shutdown(ConnectSocket, SD_SEND);
	if (result == SOCKET_ERROR)
	{
		cout << " shutdown error" << result << endl;
		closesocket(ConnectSocket);
		freeaddrinfo(addrResult);
		WSACleanup();
		return 1;
	}
	closesocket(ConnectSocket);
	freeaddrinfo(addrResult);
	WSACleanup();
	return 0;

}