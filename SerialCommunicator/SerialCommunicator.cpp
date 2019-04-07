#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <string>
#include <thread>
#include "SerialHandler.cpp"

using namespace std;

int main(int argc, char* argv[]) {
	string port;
	string rate_str;
	bool crlf = false;

	for (int i = 0; i < argc; i++) {
		if (string(argv[i]) == "-port") {
			port = argv[++i];
		}
		else if (string(argv[i]) == "-rate") {
			rate_str = argv[++i];
		}
		else if (string(argv[i]) == "-crlf") {
			crlf = true;
		}
	}

	int rate = stoi(rate_str);
	cout << "PORT: " << port.c_str() << ", RATE: " << rate << endl;

	SerialHandler connection = SerialHandler(port, rate);
	int result = connection.connect([](char symb) {
		cout << symb;
	});

	string tr_data;
	while (true) {
		getline(cin, tr_data);
		if (crlf) tr_data += "\n";
		connection.send(tr_data);
	}
	
	return 0;

}