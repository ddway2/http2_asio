#include <http2_asio/server.hpp>

namespace h2a {

server::server(
    io_service_pool& pool,
    const std::chrono::milliseconds& tls_handshake_timeout,
    const std::chrono::milliseconds& read_timeout
)
: io_service_pool_(pool),
  tls_handshake_timeout_(tls_handshake_timeout),
  read_timeout_(read_timeout)
{}

    
/// stop server
void 
server::stop()
{
    io_service_pool_.stop();
    for (auto& a : acceptor_list_) {
        acceptor_list_.close();
    }
}
    
/// join pool thread
void 
server::join()
{ io_service_pool_.join(); }

void 
server::bind(const std::string& address, const std::string& port)
{
    tcp::resolver resolv(io_service_pool_.get_io_service());
    tcp::resolver::query q(address, port);
    
    boost::system::error_code ec;
    auto it = resolv.resolve(q, ec);
    if (ec) {
        std::ostringstream oss;
        oss << "h2a::server::bind - Error during tcp::resolver: " << ec;
        throw std::runtime_error(oss.str());
    }
    
    for ( ; it != tcp::resolver::iterator() ; ++it) {
        tcp::endpoint endpoint = *it;
        auto acceptor = tcp::acceptor(io_service_pool_.get_io_service());
        if (acceptor.open(endpoint.protocol(), ec)) {
            continue;
        }
        
        acceptor.set_option(tcp::acceptor::reuse_address(true));
        
        if (acceptor.bind(endpoint, ec)) {
            continue;
        }
        
        if (acceptor.listen(boost::asio::socket_base::max_connections), ec) {
            continue;
        }
        
        acceptor_list_.push_back(acceptor);
    }
    
    if (acceptor_list_.empty() {
        std::ostringstream oss;
        oss << "h2a::server::bind - Error during binding: " << ec;
        throw std::runtime_error(oss.str());
    }
}

}   // namespace h2a
