#include "pch.h"
#include <Windows.h>
#include <string>
#include <functional>
#include <thread>
#include <queue>

using namespace std;

class SerialHandler {
private:
	HANDLE h_serial;

	wstring port;
	int rate;

	bool is_connected;

	queue<string> transmit_queue;
public:
	SerialHandler(string port, int rate) {
		this->port = wstring(port.begin(), port.end());
		this->rate = rate;
		this->is_connected = false;
		this->h_serial = NULL;
		this->transmit_queue = queue<string>{};
	}

	int connect(function<void(char)> handler) {
		this->h_serial = CreateFile(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

		// if (this->isConnected) return 5;

		if (h_serial == INVALID_HANDLE_VALUE) {
			return 1;
		}

		DCB dcbSerialParams = { 0 };

		if (!GetCommState(h_serial, &dcbSerialParams)) {
			return 3;
		}

		dcbSerialParams.DCBlength = sizeof(DCB);
		dcbSerialParams.BaudRate = rate;
		dcbSerialParams.ByteSize = 8;
		dcbSerialParams.StopBits = ONESTOPBIT;
		dcbSerialParams.fBinary = TRUE;
		dcbSerialParams.Parity = NOPARITY;
		dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE;
		dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;

		if (!SetCommState(h_serial, &dcbSerialParams)) {
			return 4;
		}

		COMMTIMEOUTS CommTimeouts;
		CommTimeouts.ReadIntervalTimeout = MAXDWORD;
		CommTimeouts.ReadTotalTimeoutMultiplier = 10;
		CommTimeouts.ReadTotalTimeoutConstant = 100;
		CommTimeouts.WriteTotalTimeoutMultiplier = 0;
		CommTimeouts.WriteTotalTimeoutConstant = 0;

		// Set COM timeouts.
		if (!SetCommTimeouts(h_serial, &CommTimeouts)) {
			return 4;
		}


		HANDLE *h_serial = &this->h_serial;
		bool *isConnected = &this->is_connected;
		queue<string> *transmit_queue = &this->transmit_queue;

		this->is_connected = true;

		thread([h_serial, isConnected, handler, transmit_queue]() {
			DWORD iSize;
			char sReceivedChar;
			while (*isConnected) {

				OVERLAPPED osReader = { 0 };
				osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
				DWORD success = ReadFile(*h_serial, &sReceivedChar, 1, &iSize, &osReader);
				if (!success) {
					DWORD dwRes = WaitForSingleObject(osReader.hEvent, 2000);
					switch (dwRes) {
					case WAIT_OBJECT_0:
						GetOverlappedResult(*h_serial, &osReader, &iSize, FALSE);
						break;
					default:
						continue;
					}
				}

				if (iSize > 0) (handler)(sReceivedChar);
				CloseHandle(osReader.hEvent);
			}
		}).detach();

		return 0;
	}

	int disconnect() {

	}

	bool isConnected() {
		return this->is_connected;
	}

	int send(string payload) {
		OVERLAPPED osWrite = { 0 };
		DWORD dwRes;
		osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		// Issue write.
		payload += "\n";
		DWORD dwWritten;
		DWORD success = WriteFile(this->h_serial, payload.c_str(), payload.length(), &dwWritten, &osWrite);

		if (!success) {
			dwRes = ::WaitForSingleObject(osWrite.hEvent, INFINITE);
			switch (dwRes) {
			case WAIT_OBJECT_0:
				GetOverlappedResult(this->h_serial, &osWrite, &dwWritten, FALSE);
				break;
			default:
				break;
			}
			CloseHandle(osWrite.hEvent);
		}
		return 0;
	}

	~SerialHandler() {
	}
};