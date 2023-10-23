#pragma once
#include <fstream>
#include <variant>
#include <vector>
#include <optional>
#include <functional>
#include <unordered_map>
#include <cstdint>

///
/// Example usage
/// 
/// milling_command_parser parser("paths/3.f10");
/// auto commands = parser.get_commands();
/// for (auto& command : commands) {
///		std::visit([](const auto& arg) {
///			using T = std::decay_t<decltype(arg)>;
///			if constexpr (std::is_same_v<T, command_invalid>) {
///				std::cout << "invalid command" << std::endl;
///			} else if constexpr (std::is_same_v<T, command_g01_t>) {
///				std::cout << "move frez " << arg.x << " " << arg.y << " " << arg.z << std::endl;
///			}
///		}, command);
///	}
///

namespace mini {
	struct command_g01_t {
		float x, y, z;
	};

	using command_invalid = std::monostate;
	using milling_command = std::variant<command_g01_t, command_invalid>;
	using command_parser = std::function<milling_command (const std::string&, std::string::const_iterator&)>;

	class milling_command_parser {
		private:
			std::unordered_map<uint64_t, command_parser> m_parsers;

			std::ifstream m_stream;
			std::size_t m_previous_line;

		public:
			milling_command_parser(const std::string& path);
			~milling_command_parser() = default;

			milling_command_parser(const milling_command_parser&) = delete;
			milling_command_parser& operator=(const milling_command_parser&) = delete;

			bool is_good() const;

			std::optional<milling_command> get_next_command();
			std::vector<milling_command> get_commands();

		private:
			milling_command m_read_g01_command(const std::string& line, std::string::const_iterator& iter) const;

			char m_peek(const std::string& line, std::string::const_iterator& iter) const;
			char m_get(const std::string& line, std::string::const_iterator& iter) const;

			std::optional<int> m_try_read_int(const std::string& line, std::string::const_iterator& iter) const;
			std::optional<float> m_try_read_float(const std::string & line, std::string::const_iterator & iter) const;
	};
}
