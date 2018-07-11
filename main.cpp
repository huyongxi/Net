#include "server.hpp"


int main(void) {
	struct sigaction act;
	act.sa_handler = SIG_IGN;
	if (sigaction(SIGPIPE, &act, NULL) == 0){
		cout << "SIGPIPE ignore" <<endl;
	}
	Server s("192.168.85.128",8000,4);
	s.start();
	return 0;
}