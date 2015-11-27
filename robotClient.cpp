#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;

int main(int argc, char *argv[]) {
	string serverName;
	in_port_t serverPort;
	int lenOfSides, numOfSides;

	if (argc != 9) {
		cout << "Usage: " << argv[0] << " -h <hostname-of-server> -p <port> " <<
			"-n <number-of-sides> -l <length-of-sides>" << endl;
		exit(1);
	}

	for (int i = 0; i < 9; i++) {
		string argvStr = argv[i];
		if (argvStr.compare("-h") == 0) {
			serverName = argv[i+1];
		} else if (argvStr.compare("-l") == 0) {
			lenOfSides = atoi(argv[i+1]);
		} else if (argvStr.compare("-n") == 0) {
			numOfSides = atoi(argv[i+1]);
		} else if (argvStr.compare("-p") == 0) {
			serverPort = atoi(argv[i+1]);
		}
	}

}