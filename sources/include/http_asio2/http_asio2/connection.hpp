#pragma once

#include <http2_asio/config.h>
#include <http2_asio/common.hpp>

#include <memory>

namespace h2a {

template<
    typename Server,
    typename Socket
>
class tcp_connection
: public std::shared_from_this<self_type>
{
public:
    using self_type = connection<Server,Socket>;
    using socket_type = Socket;
    using server_type = Server;
    
public:
    /// CTOR 
    template<typename ...Args>
    explicit tcp_connection(
        server_type& s,
        const std::chrono::milliseconds& tls_handshake,
        const std::chrono::milliseconds& read_timeout,
        Args&& ...args
    )
    : server_(s),
      socket_(std::forward<Args>(args)...)
    {}
    
    /// start async connection
    void start()
    {}
    
    /// Stop async connection
    void stop()
    {}
    
    /// Get socket
    inline socket_type& socket()
    { return socket_; }
    
private:
    server_type&        server_;
    socket_type         socket_;
};


}   // namespace h2a