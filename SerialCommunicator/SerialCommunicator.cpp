#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <string>
#include <thread>

using namespace std;

HANDLE h_serial;

void ReadCOM();

int main(int argc, char* argv[]) {

	string port_str;
	string rate_str;

	for (int i = 0; i < argc; i++) {
		if (string(argv[i]) == "-port") {
			port_str = argv[++i];
		}
		else if (string(argv[i]) == "-rate") {
			rate_str = argv[++i];
		}
	}

	cout << "PORT: " << port_str.c_str() << ", RATE: " << rate_str.c_str() << endl;
	
	wstring port = wstring(port_str.begin(), port_str.end());
	int rate = stoi(rate_str);

	h_serial = CreateFile(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (h_serial == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			cout << "serial port does not exist.\n";
		}
		cout << "some other error occurred.\n";
	}

	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	if (!GetCommState(h_serial, &dcbSerialParams)) {
		cout << "getting state error\n";
	}

	dcbSerialParams.BaudRate = rate;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!SetCommState(h_serial, &dcbSerialParams))
	{
		cout << "error setting serial port state\n";
	}

	char data[] = "Hello from C++";
	DWORD dwSize = sizeof(data);
	DWORD dwBytesWritten;

	BOOL iRet = WriteFile(h_serial, data, dwSize, &dwBytesWritten, NULL);

	thread([]() {
		while (true)
		{
			ReadCOM();
		}
	}).join();

	return 0;

}

void ReadCOM()
{
	DWORD iSize;
	char sReceivedChar;
	while (true)
	{
		ReadFile(h_serial, &sReceivedChar, 1, &iSize, 0);
		if (iSize > 0) cout << sReceivedChar;
	}
}