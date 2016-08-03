#include <http2_asio/core.hpp>

namespace h2a {

core::core()
{}

core&
core::get()
{
    if (_inst.load() == nullptr) {
        _inst = new core();
    }
    return *_inst;
}

}   // namespace h2a
