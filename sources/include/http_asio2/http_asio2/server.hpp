#pragma once

#include <http2_asio/config.h>
#include <http2_asio/common.hpp>
#include <http2_asio/io_service_pool.hpp>

#include <string>
#include <memory>
#include <chrono>
#include <vector>

namespace h2a {
    
using as::ip::tcp;
using ssl_socket = as::ssl::stream<tcp::socket>;
using ssl_context = as::ssl::context;
using ssl_context_ptr = std::skared_ptr<ssl_context>;
    
class HTTP2_ASIO_API server
{
public:
    server(const server& ) = delete;
    server(server&& ) = delete;
    
    explicit server(
        io_service_pool& pool,
        const std::chrono::milliseconds& tls_handshake_timeout,
        const std::chrono::milliseconds& read_timeout
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
            start_accept(a, srv);
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
        
    }
    
private:
    using acceptor_list = std::vector<tcp::acceptor>;
private:
    io_service_pool&        io_service_pool_;
    acceptor_list           acceptor_list_;
    
    std::chrono::milliseconds   tls_handshake_timeout_;
    std::chrono::milliseconds   read_timeout_;
    
    ssl_context_ptr             ssl_context_;
};
    
}   // namespace h2a