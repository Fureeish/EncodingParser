#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <exception>
#include <type_traits>
#include <vector>

#include <boost/format.hpp>

namespace {
/**
 * Takes any T that can represent a file path (std::string, const char[], etc.),
 * opens a file and returns its content packed in std::string.
 *
 * @tparam T    - the type that can be used to initialize std::fstream object
 * @param t     - the parameter to initialize std::fstream object to open a file
 * @return      - returns the content of a file specified by @t in form of a
 *                std::string
 */
    template<typename T,
            typename = std::enable_if_t<std::is_convertible_v<T, std::string>>>
    auto read_file(const T &t) {
        std::ifstream file{t};
        if (!file) {
            throw std::runtime_error{"File not found."};
        }

        std::ostringstream in{};
        in << file.rdbuf();

        return in.str();
    }

/**
 * Returns the @bit-th bit of @c.
 *
 * @param c     - the character we're inspecting
 * @param bit   - the bit number we want to obtain (7th is most significant)
 * @return      - returns the @bit-th bit of @c
 */
    auto nth_bit(const uint8_t c, const uint8_t bit) noexcept {
        return (c >> bit) & 1;
    }

/**
 * Checks whether @c is a ASCII character.
 *
 * @param c - the character we're inspecting
 * @return  - returns true or false - depending on whether @c can be encoded
 *            by using only a single byte (is an ASCII character)
 */
    auto is_ASCII(const uint8_t c) noexcept {
        return nth_bit(c, 7) == 0;
    }

/**
 * Adds a new character from @file_content to @values.
 *
 * @param file_content      - string representation of a file
 * @param values            - container to add parsed characters
 * @param index             - number of currently inspected byte
 * @param current_char      - currently inspected byte
 * @param number_of_bytes   - number of bytes needed to parse the character
 */
    void add_char(const std::string &file_content, std::vector<uint32_t> &values,
                  size_t index, const uint8_t current_char,
                  const int number_of_bytes) {
        const uint8_t first_byte_mask = 0b00011111;
        const uint8_t following_byte_mask = 0b00111111;
        const auto &fc = file_content;
        switch (number_of_bytes) {
            case 1: {
                uint8_t first = current_char & first_byte_mask;
                uint8_t second =
                        static_cast<uint8_t>(fc[index + 1]) & following_byte_mask;
                values.emplace_back((first << 6) | second);
                break;
            }
            case 2: {
                uint8_t first = current_char & first_byte_mask;
                uint8_t second =
                        static_cast<uint8_t>(fc[index + 1]) & following_byte_mask;
                uint8_t third = static_cast<uint8_t>(fc[index + 2]) & following_byte_mask;
                uint32_t result = (first << 16) | (second << 12) | third;
                values.emplace_back(result);
                break;
            }
            case 3: {
                uint8_t first = current_char & first_byte_mask;
                uint8_t second =
                        static_cast<uint8_t>(fc[index + 1]) & following_byte_mask;
                uint8_t third = static_cast<uint8_t>(fc[index + 2]) & following_byte_mask;
                uint8_t fourth =
                        static_cast<uint8_t>(fc[index + 3]) & following_byte_mask;
                uint32_t result = (first << 22) | (second << 18) | (third << 12) | fourth;
                values.emplace_back(result);
                break;
            }
            default:
                throw std::logic_error{"Incorrect number of bytes to parse: "
                                       + std::to_string(number_of_bytes)};
        }
    }

/**
 *
 * @param file_content  - the content of a file in bytes
 * @return              - a std::vector of uint32_t, where each elements
 *                        represents a Unicode value of a consecutive character
 *                        from @file_content
 */
    auto to_unicode_values(const std::string &file_content) {
        std::vector<uint32_t> values{};

        std::size_t index = 3;
        while (index < file_content.size()) {
            const auto current_char = static_cast<uint8_t>(file_content[index]);
            if (is_ASCII(current_char)) {
                values.emplace_back(current_char);
                ++index;
            } else {
                /* UTF-8 character - requires parsing (we won't need more than 4
                 * bytes per character, because of the current number of Unicode chars) */
                const auto number_of_bytes = nth_bit(current_char, 6)
                                             + nth_bit(current_char, 5)
                                             + nth_bit(current_char, 4);

                if (index + number_of_bytes >= file_content.size()) {
                    throw std::length_error{
                            "File structure suggests more characters, but "
                            "reached end of file on byte: "
                            + std::to_string(index)};
                }
                add_char(file_content, values, index, current_char, number_of_bytes);
                index += number_of_bytes + 1;
            }
        }
        return values;
    }
}

int main(int argc, char** argv) try {
    const char* path = ""; // not null since we want a default path to not break ifstream
    if(argc == 2) {
        path = argv[1];
    } else {
        std::cerr << "Expected a file path\n";
        return 1;
    }
    const auto text    = read_file(path);
    const auto unicode = to_unicode_values(text);

    std::cout << "There are " << unicode.size() << " UTF-8 characters:\n";

    boost::format num_format{"%4i"};
    for (std::size_t i = 0; i < unicode.size(); ++i) {
        std::cout << boost::str(num_format % unicode[i]);
        if((i + 1) % 10 == 0) std::cout << '\n';
    }
} catch (const std::exception &ex) {
    std::cerr << ex.what();
}
