#pragma once
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <string>
#include "type.h"
#include "define.h"
#include "socket.h"

#ifdef _DEBUG
	#include <iostream>
	using namespace std;
#endif

#ifndef RECV_MAX_SIZE
	#define RECV_MAX_SIZE 65000
#endif

using std::string;

namespace Tribal {
	static uint8 localhost[] = LOCALHOST;
	class TCPSocket :public Socket<TCPSocket> {
	private:
		TCPSocket* m_ptr;	// Current TCPSocket's object pointer

	protected:
		sockaddr_in m_sa;	// Server have Protocol & IP & Port itself, but Client have target's Protocol & IP & Port

	protected:
		/*
		Create a socket.
		If socket() return error, this function will return -1.
		*/
		SOCKET CreateSocket() override {
#ifndef _WIN32
			m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
			WSADATA wsaData;
			int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (ret != 0) {
				// need to add error Handler
				return null;
			}

			m_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, null, 0, WSA_FLAG_OVERLAPPED);
#endif
			if (m_sock == INVALID_SOCKET) {
				DWORD error = GetLastError();
				printf("%u\n", error); // WASNOTINITIAILIZED (WSAStartup)
			}

			return m_sock;
		}
		
		/* Cleanup current object's information(socket, structure sockaddr_in) */
		void CloseSocket() override {
			int ret = 0;
			ret = closesocket(m_sock);
			if (ret != -1) {
				WSACleanup();
				ZeroMemory(&m_sa, sizeof(sockaddr_in));
				m_ptr = NULL;
			}
			else {
				fprintf(stderr, "[-] error closesocket\n");
			}
		}

		/* It is function for Send, It is used as '<<' */
		int32 SendData(ccData data, const size_t len) override {
			size_t send_size = 0;
			DWORD dwFlags = 0;
#ifdef _DEBUG
			std::cout << "[+] TCPData Send... - " << len << std::endl;
			std::cout << data << std::endl;
#endif
#ifndef _WIN32
			return send(m_sock, data, len, dwFlags);
#else
			WSABUF wsaBuf;
			OVERLAPPED overlapped;

			wsaBuf.buf = (CHAR*)data;
			wsaBuf.len = len;
			
			ZeroMemory(&overlapped, sizeof(OVERLAPPED));
			// for overlapped
			return WSASend(m_sock, &wsaBuf, 1, (LPDWORD)&send_size, dwFlags, (LPWSAOVERLAPPED)&overlapped, NULL);
#endif
		}

		/* It is function for Receive, It is used as '>>' */
		int32 RecvData(cData data, const size_t max) override {
			size_t recv_size = 0;
			DWORD dwFlags = 0;
#ifdef _DEBUG
			std::cout << "[+] TCPData Recv... - " << max << std::endl;
			std::cout << data << std::endl;
#endif
#ifndef _WIN32
			return recv(m_sock, data, max, 0);
#else
			WSABUF wsaBuf;
			OVERLAPPED overlapped;

			wsaBuf.buf = (CHAR*)data;
			wsaBuf.len = max;

			ZeroMemory(&overlapped, sizeof(OVERLAPPED));
			// for overlapped
			return WSARecv(m_sock, &wsaBuf, 1, (LPDWORD)&recv_size, &dwFlags, (LPWSAOVERLAPPED)&overlapped, NULL);
#endif
		}

		TCPSocket() :Socket() {
			ZeroMemory(&m_sa, sizeof(sockaddr_in));
			m_ptr = null;

			CreateSocket();
		}

	public:
		/* Hmm... */
		TCPSocket(TCPSocket& t_socket)/* :Socket() */{
			m_ptr = &t_socket;
			memcpy(&m_sa, &t_socket.m_sa, sizeof(sockaddr_in));
			m_sock = t_socket.m_sock;
		}

		// send override
		friend TCPSocket& operator <<(TCPSocket& t_socket, string data) {
			t_socket.SendData(data.c_str(), data.length());

			return t_socket;
		}

		// recv override
		friend TCPSocket& operator >>(TCPSocket& t_socket, string& data) {
			int32 recved_size = 0;	// for -1 check

			data.resize(RECV_MAX_SIZE);
			recved_size = t_socket.RecvData((cData)data.data(), RECV_MAX_SIZE);
			data.resize(recved_size > 0 ? recved_size : 0);

			return t_socket;
		}

		TCPSocket& operator =(TCPSocket& Right) {
			this->m_ptr = &Right;
			memcpy(&this->m_sa, &Right.m_sa, sizeof(sockaddr_in));
			this->m_sock = Right.m_sock;

			return *this;
		}

