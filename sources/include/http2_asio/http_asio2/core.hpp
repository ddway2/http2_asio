#pragma once

#include <http2_asio/config.h>

#include <atomic>
#include <memory>

#include <boost/asio.hpp>

#include <http2_asio/io_service_pool.hpp>

namespace h2a {

class HTTP2_ASIO_API core 
{
public:
    core(const core& ) = delete;
    core(core&& ) = delete;
    core& operator=(const core& ) = delete;
    core& operator=(core&& ) = delete;
    
    static core& get();
    
private:
    core();
 
private:
    
    
private:
    static std::atomic<core*>   _inst{nullptr};    
    
};
    
}   // namespace h2a
