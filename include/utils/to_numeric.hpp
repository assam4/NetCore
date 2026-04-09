#ifndef TO_NUMERIC_HPP
# define TO_NUMERIC_HPP

# include <exception>
# include <string>
# include <cstdlib>

namespace http {
    namespace utils {

        enum num_system {
            OCTAL = 8,
            DECIMAL = 10,
            HEXDECIMAL = 16
        };
        
        template <size_t N>
        inline size_t	atoul_base(const std::string&) {
			throw std::logic_error("Unsupported template/base.\n");
		}
        template <>
        inline size_t	atoul_base<OCTAL>(const std::string& str) {
			size_t	result = 0;
			for (size_t i = 0; i < str.length(); ++i) {
				if (str[i] >= '0' && str[i] <= '7')
					result = result * HEXDECIMAL + (str[i] - '0');
				else
                    throw std::logic_error("atoul_base: Not a valid octal digit.\n");
            }
            return result;
		}

        template <>
        inline size_t	atoul_base<DECIMAL>(const std::string& str) {
			size_t	result = 0;
			for (size_t i = 0; i < str.length(); ++i)
				if (str[i] >= '0' && str[i] <= '9')
					result = result * DECIMAL + (str[i] - '0');
				else
                    throw std::logic_error("atoul_base: Not a valid decimal digit.\n");
            return result;
		}

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