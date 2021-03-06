#include <http2_asio/http2_session.hpp>

// nghttp2_session_callbacks_set_on_begin_headers_callback
// nghttp2_session_callbacks_set_on_header_callback
// nghttp2_session_callbacks_set_on_frame_recv_callback
// nghttp2_session_callbacks_set_on_data_chunk_recv_callback
// nghttp2_session_callbacks_set_on_stream_close_callback
// nghttp2_session_callbacks_set_on_frame_send_callback
// nghttp2_session_callbacks_set_on_frame_not_send_callback

namespace h2a {

namespace {
    
// Stream error_code
int
stream_error_callback(
    nghttp2_session* session,
    int32_t stream_id,
    uint32_t error_code
)
{
    return nghttp2_submit_rst_stream(
        session,
        NGHTTP2_FLAG_NONE,
        stream_id,
        error_code
    );
}

// Header Begins
int 
on_begin_headers_callback(
    nghttp2_session* session,
    const nghttp2_frame* frame, 
    void* user_data
)
{
    auto self = static_cast<http2_session*>(user_data);
    
    if (
        frame->hd.type != NGHTTP2_HEADERS
        ||
        frame->headers.cat != NGHTTP2_HCAT_REQUEST
    ) {
        return 0;
    }
    
    self->make_stream(frame->hd.stream_id);
    
    return 0;
}

// Headers
int 
on_header_callback(
    nghttp2_session* session,
    const nghttp2_frame* frame,
    
    const uint8_t* name, size_t name_size,
    const uint8_t* value, size_t value_size,
    uint8_t flags,
    void* user_data
)
{
    auto self = static_cast<http2_session*>(user_data);
    auto stream_id = frame->hd.stream_id;
    
    if (
        frame->hd.type != NGHTTP2_HEADERS
        ||
        frame->headers.cat != NGHTTP2_HCAT_REQUEST
    ) {
        return 0;
    }
    
    auto stream = self->find_stream(stream_id);
    if (!stream) {
        return 0;
    }
    
    // TODO: Prepare header request
    // switch (nghttp2::http2::lookup_token(name, namelen)) {
    // case nghttp2::http2::HD__METHOD:
    
    //     break;
    // default:
    //     break;
    // }

    return 0;
}

// Frame Recv
int 
on_frame_recv_callback(
    nghttp2_session* session,
    const nghttp2_frame* frame,
    void* user_data
)
{
    auto self = static_cast<http2_session*>(user_data); 
    auto stream = self->find_stream(frame->hd.stream_id);
    
    if (!stream) {
        return 0;
    }
    
    switch(frame->hd.type) {
    case NGHTTP2_DATA:
        if (frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
            stream->end_of_stream();
        }
    break;
    case NGHTTP2_HEADERS: 
        if (frame->headers.cat == NGHTTP2_HCAT_REQUEST) {
            // Call request
            
            if (frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
                stream->end_of_stream();
            }
        }
    break;
    }
    
    return 0;
}

// Data Chunk Recv
int
on_data_chunk_recv_callback(
    nghttp2_session* session,
    uint8_t flags,
    int32_t stream_id,
    const uint8_t* data, size_t size,
    void* user_data
    
)
{
    auto self = static_cast<http2_session*>(user_data); 
    auto stream = self->find_stream(stream_id);
    
    if (!stream) {
        return 0;
    }
    
    stream->call_on_data(data, size);
    
    return 0;
}

// Stream closed
int
on_stream_close_callback(
    nghttp2_session* session,
    int32_t stream_id,
    uint32_t error_code,
    void* user_data
)
{
    auto self = static_cast<http2_session*>(user_data);
    auto stream = self->find_stream(stream_id);
    
    if (!stream) {
        return 0;
    }
    
    stream->call_on_close(error_code);
    self->destroy_stream(stream_id);
    
    return 0;
}

// Frame sent
int
on_frame_send_callback(
    nghttp2_session* session,
    const nghttp2_frame* frame,
    void* user_data
)
{
    if (frame->hd.type != NGHTTP2_PUSH_PROMISE) {
        return 0;
    }
    
    auto self = static_cast<http2_session*>(user_data);
    auto stream = self->find_stream(frame->push_promise.promised_stream_id);
    
    if (!stream) {
        return 0;
    }
    stream->push_promise_sent();
    
    return 0;
}

// Frame not sent
int
on_frame_not_send_callback(
    nghttp2_session* session,
    const nghttp2_frame* frame,
    int error_code,
    void* user_data
)
{
    if (frame->hd.type != NGHTTP2_HEADERS) {
        return 0;
    }
    
    nghttp2_submit_rst_stream(
        session, 
        NGHTTP2_FLAG_NONE, 
        frame->hd.stream_id,
        NGHTTP2_INTERNAL_ERROR
    );
    
    return 0;
}

}   // namespace

bool
http2_session::start()
{
    nghttp2_session_callbacks* calls = nullptr;
    
    int rv = nghttp2_session_callbacks_new(&calls);
    if (rv != 0) {
        return false;
    }
    
    // Remove calls struct
    auto delcb = defer(nghttp2_session_callbacks_del, calls);
    
    // Prepare callbacks
    nghttp2_session_callbacks_set_on_begin_headers_callback(
        calls, on_begin_headers_callback
    );
    nghttp2_session_callbacks_set_on_header_callback(
        calls, on_header_callback
    );
    nghttp2_session_callbacks_set_on_frame_recv_callback(
        calls, on_frame_recv_callback
    );
    nghttp2_session_callbacks_set_on_data_chunk_recv_callback(
        calls, on_data_chunk_recv_callback
    );
    nghttp2_session_callbacks_set_on_stream_close_callback(
        calls, on_stream_close_callback
    );
    nghttp2_session_callbacks_set_on_frame_send_callback(
        calls, on_frame_send_callback
    );
    nghttp2_session_callbacks_set_on_frame_not_send_callback(
        calls, on_frame_not_send_callback
    );
    
    // Create http2 session server
    rv = nghttp2_session_server_new(
        &session_,
        calls,
        this
    );
    if (rv != 0) {
        return false;
    }
    
    // Set default setting for session connection
    nghttp2_settings_entry entry{
        NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS, MAX_CONCURRENT_STREAMS
    };
    nghttp2_submit_settings(session_, NGHTTP2_FLAG_NONE, &entry, 1);
    
    return true;
}

bool
http2_session::should_stop() const
{
    return !nghttp2_session_want_read(session_) &&
           ! nghttp2_session_want_write(session_);
}
    
}   // namespace h2a