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
    {
        int rv = nghttp2_session_mem_recv(session_, buffer.data(), length);
        return (rv >= 0);
    }
       
    template<size_t N>
    inline bool on_write(
        std::array<uint8_t, N>& buffer, 
        size_t& length
    )
    {
        length = 0;
        
        if (output_data_) {
            std::copy_n(output_data_, output_size_, std::begin(buffer));
            length += output_size_;
            
            output_data_ = nullptr;
            output_size_ = 0;
        }
        
        for  (;;) {
            const uint8_t* data = nullptr;
            auto nread = nghttp2_session_mem_send(session_, &data);
            if (nread < 0) {
                return false;
            }
            
            if (nread == 0) {
                break;
            }
            
            if (length + nread > buffer.size()) {
                output_data_ = data;
                output_size_ = nread;
                break;
            }
            
            std::copy_n(data, nread, std::begin(buffer) + length);
            length += nread;
        }
        return true;
    }
        
    inline bool should_stop() const
    { return false; }

    inline as::io_service&     get_io_service()
    { return io_service_; }
    
private:
    as::io_service&             io_service_;
    as::ip::tcp::endpoint       ep_;
    
    nghttp2_session*            session_ = nullptr;
    
    const uint8_t*              output_data_ = nullptr;
    size_t                      output_size_ = 0;
};
    
}   // namespace h2a