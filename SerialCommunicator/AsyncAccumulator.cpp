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
		this->accumulator = accumulator;
	}

	void append(char payload) {
		if (payload == this->separator) {
			this->subscriber(this->accumulator);
			this->accumulator = string("");
			return;
		}
		this->accumulator += payload;
	}

	~AsyncAccumulator() {}

};