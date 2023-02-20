#pragma once

#include "line.hh"

namespace plux {
    /**
     * Any header in the script, between documentation tag and
     * shell/script lines.
     */
    class Header : public Line {
    public:
        using Line::Line;
    };

    /**
     * [config *]
     */
    class HeaderConfig : public Header {
    public:
        HeaderConfig(const std::string& file, unsigned int line,
                     const std::string& key, const std::string& val)
            : Header(file, line, ""),
              _key(key),
              _val(val)
        {
        }
        virtual ~HeaderConfig(void) { }

        const std::string& key(void) const { return _key; }
        const std::string& val(void) const { return _val; }
        void set_val(const std::string& val) { _val = val; }

    protected:
        std::string _key;
        std::string _val;
    };

    /**
     * [config require=VAR=value]
     */
    class HeaderConfigRequire : public HeaderConfig {
    public:
        using HeaderConfig::HeaderConfig;
        virtual ~HeaderConfigRequire() { }

        virtual LineRes run(ShellCtx& ctx, ShellEnv& env) override;
        virtual std::string to_string(void) const override;
    };

    /**
     * [config set=VAR=value]
     */
    class HeaderConfigSet : public HeaderConfig {
    public:
        using HeaderConfig::HeaderConfig;
        virtual ~HeaderConfigSet() { }

        virtual LineRes run(ShellCtx& ctx, ShellEnv& env) override;
        virtual std::string to_string(void) const override;
    };

    /**
     * [include path]
     */
    class HeaderInclude : public Header {
    public:
        HeaderInclude(const std::string& file, unsigned int line,
                      const std::string& include_file)
            : Header(file, line, ""),
              _include_file(include_file)
        {
        }
        virtual ~HeaderInclude(void) { }

        virtual LineRes run(ShellCtx& ctx, ShellEnv& env) override;
        virtual std::string to_string(void) const override;
    private:
        std::string _include_file;
    };
}
