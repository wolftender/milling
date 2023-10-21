#include <ios>
#include <string>
#include <iostream>

#include "parser.hpp"

namespace mini {
	milling_command_parser::milling_command_parser(const std::string& path) : m_stream(path) { 
		m_previous_line = 0;

		m_parsers[1] = std::bind(&milling_command_parser::m_read_g01_command, this, std::placeholders::_1, std::placeholders::_2);
	}

	bool milling_command_parser::is_good() const {
		return m_stream.good();
	}

	std::optional<milling_command> milling_command_parser::get_next_command() {
		std::string line;
		std::string::const_iterator iter;

		if (std::getline(m_stream, line)) {
			iter = line.begin();

			if (m_get(line, iter) != 'N') {
				return std::nullopt;
			}

			auto line_number_opt = m_try_read_int(line, iter);
			if (!line_number_opt.has_value()) {
				return std::nullopt;
			}

			auto line_number = line_number_opt.value();
			if (line_number - m_previous_line != 1) {
				std::cerr << "milling format warning: line is " << line_number << ", previous was " << m_previous_line << std::endl;
			}

			if (m_get(line, iter) != 'G') {
				return std::nullopt;
			}

			auto command_number_opt = m_try_read_int(line, iter);
			if (!command_number_opt.has_value()) {
				return std::nullopt;
			}

			auto command_number = command_number_opt.value();
			if (command_number < 0 || command_number > 99) {
				return std::nullopt;
			}

			auto parser = m_parsers.find(command_number);
			if (parser == m_parsers.end()) {
				return std::nullopt;
			}

			m_previous_line = line_number;
			return parser->second(line, iter);
		}

		return std::nullopt;
	}

	std::vector<milling_command> milling_command_parser::get_commands() {
		auto command = get_next_command();
		std::vector<milling_command> commands;

		while (command.has_value()) {
			commands.push_back(command.value());
			command = get_next_command();
		}

		return commands;
	}

	milling_command milling_command_parser::m_read_g01_command(const std::string& line, std::string::const_iterator& iter) const {
		if (m_get(line, iter) != 'X') {
			return milling_command(command_invalid());
		}

		auto x_opt = m_try_read_float(line, iter);

		if (!x_opt.has_value()) {
			return milling_command(command_invalid());
		}

		if (m_get(line, iter) != 'Y') {
			return milling_command(command_invalid());
		}

		auto y_opt = m_try_read_float(line, iter);

		if (!y_opt.has_value()) {
			return milling_command(command_invalid());
		}

		if (m_get(line, iter) != 'Z') {
			return milling_command(command_invalid());
		}

		auto z_opt = m_try_read_float(line, iter);

		if (!z_opt.has_value()) {
			return milling_command(command_invalid());
		}

		return milling_command(command_g01_t{
			x_opt.value(), y_opt.value(), z_opt.value()
		});
	}

	char milling_command_parser::m_peek(const std::string& line, std::string::const_iterator& iter) const {
		if (iter == line.end()) {
			return '\0';
		}

		return *iter;
	}

	char milling_command_parser::m_get(const std::string& line, std::string::const_iterator& iter) const {
		if (iter == line.end()) {
			return '\0';
		}

		auto ch = *iter;
		iter++;

		return ch;
	}

	std::optional<int> milling_command_parser::m_try_read_int(const std::string& line, std::string::const_iterator& iter) const {
		auto begin = iter;
		auto first_char = m_peek(line, iter);

		if (first_char < '0' || first_char > '9') {
			return std::nullopt;
		}

		while (iter != line.end()) {
			if (*iter >= '0' && *iter <= '9') {
				iter++;
			} else {
				break;
			}
		}

		return std::stoi(std::string(begin, iter));
	}

	std::optional<float> milling_command_parser::m_try_read_float(const std::string& line, std::string::const_iterator& iter) const {
		auto begin = iter;
		bool after_dot = false;
		int chars_after_dot = 0;

		while (iter != line.end()) {
			if (*iter >= '0' && *iter <= '9') {
				iter++;

				if (after_dot) {
					chars_after_dot++;
				}
			} else if (*iter == '.') {
				if (iter == begin) {
					return std::nullopt;
				}

				after_dot = true;
				iter++;
			} else if (*iter == '-') {
				if (iter != begin) {
					return std::nullopt;
				}

				iter++;
			} else {
				break;
			}
		}

		if (chars_after_dot != 3) {
			return std::nullopt;
		}

		return std::stof(std::string(begin, iter));
	}
}