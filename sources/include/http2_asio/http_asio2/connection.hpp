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
    
    using handler_type = Server::handler_type;
    using handler_ptr = std::shared_ptr<handler_type>;
    
public:
    /// CTOR 
    template<typename ...Args>
    explicit tcp_connection(
        server_type& s,
        const boost::posix_time::time_duration& tls_handshake_timeout,
        const boost::posix_time::time_duration& read_timeout,
        Args&& ...args
    )
    : server_(s),
      socket_(std::forward<Args>(args)...),
      deadline_(socket_.get_io_service()),
      tls_handshake_timeout_(tls_handshake_timeout),
      read_timeout_(read_timeout)
    {}
    
    /// start async connection
    void start()
    {
        handler_ = std::make_shared<handler_type>();
    }
    
    /// Stop async connection
    void stop()
    {
        if (stopped_) {
            return;
        }
        
        stopped_ = true;
        boost::system::error_code ec;
        socket_.lowest_lasyer().close(ec);
        deadline_.cancel();
    }
    
    /// Start handshake timeout for SSL connection
    void start_handshake_timeout()
    {
        deadline_.expires_from_now(tls_handshake_timeout_);
        deadline_.async_wait(
            std::bind(&connection::timeout_handler, shared_from_this())
        );
    }
    
    /// Start read timeout for TCP connection
    void start_read_timeout()
    {
        deadline_.expires_from_now(read_timeout_);
        deadline_.async_wait(
            std::bind(&connection::timeout_handler, shared_from_this())
        );
    }
    
    /// Get socket
    inline socket_type& socket()
    { return socket_; }
    
private:
    void timeout_handler()
    {
        if (stopped_) {
            return;
        }
        
        if (deadline_.expires_at() <= as::deadline_timer::traits_type::now()) {
            stop();
            deadline_.expires_at(boost::posix_time::pos_infin);
            return;
        }
        
        deadline_.async_wait(
            std::bind(&connection::timeout_handler, shared_from_this())
        );
    }
    
private:
    server_type&        server_;
    socket_type         socket_;
    
    handler_ptr         handler_;
    
    as::deadline_timer          deadline_;
    boost::posix_time::time_duration   tls_handshake_timeout_;
    boost::posix_time::time_duration   read_timeout_;
    
    bool        stopped_ = false;
};


}   // namespace h2a