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
    
private:
    int32_t         stream_id_;
};
    
using http2_stream_ptr = std::unique<http2_stream>;
    
}   // namespace h2a