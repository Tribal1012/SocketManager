#pragma once

#define null NULL

#define CLIENTCONNECT 1

#define CLEANUPOBJECT(x) {                          \
	if (x != NULL)                                  \
		delete x;                                   \
                                                    \
	x = NULL;                                       \
                                                    \
}

/* It is used for define SocketConfig class */
// #define EXIST_IPX
// #define TIME0
// #define EXIST_ACCEPT_CONTEXT

namespace Tribal {
#include <cstdlib>
	/*
	   basic template singleton
	 */
	template<typename T>
	class Singleton {
	private:
		static T* m_pInstance;

	private:
		static void destroy() {
			delete m_pInstance;
			m_pInstance = NULL;
		}

	protected:
		Singleton() {}

	public:
		//Singleton(const Sigleton& s) {}
		virtual ~Singleton() {}

		static T* GetInstance() {
			if (m_pInstance == NULL) {
				m_pInstance = new T();
				atexit(destroy);
			}

			return m_pInstance;
		}
	};

	/* command pattern sample */
	class Command {
	public:
		Command() {}
		virtual ~Command() {}
		virtual void execute() = 0;
	};

	template<typename T>
	class Sample {
	private:
	public:
		Sample() {}
		virtual ~Sample() {}

		T* sample() {
			return new T();
		}
	};


}