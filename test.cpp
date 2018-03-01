#include "tcp.h"
#include <vector>
#include <iostream>

using std::vector;

int main(void) {
	char buf[256] = { 0 };
	_TCPSocket ts = CreateTCPServer(18502);            // Allocate TCPServer object that have a socket.
	_TCPSocket tc = CreateTCPClient((IP_ADDR)LOCALHOST, 18502); // Allocate TCPClient object that have a socket.

	ts.SetOption(1, 2);					// Option Setting.
	ts.Operate();						// listen.

	std::cout << "Send Test" << std::endl;
	ts << "123";						// Send.

	std::cout << "Recv Test" << std::endl;
	ts >> buf;							// Receive
	std::cout << buf << std::endl;

	return 0;
}