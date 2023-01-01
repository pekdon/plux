#pragma once

#include <string>
#include <vector>

#include "line.hh"

namespace plux
{
    /**
     * Function.
     */
    class Function {
    public:
        Function(const std::string& file,  unsigned int line,
                 const std::string& name, std::vector<std::string> args)
            : _file(file),
              _line(line),
              _name(name),
              _args(args)
        {
        }
        virtual ~Function(void) { }

        const std::string& file(void) const { return _file; }
        unsigned int line(void) const { return _line; }
        const std::string& name(void) const { return _name; }

        int num_args(void) const { return _args.size(); }
        std::vector<std::string>::const_iterator args_begin(void) const {
            return _args.begin();
        }
        std::vector<std::string>::const_iterator args_end(void) const {
            return _args.end();
        }

        line_it line_begin(void) const { return _lines.begin(); }
        line_it line_end(void) const { return _lines.end(); }
        void line_add(Line* line) { _lines.push_back(line); }

    private:
        /** file line was parsed in. */
        const std::string _file;
        /** file line number. */
        unsigned int _line;
        /** function name. */
        std::string _name;
        /** function argument names. */
        std::vector<std::string> _args;

        /** function content, individual script lines. */
        line_vector _lines;
    };

    typedef std::map<std::string, Function*> fun_map;
    typedef fun_map::const_iterator fun_it;

}
