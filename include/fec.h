#pragma once

#include <vector>
#include <tuple>


namespace shining {
	class PacketFec {
	public:
		PacketFec();
		~PacketFec();

		std::vector<std::tuple<uint8_t*, int>> mux(std::vector<std::tuple<uint8_t*, int>>& input, int redundancy);

		bool demux(std::vector<std::tuple<uint8_t*, int>>& input, std::vector<std::tuple<uint8_t*, int>>& output);
	};
}