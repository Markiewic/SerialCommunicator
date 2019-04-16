#include "pch.h"
#include <string>
#include <functional>

using namespace std;

class AsyncAccumulator {
private:

	string accumulator;
	char separator;
	function<void(string)> subscriber;

public:

	AsyncAccumulator(char separator, function<void(string)> subscriber) {
		this->accumulator = string("");
		this->separator = separator;
		this->subscriber = subscriber;
	}

	void append(char payload) {
		this->accumulator += payload;
		if (payload == this->separator) {
			this->subscriber(this->accumulator);
			this->accumulator = string("");
		}
	}

	~AsyncAccumulator() {}

};