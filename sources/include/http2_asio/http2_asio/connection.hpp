#pragma once

#include <http2-asio/config.h>
#include <http2_asio/common.hpp>

#include <memory>
#include <array>

namespace h2a {

template<
    typename Server,
    typename Socket
>
class connection
: public std::enable_shared_from_this<connection<Server, Socket>>
{
public:
    using self_type = connection<Server,Socket>;
    using socket_type = Socket;
    using server_type = Server;
    
    using handler_type = typename Server::handler_type;
    using handler_ptr = std::shared_ptr<handler_type>;
    
public:
    /// CTOR 
    template<typename ...Args>
    explicit connection(
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
        boost::system::error_code ec;
        
        handler_ = std::make_shared<handler_type>(
            socket_.get_io_service(),
            socket_.lowest_layer().remote_endpoint(ec)    
        );
        
        // Error during http2 handler start
        if (!handler_->start()) {
            stop();
            return;
        }
        
        read_data();
    }
    
    /// Stop async connection
    void stop()
    {
        if (stopped_) {
            return;
        }
        
        stopped_ = true;
        boost::system::error_code ec;
        socket_.lowest_layer().close(ec);
        deadline_.cancel();
    }
    
    /// Start handshake timeout for SSL connection
    void start_handshake_timeout()
    {
        deadline_.expires_from_now(tls_handshake_timeout_);
        deadline_.async_wait(
            std::bind(&connection::timeout_handler, this->shared_from_this())
        );
    }
    
    /// Start read timeout for TCP connection
    void start_read_timeout()
    {
        deadline_.expires_from_now(read_timeout_);
        deadline_.async_wait(
            std::bind(&connection::timeout_handler, this->shared_from_this())
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
            std::bind(&connection::timeout_handler, this->shared_from_this())
        );
    }
    
    /// Read data from socket connection
    void read_data()
    {
        deadline_.expires_from_now(read_timeout_);
        
        auto self = this->shared_from_this();
        socket_.async_read_some(
            as::buffer(read_buffer_),
            [this,self](const boost::system::error_code& ec, size_t bytes_received){
                if (ec) {
                    stop();
                    return;
                }
                
                if(!handler_->on_read(read_buffer_, bytes_received)) {
                    stop();
                    return;
                }
        
                write_data();        
                
                if (!writing_ && handler_->should_stop()) {
                    stop();
                    return;
                }
                
                read_data();
            }
        );
    }
    
    /// Write data on socket
    void write_data()
    {
        if (writing_) {
            return;
        }
        
        size_t len = 0;
        if (!handler_->on_write(write_buffer_, len)) {
            stop();
            return;
        }
        
        if (len == 0) {
            if (handler_->should_stop()) {
                stop();
            }
            return;
        }
        
        // Change state of connection
        writing_ = true;
        deadline_.expires_from_now(read_timeout_);
        
        auto self = this->shared_from_this();
        socket_.async_write_some(
            as::buffer(write_buffer_, len),
            [this,self](const boost::system::error_code& ec, size_t bytes_sent) {
                if (ec) {
                    stop();
                    return;
                }
                
                // Ready to write
                writing_ = false;
                
                write_data();
            }
        );
    }
    
private:
    server_type&        server_;
    socket_type         socket_;
    
    handler_ptr         handler_;
    
    std::array<uint8_t, 8128>    read_buffer_;
    std::array<uint8_t, 65536>   write_buffer_;
    
    bool                writing_ = false;
    
    as::deadline_timer          deadline_;
    boost::posix_time::time_duration   tls_handshake_timeout_;
    boost::posix_time::time_duration   read_timeout_;
    
    bool        stopped_ = false;
};


}   // namespace h2a