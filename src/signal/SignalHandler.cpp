#include "SignalHandler.hpp"

namespace http {
	namespace core {

		void SignalHandler::setup() {
			_shutdown = 0;
			std::signal(SIGINT, SignalHandler::signal_handler);
			std::signal(SIGTERM, SignalHandler::signal_handler);
			std::signal(SIGPIPE, SIG_IGN);
		}

		void SignalHandler::signal_handler(int sig) {
			static_cast<void>(sig);
			_shutdown = 1;
		}

		volatile sig_atomic_t SignalHandler::get_shutdown() const { return _shutdown; }
	}
}

