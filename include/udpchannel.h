#pragma once
#include <string>
#include <functional>
namespace shining {

	typedef std::function<void(uint8_t*, int)> MSG_CALLBACK;

	class Channel {
	public:
		Channel() = default;
		virtual void SendMsg(uint8_t* data, int size) = 0;
		void SetMsgCallback(MSG_CALLBACK callback) { msgCallback = callback; }
		MSG_CALLBACK msgCallback;
	};
}