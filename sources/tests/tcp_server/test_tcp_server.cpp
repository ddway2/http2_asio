#define BOOST_TEST_MODULE test_tcp_server
#include <boost/test/unit_test.hpp>

#include <http2_asio/unit_test.hpp>
#include <http2_asio/system_config.hpp>

#include <http2_asio/server.hpp>

class dummy_server {
public:
    struct handler_type {
        std::string toto;  
    };
};

struct tcp_server_fixture {
    
};

BOOST_FIXTURE_TEST_SUITE(tcp_server_test, tcp_server_fixture);

BOOST_AUTO_TEST_CASE(instanciate_tcp_server)
{
    
}

BOOST_AUTO_TEST_SUITE_END();