		// When object was used as pointer type, we can use this.
		TCPSocket& operator *(TCPSocket& Right) {
			return *this;
		}
#ifdef _DEBUG
		virtual void Testing() {
			cout << "[-] It is TCPSocket..." << endl;
		}
#endif
		/* Define Operate() function */
		virtual void Operate() {}

		/* Cleanup object's information, and close socket, then delete current object */
		virtual void Close() {
			CloseSocket();
			delete this;
		}
	};

	class TCPServer :public TCPSocket {
	private:
		SOCKET m_clnt_sock;			// Client's socket
		sockaddr_in m_clnt_sa;		// Client's Protocol & IP & Port

	public:
		/* Set port address for bind */
		TCPServer(PORT_ADDR port) :TCPSocket() {
			ZeroMemory(&m_clnt_sa, sizeof(sockaddr_in));

			m_sa.sin_family = AF_INET;
			m_sa.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
			m_sa.sin_port = htons(port);
		}
		~TCPServer() {}

		/*
		    execute Socket API function(bind(), listen(), accept()).
		    This function require server's Port Address with own socket for bind. And server is blocked for accept.
		 */
		void Operate() override {
			int32 result = 0;
			uint32 clnt_sa_size = sizeof(sockaddr_in);

			/* Commit registed socket option.*/
			ConfigCommit();
			
			result = bind(m_sock, (sockaddr*)&m_sa, sizeof(sockaddr_in));
			if (result == -1) {
				return;
			}
		
			// Current defined, CLIENTCONNECT = 5
			result = listen(m_sock, CLIENTCONNECT);
			if (result == -1) {
				return;
			}

			// Basic for accept functions
#ifndef _WIN32
			m_clnt_sock = accept(m_sock, (sockaddr*)&m_clnt_sa, (int*)&clnt_sa_size);
#else
			m_clnt_sock = WSAAccept(m_sock, (sockaddr*)&m_clnt_sa, (LPINT)&clnt_sa_size, NULL, NULL);
#endif
			if (m_clnt_sock == INVALID_SOCKET) {
				int error = WSAGetLastError();

				// 10014 : The system detected an invalid pointer address in attempting to use a pointer argument in a call.
				// => Initailize clnt_sasize = sizeof(sockaddr_in) 
				fprintf(stderr, "[-] Invalid clinet socket, because failed accept()\n");
				fprintf(stderr, "[-] %d\n", error);
				
				return;
			}
		}

#ifdef _DEBUG
		void Testing() override {
			cout << "[+] It is TCPServer..." << endl;
		}
#endif
		/* override Close() function, because for cleanup client address. */
		void Close() override {
			CloseSocket();
			ZeroMemory(&m_clnt_sa, sizeof(sockaddr_in));

			delete this;
		}
	};

	class TCPClient : public TCPSocket {
	private:

	public:
		// Set server's IP * Port Address that need to connect a server.
		TCPClient(IP_ADDR ip, PORT_ADDR port) :TCPSocket() {
			m_sa.sin_family = AF_INET;
#ifdef _WINSOCK_DEPRECATED_NO_WARNINGS
			m_sa.sin_addr.S_un.S_addr = inet_addr((const char*)ip);
#else
			InetPton(AF_INET, (PCWSTR)ip, &m_sa.sin_addr.S_un.S_addr);
#endif
			m_sa.sin_port = htons(port);
		}
		~TCPClient() {}

		/*
		    execute Socket API function(connect()).
		    This function require target's IP Address and Port Address with own socket for connect.
		 */
		void Operate() override {
			uint32 result = 0;

#ifndef _WIN32
			result = connect(m_sock, (sockaddr*)&m_sa, sizeof(sockaddr_in));
#else
			result = WSAConnect(m_sock, (sockaddr*)&m_sa, sizeof(sockaddr_in), NULL, NULL, NULL, NULL);
#endif
			if (result == -1) {
				int error = WSAGetLastError();

				fprintf(stderr, "[-] failed WSAConnect()\n");
				fprintf(stderr, "[-] %d\n", error);

				return;
			}
		}

#ifdef _DEBUG
		void Testing() override {
			cout << "[+] It is TCPClient..." << endl;
		}
#endif
	};
}

/* for store allocated TCPSocket(TCPServer or TCPClient) object. */
#ifndef _TCPSocket
	#define _TCPSocket Tribal::TCPSocket&
#endif

/* This is macro that create TCPSocket object with SocketFactory */
#define CreateTCPServer Tribal::SocketFactory::GetInstance()->CreateServer<Tribal::TCPServer>
#define CreateTCPClient Tribal::SocketFactory::GetInstance()->CreateClient<Tribal::TCPClient>
