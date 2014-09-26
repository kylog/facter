#include <facter/util/environment.hpp>
#include <boost/algorithm/string.hpp>
#include <facter/logging/logging.hpp>
#include <facter/util/windows/scoped_error.hpp>
#include <cstdlib>
#include <windows.h>

using namespace std;

LOG_DECLARE_NAMESPACE("util.windows.environment")

namespace facter { namespace util {

    struct search_path_helper
    {
        search_path_helper()
        {
            string paths;
            if (environment::get("PATH", paths)) {
                boost::split(_paths, paths, bind(equal_to<char>(), placeholders::_1, environment::get_path_separator()), boost::token_compress_on);
            }
        }

        vector<string> const& search_paths() const
        {
            return _paths;
        }

     private:
         vector<string> _paths;
    };

    bool environment::get(string const& name, string& value)
    {
        // getenv on Windows won't get vars set by SetEnvironmentVariable in the same process.
        vector<char> buf(256);
        auto numChars = GetEnvironmentVariable(name.c_str(), buf.data(), buf.size());
        if (numChars > buf.size()) {
            buf.resize(numChars);
            numChars = GetEnvironmentVariable(name.c_str(), buf.data(), buf.size());
        }

        if (numChars == 0) {
            auto err = GetLastError();
            if (err != ERROR_ENVVAR_NOT_FOUND) {
                LOG_DEBUG("failure reading environment variable %1%: %2% (%3%)", name, scoped_error(err), err);
            }
            return false;
        }

        value.assign(buf.data(), numChars);
        return true;
    }

    bool environment::set(string const& name, string const& value)
    {
        return SetEnvironmentVariable(name.c_str(), value.c_str()) != 0;
    }

    bool environment::clear(string const& name)
    {
        return SetEnvironmentVariable(name.c_str(), nullptr) != 0;
    }

    char environment::get_path_separator()
    {
        return ';';
    }

    vector<string> const& environment::search_paths()
    {
        static search_path_helper helper;
        return helper.search_paths();
    }

}}  // namespace facter::util
