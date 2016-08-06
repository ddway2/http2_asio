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

const size_t MAX_CONCURRENT_STREAMS = 100;

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
    
///
/// defer function in go style (from <http://blog.korfuri.fr/post/go-defer-in-cpp/>)
///
template<
    typename Func,
    typename ... Args
>
struct defer_handle
{
    // Default constructor
    defer_handle(Func& f, Args&& ...args)
    : callback(std::bind(std::forward<Func>(f), std::forward<Args>(args)...))
    {}
    
    defer_handle(defer_handle&& d)
    : callback(std::move(d.callback))
    {}
    
    ~defer_handle()
    { callback(); }
    
    using ResultType = typename std::result_of<
        typename std::decay<Func>::type(
            typename std::decay<Args>::type...
        )
    >::type;
        
    std::function<ResultType()>   callback;
};
  

template<
    typename Func,
    typename ...Args
>
inline defer_handle<Func, Args...> defer(Func&& f, Args&& ...args)
{
    return defer_handle<Func, Args...>(
            std::forward<Func>(f), 
            std::forward<Args>(args)...
        );
}
  
}   // namespace h2a