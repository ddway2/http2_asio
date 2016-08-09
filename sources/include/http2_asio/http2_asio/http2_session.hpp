#pragma once

#include <http2-asio/config.h>
#include <http2_asio/common.hpp>

#include <nghttp2/nghttp2.h>

#include <memory>
#include <array>
#include <functional>

namespace h2a {
    
/// Forward Declaration
class http2_server;
    
class HTTP2_ASIO_API http2_session
: public std::enable_shared_from_this<http2_session>
{
public:
    http2_session(
        as::io_service& io,
        as::ip::tcp::endpoint ep,
        http2_server& s,
        write_signal_callback call
    )
    : io_service_(io),
      ep_(ep),
      server_(s),
      write_signal_(call)
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
        callback_guard gc(*this);
        
        int rv = nghttp2_session_mem_recv(session_, buffer.data(), length);
        return (rv >= 0);
    }
       
    template<size_t N>
    inline bool on_write(
        std::array<uint8_t, N>& buffer, 
        size_t& length
    )
    {
        callback_guard gc(*this);
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
    
    /// Stop http2 session    
    bool should_stop() const;

    inline as::io_service&     get_io_service()
    { return io_service_; }
    
private:

    inline void signal_write()
    {
        if (!callback_inside_) {
            write_signal_();
        }
    }
private:
    class callback_guard
    {
    public:
        callback_guard(http2_session& s)
        : session_(s)
        { session_.callback_inside_ = true; }
        
        ~callback_guard()
        { session_.callback_inside_ = false; }
        
    private:
        http2_session& session_;
    };
    
    friend class callback_guard;
    
private:
    as::io_service&             io_service_;
    as::ip::tcp::endpoint       ep_;
    
    http2_server&               server_;
    
    nghttp2_session*            session_ = nullptr;
    
    const uint8_t*              output_data_ = nullptr;
    size_t                      output_size_ = 0;
    
    bool                        callback_inside_ = false;
    write_signal_callback       write_signal_;
};
    
}   // namespace h2a