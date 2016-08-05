#pragma once

#include <http2-asio/config.h>

#include <memory>
#include <experimental/string_view>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace std {
    using string_view = experimental::string_view;
}   // namespace std

namespace h2a {

const auto NGHTTP2_H2_ALPN          = std::string_view("\x2h2");
const auto NGHTTP2_H2               = std::string_view("h2");
const auto NGHTTP2_H2_16_ALPN       = std::string_view("\x5h2-16");
const auto NGHTTP2_H2_16            = std::string_view("h2-16");
const auto NGHTTP2_H2_14_ALPN       = std::string_view("\x5h2-14");
const auto NGHTTP2_H2_14            = std::string_view("h2-14");
const auto NGHTTP2_H1_1_ALPN        = std::string_view("\x8http/1.1");
const auto NGHTTP2_H1_1             = std::string_view("http/1.1");

namespace as = boost::asio;
using boost::asio::ip::tcp;
using ssl_socket = boost::asio::ssl::stream<tcp::socket>;
using ssl_context = as::ssl::context;
using ssl_context_ptr = std::shared_ptr<ssl_context>;
    
HTTP2_ASIO_API bool tls_h2_negotiated(ssl_socket& socket);
HTTP2_ASIO_API bool check_h2_is_selected(const std::string_view& proto);
    
}   // namespace h2a