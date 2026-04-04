#ifndef TO_STRING_HPP
# define TO_STRING_HPP

#include <cstdlib>
#include <string>
#include <sstream>

namespace http {
	namespace core {

		template <typename T>
		struct is_numeric {
			enum { value = 0 };
		};

		template<> struct is_numeric<int> { enum { value = 1}; };
		template<> struct is_numeric<short> { enum { value = 1}; };
		template<> struct is_numeric<long> { enum { value = 1}; };
		template<> struct is_numeric<long long> { enum { value = 1}; };
		template<> struct is_numeric<unsigned int> { enum { value = 1}; };
		template<> struct is_numeric<unsigned short> { enum { value = 1}; };
		template<> struct is_numeric<unsigned long> { enum { value = 1}; };
		template<> struct is_numeric<unsigned long long> { enum { value = 1}; };
		template<> struct is_numeric<float> { enum { value = 1}; };
		template<> struct is_numeric<double> { enum { value = 1}; };
		template<> struct is_numeric<long double> { enum { value = 1}; };

		template <typename T>
		std::string to_string(T value) {
				std::stringstream ss;
			ss << value;
			return ss.str();
		}

	}
}

#endif
