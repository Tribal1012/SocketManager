#pragma once
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <vector>
#include "type.h"
#include "define.h"

using std::vector;

#define LOCALHOST "127.0.0.1"
#define ADDR_LEN  16

typedef puint8 IP_ADDR;
typedef uint16 PORT_ADDR;

namespace Tribal {
	/*
	SocketLevel used setsockopt() function.
	(https://msdn.microsoft.com/en-us/library/windows/desktop/ms740476(v=vs.85).aspx).
	*/
	typedef enum _SocketLevel {
		Socket_Level = SOL_SOCKET,
		IPProto = IPPROTO_TCP
#ifdef EXIST_IPX
		,
		NSProto = NSPROTO_IPX
#endif
	} SocketLevel;
	/*
	SocketOption is used setsockopt() function.
	(https://msdn.microsoft.com/en-us/library/windows/desktop/ms740476(v=vs.85).aspx).
	*/
	typedef enum _SocketOption {
		Broadcast = SO_BROADCAST,
		Conditional_Accept = SO_CONDITIONAL_ACCEPT,
		Debug = SO_DEBUG,
		Linger = SO_LINGER,
		DontLinger = SO_DONTLINGER,
		DontRoute = SO_DONTROUTE,
		KeepAlive = SO_KEEPALIVE,
		OOBinLine = SO_OOBINLINE,
		RCVBUF = SO_RCVBUF,
		SNDBUF = SO_SNDBUF,
		ReuseAddr = SO_REUSEADDR,
		ExclusiveAddrUse = SO_EXCLUSIVEADDRUSE,

#ifdef TIME0
		RCVTIME0 = SO_RCVTIME0,
		SNDTIME0 = SO_SNDTIME0,
#endif

#ifdef EXIST_ACCEPT_CONTEXT
		Update_Accept_Context = SO_UPDATE_ACCEPT_CONTEXT,
#endif

		PVD_Config = PVD_CONFIG,
		NoDelay = TCP_NODELAY

#ifdef EXIST_IPX
		,
		PType = IPX_PTYPE,
		FilterType = IPX_FILTERTYPE,
		StopFilterType = IPX_STOPFILTERPTYPE,
		DSType = IPX_DSTYPE,
		Extended_Address = IPX_EXTENDED_ADDRESS,
		RecvHDR = IPX_RECVHDR,
		RCVBroadcast = IPX_RECEIVE_BROADCAST,
		ImmeditatesPXACK = IPX_IMMEDIATESPXACK
#endif
	} SocketOption;

	class SocketConfig {
	private:
		static SocketConfig* m_pInstance;

	private:
		vector<SOCKET> m_sock;
		vector<uint32> m_level;
		vector<uint32> m_option;
		vector<const char*>  m_value;
		vector<uint32> m_len;

	private:
		/* for Singleton pattern */
		SocketConfig() {}

		/* Callback function for clean Instance */
		static void DestroyInstance() {
			delete m_pInstance;
			m_pInstance = NULL;
		}

	public:
		~SocketConfig() {}

		static SocketConfig* GetInstance() {
			if (m_pInstance == null) {
				m_pInstance = new SocketConfig();
				atexit(DestroyInstance);
			}

			return m_pInstance;
		}

