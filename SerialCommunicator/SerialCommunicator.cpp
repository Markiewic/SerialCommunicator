#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <string>
#include <thread>
#include "SerialHandler.cpp"

using namespace std;

HANDLE h_serial;

void ReadCOM();

int main(int argc, char* argv[]) {

	string port_str = "\\\\.\\";
	string rate_str;

	for (int i = 0; i < argc; i++) {
		if (string(argv[i]) == "-port") {
			port_str += argv[++i];
		}
		else if (string(argv[i]) == "-rate") {
			rate_str = argv[++i];
		}
	}

	cout << "PORT: " << port_str.c_str() << ", RATE: " << rate_str.c_str() << endl;

	wstring port = wstring(port_str.begin(), port_str.end());
	int rate = stoi(rate_str);

	SerialHandler connection = SerialHandler(port_str, rate);
	int result = connection.connect([](char symb) {
		cout << symb;
	});

	string tr_data;
	while (true) {
		getline(cin, tr_data);
		connection.send(tr_data);
	}
	
	return 0;

}