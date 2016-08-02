#pragma once

#include <http2_asio/config.h>

#include <atomic>
#include <memory>
#include <future>
#include <thread>

#include <boost/asio.hpp>

namespace h2a {

using namespace as = boost::asio;

class HTTP2_ASIO_API io_service_pool
{
public:
    io_service_pool(const io_service_pool& ) = delete;
    io_service_pool& operator=(const io_service_pool& ) = delete;
    
    /// Initialize pool thread
    explicit io_service_pool(size_t pool_size);
    
    /// Run service
    void run(bool blocking = false);
    
    /// Stop service
    void stop();
    
    /// Join all pool thread
    void join();
    
    /// Get free io service
    as::io_service& get_io_service();
    
private:
    using io_service_ptr = std::shared_ptr<as::io_service>;
    using io_worker_ptr = std::shared_ptr<as::io_service::work>;
    using io_service_future = std::future<size_t>;
    
private:
    std::vector<io_service_ptr>         io_services_list_;
    std::vector<io_worker_ptr>          io_workers_list_;
    
    std::vector<io_service_future>      io_service_pool_;
    size_t                              next_io_service_index_ = 0;
    
};

    
}   // namespace h2a