#define BOOST_TEST_MODULE test_tcp_server
#include <boost/test/unit_test.hpp>

#include <http2-asio/unit_test.hpp>
#include <http2-asio/system_config.hpp>

#include <http2_asio/server.hpp>

class dummy_server {
public:
    struct handler_type {
        std::string toto;  
        
        template<size_t N>
        inline bool on_read(
            const std::array<uint8_t, N>& buffer,
            size_t length
        )
        { return true; }
        
        template<size_t N>
        inline bool on_write(
            std::array<uint8_t, N>& buffer, 
            size_t& length
        )
        { return true; }
        
        inline bool should_stop() const
        { return false; }
    };
};

struct tcp_server_fixture {
    dummy_server dummy;
};

BOOST_FIXTURE_TEST_SUITE(tcp_server_test, tcp_server_fixture);

BOOST_AUTO_TEST_CASE(instanciate_tcp_server)
{
    h2a::io_service_pool  pool(5);
    h2a::server  srv(
        pool, 
        boost::posix_time::millisec(120),
        boost::posix_time::millisec(120)
    );
    
    srv.listen(dummy, "0.0.0.0", "12345", true);
}

BOOST_AUTO_TEST_SUITE_END();