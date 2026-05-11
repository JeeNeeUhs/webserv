#ifndef CGI_HPP
# define CGI_HPP

# include "HTTPRequest.hpp"
# include "Location.hpp"
# include <string>

std::string cgiRun(const Location& data, HTTPRequest& request);

#endif
