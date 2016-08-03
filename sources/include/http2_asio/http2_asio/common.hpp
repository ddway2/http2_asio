#pragma once

#include <boost/asio.hpp>

namespace h2a {
    
namespace as = boost::asio;
using ssl_socket = as::ssl::stream<tcp::socket>;
    
HTTP2_ASIO_API bool tls_h2_negotiated(ssl_context& socket);
    
}   // namespace h2a