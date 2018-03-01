#include "tcp.h"
#include <vector>
#include <iostream>

using std::vector;

int main(void) {
	string sBuf;
	_TCPSocket ts = CreateTCPServer(18502);            // Allocate TCPServer object that have a socket.
	_TCPSocket tc = CreateTCPClient((IP_ADDR)LOCALHOST, 18502); // Allocate TCPClient object that have a socket.

	ts.SetOption(1, 2);					// Option Setting.
	ts.Operate();						// listen.

	std::cout << "Send Test" << std::endl;
	ts << "123";						// Send.

	std::cout << "Recv Test" << std::endl;
	ts >> sBuf;							// Receive
	std::cout << sBuf << std::endl;

	return 0;
}