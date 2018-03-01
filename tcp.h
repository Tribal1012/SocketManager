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
			m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			return m_sock;
		}
		
		void CloseSocket() override {
			closesocket(m_sock);
		}

		int32 SendData(ccData data, const size_t len) override {
#ifdef _DEBUG
			std::cout << "[+] TCPData Send... - " << len << std::endl;
			std::cout << data << std::endl;
#endif
			return send(m_sock, data, len, 0);
		}

		int32 RecvData(cData data, const size_t max) override {
#ifdef _DEBUG
			std::cout << "[+] TCPData Recv... - " << max << std::endl;
			std::cout << data << std::endl;
#endif
			return recv(m_sock, data, max, 0);
		}

		TCPSocket() :Socket() {
			ZeroMemory(&m_sa, sizeof(sockaddr_in));
			m_ptr = null;
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

		TCPSocket& operator *(TCPSocket& Right) {
			// When object was used as pointer type, we can use this.
			return *this;
		}
#ifdef _DEBUG
		virtual void Testing() {
			cout << "[-] It is TCPSocket..." << endl;
		}
#endif
		virtual void Operate() {
			
		}
	};

	class TCPServer :public TCPSocket {
	private:
		SOCKET m_clnt_sock;			// Client's socket
		sockaddr_in m_clnt_sa;		// Client's Protocol & IP & Port

	private:
		/*
		    execute Socket API function(bind(), listen()).
		    This function require server's Port Address with own socket for bind.
		 */
		bool ListenServer() {
			uint32 result = 0;

			result = bind(m_sock, (sockaddr*)&m_sa, sizeof(sockaddr_in));
			if (result == -1) {
				return false;
			}

			result = listen(m_sock, CLIENTCONNECT);
			if (result == -1) {
				return false;
			}

			return true;
		}

	public:
		TCPServer(PORT_ADDR port) :TCPSocket() {
			ZeroMemory(&m_clnt_sa, sizeof(sockaddr_in));

			m_sa.sin_family = AF_INET;
			m_sa.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
			m_sa.sin_port = htons(port);
		}
		~TCPServer() {}

		/*
		    execute Socket API function(accept()).
			server is blocked for accept.
		 */
		void Operate() override {
			uint32 clnt_sa_size = 0;

			m_clnt_sock = accept(m_sock, (sockaddr*)&m_clnt_sa, (int*)&clnt_sa_size);
			if (m_clnt_sock == -1) return;
		}

#ifdef _DEBUG
		void Testing() override {
			cout << "[+] It is TCPServer..." << endl;
		}
#endif
	};

	class TCPClient : public TCPSocket {
	private:
		
	public:
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
		void remote() {
			uint32 result = 0;

			result = connect(m_sock, (sockaddr*)&m_sa, sizeof(sockaddr_in));
			if (result == -1) return;
		}

#ifdef _DEBUG
		void Testing() override {
			cout << "[+] It is TCPClient..." << endl;
		}
#endif

		void Operate() override {}
	};
}

#ifndef _TCPSocket
	#define _TCPSocket Tribal::TCPSocket&
#endif

/* This is macro that create TCPSocket object with ObjectFactory */
#define CreateTCPServer Tribal::ObjectFactory::GetInstance()->CreateServer<Tribal::TCPServer>
#define CreateTCPClient Tribal::ObjectFactory::GetInstance()->CreateClient<Tribal::TCPClient>
