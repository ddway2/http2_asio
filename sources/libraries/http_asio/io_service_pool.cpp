#include <http2_asio/io_service_pool.hpp>

#include <iostream>

namespace h2a {

io_service_pool::io_service_pool(size_t pool_size)
{
    if (pool_size == 0) {
        std::ostringstream oss;
        oss << "h2a::io_service_pool::io_service_pool - pool size must be > 0";
        throw std::runtime_error(oss.str());
    }
    
    io_services_list_.reserve(pool_size);
    io_workers_list_.reserve(pool_size);
    
    for (size_t i = 0 ; i < pool_size ; ++i) {
        auto ios = std::make_shared<as::io_service>();
        auto work = std::make_shared<as::io_service::work>(*ios);
        io_services_list_.push_back(ios);
        io_workers_list_.push_back(work);
    }
}
 
void 
io_service_pool::run(bool blocking)
{
    for (size_t i = 0 ; i < io_services_list_.size() ; ++i) {
        io_service_pool_.push_back(
            std::async(
                std::launch::async,
                (size_t (as::io_service::*)(void))& as::io_service::run, io_services_[i] 
            );
        );
    }
    
    if (blocking) {
        join();
    }
}

void 
io_service_pool::stop()
{
    for (auto& ios : io_services_list_) {
        ios->stop();
    }
}

void 
io_service_pool::join()
{
    for (auto& p : io_service_pool_) {
        p.get();
    }
}

as::io_service& get_io_service()
{
    auto& ios = *io_services_list_[next_io_service_index_];
    next_io_service_index_ = (next_io_service_index_ + 1) % io_services_list_.size();
    return ios;
}

}   // namespace h2a