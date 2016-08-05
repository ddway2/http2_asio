#include <http2-asio/config.h>

#include <http2_asio/common.hpp>

#include <openssl/evp.h>

namespace h2a {
    
bool 
tls_h2_negotiated(ssl_socket& socket)
{
    auto ssl = socket.native_handle();
    
    const unsigned char* next_proto = nullptr;
    unsigned int next_proto_len = 0;
    
    SSL_get0_next_proto_negotiated(ssl, &next_proto, &next_proto_len);
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
    if (next_proto == nullptr) {
        SSL_get0_alpn_selected(ssl, &next_proto, &next_proto_len);
    }
#endif
    
    if (next_proto == nullptr) {
        return false;
    }
    
    return check_h2_is_selected(std::string_view{(const char*)next_proto, next_proto_len});
}

bool 
check_h2_is_selected(const std::string_view& proto)
{
    return (proto == NGHTTP2_H2)
           ||
           (proto == NGHTTP2_H2_16)
           ||
           (proto == NGHTTP2_H2_14);
}
    
}   // namespace h2a