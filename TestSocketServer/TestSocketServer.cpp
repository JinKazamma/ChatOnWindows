#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mutex>
#include <thread>
#include <string>
using namespace std;
mutex mtx;
int SendMsg(SOCKET ClientSocket, PADDRINFOA addrResult)
{
	string massage;
	int result;
	cout << "ВВедите сообщение: "; getline(cin, massage);
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
	SOCKET ClientSocket = INVALID_SOCKET;
	SOCKET ListenSocked = INVALID_SOCKET;
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
	hints.ai_flags = AI_PASSIVE;
	const char* sendbuffer = "rteteert";
	char recvbuffer[512];
	result = getaddrinfo(NULL, "666", &hints, &addrResult);
	if (result != 0)
	{
		cout << "getadrinfo failed with error:" << result << endl;
		WSACleanup();
		return 1;
	}
	ListenSocked = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
	if (ListenSocked == INVALID_SOCKET)
	{
		cout << "socket creation failed with error:" << endl;
		freeaddrinfo(addrResult);
		WSACleanup();
		return 1;
	}

	result = bind(ListenSocked, addrResult->ai_addr, (int)addrResult->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		cout << "Binding socked server" << endl;
		closesocket(ListenSocked);
		ListenSocked = INVALID_SOCKET;
		freeaddrinfo(addrResult);
		WSACleanup();
		return 1;
	}
	result = listen(ListenSocked, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		cout << "listing socked fail" << endl;
		closesocket(ListenSocked);
		freeaddrinfo(addrResult);
		WSACleanup();
		return 1;
	}

	ClientSocket = accept(ListenSocked, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET)
	{
		cout << "Accepting socked fail" << endl;
		closesocket(ListenSocked);
		freeaddrinfo(addrResult);
		WSACleanup();
		return 1;
	}
	closesocket(ListenSocked);
	
	thread t;
	
	do
	{
		mtx.lock();
		ZeroMemory(recvbuffer, 512);
		result = recv(ClientSocket, recvbuffer, 512, 0);
		if (result > 0)
		{
			//cout << "Received " << result << "Bytes" << endl;
			cout << "Полученное сообщение: " << recvbuffer << endl;
			
		}
		mtx.unlock();
		
		t = thread(SendMsg, ClientSocket, addrResult);
		t.join();
	} while (true);

	

	result = shutdown(ClientSocket, SD_SEND);
	if (result == SOCKET_ERROR)
	{
		cout << "shutdown client socket failed" << endl;
		closesocket(ClientSocket);
		freeaddrinfo(addrResult);
		WSACleanup();
		return 1;
	}
	closesocket(ClientSocket);
	freeaddrinfo(addrResult);
	WSACleanup();
	return 0;

}
