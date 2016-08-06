#pragma once

#include <http2-asio/config.h>
#include <http2_asio/common.hpp>

#include <nghttp2/nghttp2.h>

#include <memory>
#include <array>

namespace h2a {
    
class HTTP2_ASIO_API http2_session
: std::enable_shared_from_this<http2_session>
{
public:
    http2_session(
        as::io_service& io,
        as::ip::tcp::endpoint ep
    )
    : io_service_(io),
      ep_(ep)
    {}
    
    ~http2_session()
    {}
    
    bool start();
    
    template<size_t N>
    inline bool on_read(
        const std::array<uint8_t, N>& buffer,
        size_t length
    )
    { return true; }
       
    template<size_t N>
    inline bool on_write(
        std::array<uint8_t, N>& buffer, 
        size_t& length
    )
    { return true; }
        
    inline bool should_stop() const
    { return false; }

    inline as::io_service&     get_io_service()
    { return io_service_; }
    
private:
    as::io_service&             io_service_;
    as::ip::tcp::endpoint       ep_;
    
    nghttp2_session*            session_ = nullptr;
};
    
}   // namespace h2a