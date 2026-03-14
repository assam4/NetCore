#ifndef NETEXCEPTIONS_HPP
#define NETEXCEPTIONS_HPP

#include <exception>
#include <string>

class WeakError : public std::exception {
	private:
		std::string message;
	public:
		explicit WeakError(const std::string& msg);
		virtual ~WeakError() throw();
		virtual const char* what() const throw();
};

class StrictError : public std::exception {
	private:
		std::string message;
	public:
		explicit StrictError(const std::string& msg);
		virtual ~StrictError() throw();
		virtual const char* what() const throw();
};

#endif
