#pragma once

#include <http2-asio/config.h>

#include <memory>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace h2a {
    
namespace as = boost::asio;
using boost::asio::ip::tcp;
using ssl_socket = boost::asio::ssl::stream<tcp::socket>;
using ssl_context = as::ssl::context;
using ssl_context_ptr = std::shared_ptr<ssl_context>;
    
HTTP2_ASIO_API bool tls_h2_negotiated(ssl_socket& socket);
    
}   // namespace h2a