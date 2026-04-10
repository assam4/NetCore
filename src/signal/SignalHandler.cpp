#include "SignalHandler.hpp"

namespace http {
	namespace core {


		volatile sig_atomic_t http::core::SignalHandler::_shutdown = 0;

		void SignalHandler::setup() {
			std::signal(SIGINT, SignalHandler::signal_handler);
			std::signal(SIGTERM, SignalHandler::signal_handler);
			std::signal(SIGPIPE, SIG_IGN);
		}

		void SignalHandler::signal_handler(int sig) {
			static_cast<void>(sig);
			_shutdown = 1;
		}

		sig_atomic_t SignalHandler::get_shutdown() { return _shutdown; }
	}
}

