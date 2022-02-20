#include "fec.h"

namespace shining {
	PacketFec::PacketFec() {

	}

	PacketFec::~PacketFec() {}

	std::vector<std::tuple<uint8_t*, int>> PacketFec::mux(std::vector<std::tuple<uint8_t*, int>>& input, int redundancy) {
		std::vector<std::tuple<uint8_t*, int>> result(input.size() + redundancy);

		return result;
	}

	bool PacketFec::demux(std::vector<std::tuple<uint8_t*, int>>& input, std::vector<std::tuple<uint8_t*, int>>& output) {
		return true;
	}

}