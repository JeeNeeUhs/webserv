#ifndef CGI_HPP
# define CGI_HPP

# include "HTTPRequest.hpp"
# include "Location.hpp"
# include <string>

void cgiRun(const Location& data, const HTTPRequest& request, HTTPResponse& response);

#endif
