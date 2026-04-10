#ifndef SIGNAL_HANDLER_HPP
# define SIGNAL_HANDLER_HPP

# include <csignal>

namespace http {
	namespace core {

		/**
		 * @class SignalHandler
		 * @brief Centralized process signal management helper.
		 * @details Installs handlers for shutdown-related OS signals.
		 *          Exposes an atomic shutdown flag polled by the main loop.
		 */
		class SignalHandler {
			private:
				static volatile sig_atomic_t _shutdown;

				SignalHandler();
				SignalHandler(const SignalHandler& );
				SignalHandler& operator=(const SignalHandler&);
				static void signal_handler(int sig);
			public:
				static void setup();
				static sig_atomic_t get_shutdown();
		};

	}
}

#endif
