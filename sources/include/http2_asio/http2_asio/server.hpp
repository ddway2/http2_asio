#pragma once

#include <http2-asio/config.h>

#include <http2_asio/common.hpp>
#include <http2_asio/io_service_pool.hpp>
#include <http2_asio/connection.hpp>

#include <string>
#include <memory>
#include <chrono>
#include <vector>

namespace h2a {
    
class HTTP2_ASIO_API server
{
public:
    server(const server& ) = delete;
    server(server&& ) = delete;
    
    explicit server(
        io_service_pool& pool,
        const boost::posix_time::time_duration& tls_handshake_timeout,
        const boost::posix_time::time_duration& read_timeout
    );
    
    /// Set TLS context if necessary
    inline void set_ssl_context(ssl_context_ptr ctx)
    { ssl_context_ = ctx; }
    
    /// Listen incoming connection
    template<typename ServerImpl>
    void listen(
        ServerImpl& srv,
        const std::string& address,
        const std::string& port,
        bool blocking = false
    )
    {
        bind(address, port);
    
        for (auto& a : acceptor_list_) {
            if (ssl_context_) { 
                start_accept_tls(a, srv, ssl_context_);
            } else {
                start_accept(a, srv);
            }
        }
        
        io_service_pool_.run(blocking);
    }
    
    /// stop server
    void stop();
    
    /// join pool thread
    void join();
    
private:
    void bind(const std::string& address, const std::string& port);
    
    template<typename ServerImpl>
    void start_accept(tcp::acceptor& a, ServerImpl& srv)
    {
        auto c = std::make_shared<connection<ServerImpl, tcp::socket>>(
            srv,
            tls_handshake_timeout_,
            read_timeout_,
            io_service_pool_.get_io_service()
        );
        
        a.async_accept(
            c->socket(),
            [this, &a,&srv,c](const boost::system::error_code& e) {
                if (!e) {
                    c->socket().set_option(tcp::no_delay(true));
                    c->start_read_timeout();
                    c->start();
                }
                start_accept(a, srv);
            }
        );
    }
    
    template<typename ServerImpl>
    void start_accept_tls(tcp::acceptor& a, ServerImpl& srv, ssl_context_ptr ctx)
    {
        auto c = std::make_shared<connection<ServerImpl, ssl_socket>>(
            srv,
            tls_handshake_timeout_,
            read_timeout_,
            io_service_pool_.get_io_service(),
            *ctx
        );
        
        a.async_accept(
            c->socket().lowest_layer(),
            [this,&a,&srv,ctx,c](const boost::system::error_code& e) {
                if (!e) {
                    c->socket().lowest_layer().set_option(tcp::no_delay(true));
                    c->start_handshake_timeout();
                    c->socket().async_handshake(
                        as::ssl::stream_base::server,
                        [c](const boost::system::error_code& e) {
                            if (e) {
                                c->stop();
                                return;
                            }
                            
                            if (!tls_h2_negotiated(c->socket())) {
                                c->stop();
                                return;
                            }
                            
                            c->start();
                        }
                    );
                }
                start_accept_tls(a, srv, ctx);
            }
        );
    }
    
private:
    using acceptor_list = std::vector<tcp::acceptor>;
private:
    io_service_pool&        io_service_pool_;
    acceptor_list           acceptor_list_;
    
    boost::posix_time::time_duration   tls_handshake_timeout_;
    boost::posix_time::time_duration   read_timeout_;
    
    ssl_context_ptr             ssl_context_;
};
    
}   // namespace h2a