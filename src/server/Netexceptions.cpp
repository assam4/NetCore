#include "NetExceptions.hpp"

WeakError::WeakError(const std::string& msg) : message(msg) {}

WeakError::~WeakError() throw() {}

const char* WeakError::what() const throw() {
	return message.c_str();
}

StrictError::StrictError(const std::string& msg) : message(msg) {}

StrictError::~StrictError() throw() {}

const char* StrictError::what() const throw() {
	return message.c_str();
}
