#pragma once

#include <http2-asio/config.h>
#include <http2_asio/common.hpp>

#include <nghttp2/nghttp2.h>

#include <memory>
#include <array>
#include <functional>

namespace h2a {
    
class HTTP2_ASIO_API http2_stream
{
public:
    http2_stream(int32_t stream_id)
    : stream_id_(stream_id)
    {}
    
    
    void call_on_data(const uint8_t* data, size_t size)
    {}
    
    void end_of_stream()
    {}
    
    void call_on_close(uint32_t error_code)
    {}
    
    void push_promise_sent()
    {}
    
private:
    int32_t         stream_id_;
};
    
using http2_stream_ptr = std::unique_ptr<http2_stream>;
    
}   // namespace h2a