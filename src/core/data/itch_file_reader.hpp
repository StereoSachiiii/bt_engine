#pragma once
#include <cstdio>
#include <cstdint>
#include <cstring>

class ITCHReader {

private:
	FILE* file_ = nullptr;
	static constexpr size_t buffer_size = 8192;  // Max message size

public:

	~ITCHReader() {
		close();
	}

	void close() {

		if (!file_) return;
		fclose(file_);
		file_ = nullptr;
	}

	bool open(const char* filename) {

		file_ = fopen(filename, "rb");
		return file_ != nullptr;

	}

	bool read_next(uint8_t* buffer, size_t& length) {
		if (!file_) return false;


		uint8_t len_bytes[2];
		if (fread(len_bytes, 1, 2, file_) != 2) {		
			return false;
		}

		// a little bit of bitwise math for it. basically
		// shift 8 bytes to the left and OR them . it's like
		// 1,0 -> 10 OR 1 -> 11 , but the thing is isnt this pushing the 
		// first byte beyond? because it's like pushed beyond the first 8 bytes 
		//to the left , why does the compiler do allow this

		//i use static cast to sidestep that at least show intent even though coversion is implicit
		uint16_t msg_len = (static_cast<uint16_t>(len_bytes[0]) << 8) | static_cast<uint16_t>(len_bytes[1]);

		if (msg_len == 0 || msg_len > buffer_size) {
			return false;
		}

		if (fread(buffer, 1, msg_len, file_) != msg_len) {
			return false;
		}




		length = msg_len;
		return true;
	}
};
