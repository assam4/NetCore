#ifndef UTILS_HPP
# define UTILS_HPP

#include <string>
#include <sstream>

namespace http {
	namespace core {

		/**
		 * @brief Returns a lowercase copy of the input string.
		 * @param input The string to convert.
		 * @return A new std::string with all characters in lowercase.
		 */
		inline std::string to_lowercase(const std::string& input) {
			std::string result(input);
			for (std::string::size_type i = 0; i < result.length(); ++i)
				result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
			return result;
		}

		/**
		 * @struct is_numeric
		 * @brief Compile-time trait marking arithmetic types.
		 * @details Defaults to false and is specialized for numeric primitives.
		 *          Used to constrain generic to_string conversion helper.
		 */
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
		// template<> struct is_numeric<unsigned long> { enum { value = 1}; };
		template<> struct is_numeric<unsigned long long> { enum { value = 1}; };
		template<> struct is_numeric<size_t> { enum { value = 1}; };
		template<> struct is_numeric<float> { enum { value = 1}; };
		template<> struct is_numeric<double> { enum { value = 1}; };
		template<> struct is_numeric<long double> { enum { value = 1}; };

		template <typename T>
		std::string to_string(T value) {
			(void)sizeof(char[is_numeric<T>::value ? 1 : -1]);
			std::stringstream ss;
			ss << value;
			return ss.str();
		}

		/**
		 * @enum num_system
		 * @brief Represents numeric bases for string-to-integer conversion.
		 */
		enum num_system {
			OCTAL = 8,
			DECIMAL = 10,
			HEXDECIMAL = 16
		};

		/**
		 * @brief Converts a string to an unsigned integer in the specified base.
		 * @tparam N Numeric base (OCTAL, DECIMAL, HEXDECIMAL).
		 * @param str The string to convert.
		 * @return The converted unsigned integer.
		 * @throws std::logic_error If the string contains invalid digits for the base,
		 *         or if the template/base is unsupported.
		 * @details This function has template specializations for OCTAL, DECIMAL, and HEXDECIMAL.
		 *          The generic template throws a logic_error by default.
		 */
		template <size_t N>
		inline size_t atoul_base(const std::string&) {
			throw std::logic_error("Unsupported template/base.\n");
		}

		/** @brief Specialization for octal. */
		template <>
		inline size_t atoul_base<OCTAL>(const std::string& str) {
			size_t result = 0;
			for (size_t i = 0; i < str.length(); ++i) {
				if (str[i] >= '0' && str[i] <= '7')
					result = result * OCTAL + (str[i] - '0');
				else
					throw std::logic_error("atoul_base: Not a valid octal digit.\n");
			}
			return result;
		}

		/** @brief Specialization for decimal. */
		template <>
		inline size_t atoul_base<DECIMAL>(const std::string& str) {
			size_t result = 0;
			for (size_t i = 0; i < str.length(); ++i) {
				if (str[i] >= '0' && str[i] <= '9')
					result = result * DECIMAL + (str[i] - '0');
				else
					throw std::logic_error("atoul_base: Not a valid decimal digit.\n");
			}
			return result;
		}

		/** @brief Specialization for hexadecimal. */
		template <>
		inline size_t atoul_base<HEXDECIMAL>(const std::string& str) {
			size_t result = 0;
			for (size_t i = 0; i < str.length(); ++i) {
				if (str[i] >= '0' && str[i] <= '9')
					result = result * HEXDECIMAL + (str[i] - '0');
				else if (str[i] >= 'A' && str[i] <= 'F')
					result = result * HEXDECIMAL + (str[i] - 55);
				else if (str[i] >= 'a' && str[i] <= 'f')
					result = result * HEXDECIMAL + (str[i] - 87);
				else
					throw std::logic_error("atoul_base: Not a valid hexadecimal digit.\n");
			}
			return result;
		}

	}
}



#endif
