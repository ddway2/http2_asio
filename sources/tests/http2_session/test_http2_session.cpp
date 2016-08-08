#define BOOST_TEST_MODULE test_http2_session
#include <boost/test/unit_test.hpp>

#include <http2-asio/unit_test.hpp>
#include <http2-asio/system_config.hpp>

#include <http2_asio/common.hpp>
#include <http2_asio/htt2_session.hpp>

struct http2_session_fixture {
    
};

BOOST_FIXTURE_TEST_SUITE(htt2_session_test, http2_session_fixture);

BOOST_AUTO_TEST_CASE(instantiate_http2_session)
{
    
}

BOOST_AUTO_TEST_SUITE_END();