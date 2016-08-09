#define BOOST_TEST_MODULE test_http2_session
#include <boost/test/unit_test.hpp>

#include <http2-asio/unit_test.hpp>
#include <http2-asio/system_config.hpp>

#include <http2_asio/common.hpp>
#include <http2_asio/http2_session.hpp>
#include <http2_asio/http2_server.hpp>
#include <http2_asio/server.hpp>

#include <thread>

struct http2_session_fixture {
    
};

BOOST_FIXTURE_TEST_SUITE(htt2_session_test, http2_session_fixture);

BOOST_AUTO_TEST_CASE(instantiate_http2_session)
{
    h2a::io_service_pool  pool(5);
    h2a::server  srv(
        pool, 
        boost::posix_time::millisec(5000),
        boost::posix_time::millisec(5000)
    );
    
    h2a::http2_server dummy;
    
    std::thread server([&](){
        srv.listen(dummy, "0.0.0.0", "12345", true);
    });
    
    std::this_thread::sleep_for(std::chrono::seconds(3));
    srv.stop();
    
    srv.join();
}

BOOST_AUTO_TEST_SUITE_END();