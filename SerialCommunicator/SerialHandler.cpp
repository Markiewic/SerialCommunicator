#include "pch.h"
#include <Windows.h>
#include <string>
#include <functional>
#include <thread>

using namespace std;

class SerialHandler {
private:

	HANDLE h_serial;
	wstring port;
	int rate;
	bool is_connected;

public:

	SerialHandler(string port, int rate) {
		port = "\\\\.\\" + port;
		this->port = wstring(port.begin(), port.end());
		this->rate = rate;
		this->is_connected = false;
		this->h_serial = NULL;
	}

	int connect(function<void(char)> on_char, function<void(int)> on_error) {
		// Error 1: Port is already connected
		if (this->is_connected) return 1;

		this->h_serial = CreateFile(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

		// Error 2: Error opening port
		if (h_serial == INVALID_HANDLE_VALUE) return 2;

		DCB dcbSerialParams = { 0 };

		// Error 3: Error getting CommState
		if (!GetCommState(h_serial, &dcbSerialParams)) return 3;

		// Configuring port
		dcbSerialParams.DCBlength = sizeof(DCB);
		dcbSerialParams.BaudRate = rate;
		dcbSerialParams.ByteSize = 8;
		dcbSerialParams.StopBits = ONESTOPBIT;
		dcbSerialParams.fBinary = TRUE;
		dcbSerialParams.Parity = NOPARITY;
		dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE;
		dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;

		// Error 4: Error setting CommState
		if (!SetCommState(h_serial, &dcbSerialParams)) return 4;

		// Configuring Reading & Writing timeouts
		COMMTIMEOUTS CommTimeouts;
		CommTimeouts.ReadIntervalTimeout = MAXDWORD;
		CommTimeouts.ReadTotalTimeoutMultiplier = 10;
		CommTimeouts.ReadTotalTimeoutConstant = 100;
		CommTimeouts.WriteTotalTimeoutMultiplier = 0;
		CommTimeouts.WriteTotalTimeoutConstant = 0;

		// Error 5: Error setting CommTimeouts
		if (!SetCommTimeouts(h_serial, &CommTimeouts)) return 5;

		this->is_connected = true;

		// Getting a pointers for passing to the thread
		HANDLE *h_serial = &this->h_serial;
		bool *is_connected = &this->is_connected;
		
		// Creating a thread
		thread([h_serial, is_connected, on_char, on_error]() {

			DWORD readed_size;
			char readed_char;
			OVERLAPPED reader_event;

			// This code will be looping while the is_connected variable is true
			// Else the thread will be compeleted & destroyed
			while (*is_connected) {

				// Creating receiving event
				reader_event = { 0 };
				reader_event.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

				// Trying to read the input
				if (!ReadFile(*h_serial, &readed_char, 1, &readed_size, &reader_event)) {
					// If input is empty waiting for it
					//DWORD event_res = ;

					switch (WaitForSingleObject(reader_event.hEvent, 1000)) {
					case WAIT_OBJECT_0:
						// Error 1: Error in communications, calling lambda-function
						if (!GetOverlappedResult(*h_serial, &reader_event, &readed_size, FALSE)) on_error(1);
						break;
					default:
						continue;
					}
				}

				// Closing the event
				if (reader_event.hEvent != NULL) CloseHandle(reader_event.hEvent);

				// Calling lamda-function with result
				if (readed_size > 0) (on_char)(readed_char);

			}
		}).detach();
		// Leaving it by itself & returning zero

		return 0;
	}

	// Overloading function above
	int connect(function<void(char)> on_char) {
		return this->connect(on_char, [](int error) {});
	}

	int disconnect() {
		this->is_connected = false;
	}

	bool connected() {
		return this->is_connected;
	}

	int send(string payload, DWORD *written_size) {
		OVERLAPPED reader_event = { 0 };
		
		reader_event.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		if (!WriteFile(this->h_serial, payload.c_str(), (DWORD)payload.length(), written_size, &reader_event)) {
			switch (WaitForSingleObject(reader_event.hEvent, INFINITE)) {
			case WAIT_OBJECT_0:
				if (!GetOverlappedResult(this->h_serial, &reader_event, written_size, FALSE)) return 1;
				break;
			default:
				break;
			}
			if (reader_event.hEvent != NULL) CloseHandle(reader_event.hEvent);
		}
		return 0;
	}

	int send(string payload) {
		DWORD written_size;
		this->send(payload, &written_size);
		return written_size;
	}

	~SerialHandler() {
	}
};