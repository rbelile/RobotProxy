#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;

int main(int argc, char *argv[]) {
	string hostName;
	int robotID, robotNum;
	in_port_t port;

	if (argc != 9) {
		cout << "Usage: " << argv[0] << " -h <hostname-of-robot> -i <robot-id> -n " <<
			"<robot-number> -p <port>" << endl;
		exit(1);
	}

	for (int i = 0; i < 9; i++) {
		string argvStr = argv[i];
		if (argvStr.compare("-h") == 0) {
			hostName = argv[i+1];
		} else if (argvStr.compare("-i") == 0) {
			robotID = atoi(argv[i+1]);
		} else if (argvStr.compare("-n") == 0) {
			robotNum = atoi(argv[i+1]);
		} else if (argvStr.compare("-p") == 0) {
			port = atoi(argv[i+1]);
		}
	}
}