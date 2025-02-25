#pragma once

#include <string>
#include <vector>

namespace plux
{
	/**
	 * Output formatter, supporting simple input processing such as string
	 * length.
	 */
	class OutputFormat {
	public:
		typedef std::vector<std::string> string_vector;

		OutputFormat(const std::string& str,
			     const string_vector &args)
			: _str(str),
			  _args(args)
		{
		}

		bool format(std::string& formatted);
		const std::string& error() const { return _error; }

	private:
		bool expand(std::string& formatted, const std::string& type,
			    const std::string& name);
		bool eval_var(const std::string& name, std::string& var);
		bool get_arg(const std::string &idx_str, std::string& arg);
		bool get_arg(size_t idx, std::string& arg);
		template<typename T>
		bool to_int_bytes(std::string &formatted,
				  const std::string &str);
		template<typename T>
		bool to_uint_bytes(std::string &formatted,
				   const std::string &str);

		std::string _error;
		std::string _str;
		string_vector _args;
	};
}