		SocketConfig* RegisterOption(SOCKET sock, uint32 level, uint32 opt, void* value, uint32 len) {
			m_sock.push_back(sock);
			m_level.push_back(level);
			m_option.push_back(opt);
			m_value.push_back((const char*)value);
			m_len.push_back(len);

			return this;
		}

#define EMPTY_CHECK(x) { if(x.empty()) { ResetOption(); return false; } }
#define SIZE_CHECK(a, b, c, d, e) { if(!(a.size() == b.size() == c.size() == d.size() == e.size())) { ResetOption(); return false; } }
		bool Apply() {
			EMPTY_CHECK(m_sock)
			EMPTY_CHECK(m_level)
			EMPTY_CHECK(m_option)
			EMPTY_CHECK(m_value)
			EMPTY_CHECK(m_len)
			SIZE_CHECK(m_sock, m_level, m_option, m_value, m_len)

			for (uint32 i = 0; i < m_sock.size(); i++) {
				int result = 0;

#ifdef _DEBUG
				SOCKET sock = m_sock.at(i);
				int level = m_level.at(i);
				int opt = m_option.at(i);
				const char* val = m_value.at(i);
				int len = m_len.at(i);
				printf("[?] setsockopt(%x %d %d %x %d);", sock, level, opt, val, len);
#endif
				result = setsockopt(m_sock.at(i), m_level.at(i), m_option.at(i), m_value.at(i), m_len.at(i));
				if (result == SOCKET_ERROR) {
#ifdef _DEBUG
					int error = WSAGetLastError();
					printf("%d\n", error);
#endif
					//ResetOption();
					//return false;
				}
			}

			ResetOption();

			return true;
		}

		void ResetOption() {
			m_sock.clear();
			m_level.clear();
			m_option.clear();
			m_value.clear();
			m_len.clear();
		}
	};
	SocketConfig* SocketConfig::m_pInstance = nullptr;

	/*
	define socket's type like TCP or UDP for CreateSocket()
	Hmm...
	*/
	typedef enum _SocketType {
		TCP = 0,
		UDP
	} SocketType;

	/* Abstract Class */
	template <typename T>
	class Socket {
	private:
		SocketConfig* m_sockconfig;      // for handle setsockopt()

	protected:
		SOCKET m_sock;					// Object Socket allocated socket.

	protected:
		virtual SOCKET CreateSocket() = 0;
		virtual void CloseSocket() = 0;
		virtual int32 SendData(ccData data, const size_t len) = 0;
		virtual int32 RecvData(cData data, const size_t max) = 0;

		Socket() {
			m_sockconfig = SocketConfig::GetInstance();
		}

		/* Commit socket options with SocketConfig object using setsockopt(). */
		bool ConfigCommit() { 
			if (!m_sockconfig) return false;

			bool result = m_sockconfig->Apply();
			if (result) {
				delete m_sockconfig;
				m_sockconfig = null;
			} else fprintf(stderr, "[-] Failed to commit socket option\n");

			return  result;
		}

	public:
		virtual ~Socket() {}

		/* Repack SocketConfig's RegisterOption() */
		Socket* SetOption(uint32 level, uint32 opt, void* value = null, uint32 len = 0) {
			if ((value == null && len != 0) && (value != null && len == 0)){
				fprintf(stderr, "[-] SetOption error, Please check value and len.\n");

				return this;
			}

			/* should fetch the return value. */
			if(m_sockconfig) m_sockconfig->RegisterOption(m_sock, level, opt, value, len);
			else fprintf(stderr, "[-] It has been already commited.\n");
			
			return this;
		}
	};

	class SocketFactory {
	private:
		static SocketFactory* m_pInstance;			// return itself

	private:
		SocketFactory() {}
		~SocketFactory() {}

		// for clean Socket's Instance
		static void DestroyInstance() {
			delete m_pInstance;
			m_pInstance = NULL;
		}

	public:
		static SocketFactory* GetInstance() {
			if (m_pInstance == null) {
				m_pInstance = new SocketFactory();
				atexit(DestroyInstance);
			}

			return m_pInstance;
		}

		/* for server object */
		template <typename T>
		T& CreateServer(PORT_ADDR port) {
			// It don't return pointer type.
			return *(new T(port));
		}

		/* for client object */
		template <typename T>
		T& CreateClient(IP_ADDR ip, PORT_ADDR port) {
			// It don't return pointer type.
			return *(new T(ip, port));
		}

		//template <typename T>
		//T& CreateObject() {
		//	// It don't return pointer type.
		//	return *(new T());
		//}
	};
	SocketFactory* SocketFactory::m_pInstance = nullptr;
}