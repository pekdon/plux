#include "output_format.hh"

#include <cstdint>
#include <iostream>

namespace plux
{

enum in_var_state {
	IN_VAR_NO,
	IN_VAR_NAME,
	IN_VAR_TYPE
};

/**
 * Format content of the provided string and arguments using the current run
 * state and store the result in formatted.
 *
 * @ return true on success, else false.
 */
bool
OutputFormat::format(std::string &formatted)
{
	std::string name, type;
	in_var_state state = IN_VAR_NO;
	for (size_t i = 0; i < _str.size(); i++) {
		if (state == IN_VAR_TYPE) {
			if (_str[i] == '[') {
				state = IN_VAR_NAME;
			} else {
				type += _str[i];
			}
		} else if (state == IN_VAR_NAME) {
			if (_str[i] == ']') {
				if (! expand(formatted, type, name)) {
					return false;
				}
				state = IN_VAR_NO;
				type = "";
				name = "";
			} else {
				name += _str[i];
			}
		} else if (_str[i] == '%' && _str[i + 1] == '%') {
			// verbatim %
			i++;
			formatted += '%';
		} else if (_str[i] == '%') {
			// parse until (, type of format
			state = IN_VAR_TYPE;
		} else {
			formatted += _str[i];
		}
	}

	if (state != IN_VAR_NO) {
		_error = "incomplete format";
		return false;
	}

	return true;
}

bool
OutputFormat::expand(std::string& formatted, const std::string& type,
		     const std::string& name)
{
	if (type.empty()) {
		_error = "format type is empty";
		return false;
	} else if (name.empty()) {
		_error = "format name/index is empty for " + type;
		return false;
	}

	std::string var;
	if (! eval_var(name, var)) {
		return false;
	}

	if (type == "i8") {
		return to_int_bytes<int8_t>(formatted, var);
	} else if (type == "i16") {
		return to_int_bytes<int16_t>(formatted, var);
	} else if (type == "i32") {
		return to_int_bytes<int32_t>(formatted, var);
	} else if (type == "i64") {
		return to_int_bytes<int64_t>(formatted, var);
	} else if (type == "u8") {
		return to_uint_bytes<uint8_t>(formatted, var);
	} else if (type == "u16") {
		return to_uint_bytes<uint16_t>(formatted, var);
	} else if (type == "u32") {
		return to_uint_bytes<uint32_t>(formatted, var);
	} else if (type == "u64") {
		return to_uint_bytes<uint64_t>(formatted, var);
    } else if (type == "b") {
        if (var == "" || var == "0" || var == "false") {
            formatted += "false";
        } else {
            formatted += "true";
        }
        return true;
	} else if (type == "s") {
		formatted += var;
		return true;
	} else {
		_error = "unsupported type " + type;
		return false;
	}
}

bool
OutputFormat::eval_var(const std::string& name, std::string& var)
{
	size_t pos = name.find('(');
	if (pos == std::string::npos) {
		return get_arg(name, var);
	} else if (name[name.size() - 1] != ')') {
		_error = "invalid function " + name + ", missing end )";
		return false;
	}

	std::string fun = name.substr(0, pos);
	std::string arg = name.substr(pos + 1, name.size() - pos - 2);
	if (fun == "len") {
		std::string val;
		if (! get_arg(arg, val)) {
			return false;
		}
		var = std::to_string(val.size());
		return true;
	} else {
		_error = "unknown function " + name;
		return false;
	}
}

bool
OutputFormat::get_arg(const std::string &idx_str, std::string& arg)
{
	try {
		size_t idx = std::stoi(idx_str);
		return get_arg(idx, arg);
	} catch (std::invalid_argument&) {
		_error = arg + " is not a valid argument index";
		return false;
	}
}

bool
OutputFormat::get_arg(size_t idx, std::string& arg)
{
	if (idx >= _args.size()) {
		_error = "argument " + std::to_string(idx) + " missing";
		return false;
	}
	arg = _args[idx];
	return true;
}

template<typename T>
bool
OutputFormat::to_int_bytes(std::string &formatted, const std::string &str)
{
	try {
		T val = static_cast<T>(std::stoll(str));
		formatted += std::string(reinterpret_cast<char*>(&val),
					 sizeof(val));
		return true;
	} catch (std::invalid_argument&) {
		_error = "invalid integer: " + str;
		return false;
	}
}

template<typename T>
bool
OutputFormat::to_uint_bytes(std::string &formatted, const std::string &str)
{
	try {
		T val = static_cast<T>(std::stoull(str));
		formatted += std::string(reinterpret_cast<char*>(&val),
					 sizeof(val));
		return true;
	} catch (std::invalid_argument&) {
		_error = "invalid unsigned integer: " + str;
		return false;
	}
}

}
