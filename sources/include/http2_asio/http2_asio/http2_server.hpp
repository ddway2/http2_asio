#pragma once

#include <http2-asio/config.h>
#include <http2_asio/common.hpp>

#include <memory>
#include <array>

#include <http2_asio/http2_session.hpp>

namespace h2a {

class HTTP2_ASIO_API http2_server
{
public:
    using handler_type = http2_session;
    
public:
    http2_server() = default;
    http2_server(const http2_server& ) = delete;
    http2_server& operator= (const http2_server& ) = delete;
};

}   // namespace h2a