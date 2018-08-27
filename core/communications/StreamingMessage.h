/// @ingroup communications
#pragma once

/// @addtogroup communications
///@{
#define MAX_STREAMING_MESSAGE_LEN 6'000'000

#include <boost/asio.hpp>
class StreamingMessage {
  using tcp = boost::asio::ip::tcp;
  public:
    boost::system::error_code sendMessage(tcp::socket &sock, const unsigned char *data, int32_t n);
    std::vector<unsigned char> postReceive(int32_t read_size);

    inline int32_t& send_len() { return send_len_; }
    inline int32_t& orig_len() { return orig_len_; }
    inline unsigned char* data() { return data_.data(); }

  protected:
    int32_t send_len_;
    int32_t orig_len_;
    std::array<unsigned char, MAX_STREAMING_MESSAGE_LEN> data_;
    bool preSend(const unsigned char *data,int32_t n);
};

///@}
