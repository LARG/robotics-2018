/// @ingroup communications
#ifndef STREAMINGMESSAGE_9P0HZ3LW
#define STREAMINGMESSAGE_9P0HZ3LW

#include <sys/types.h>
#include <sys/socket.h>
#include <zlib.h>

#include <iostream>

/// @addtogroup communications
///@{
#define MAX_STREAMING_MESSAGE_LEN 5000000

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

class StreamingMessage {
public:
  StreamingMessage() {}

  bool preSend(unsigned char *data,unsigned long n) {
    send_len_ = MAX_STREAMING_MESSAGE_LEN;
    orig_len_ = n;
    int res = compress2(data_,&send_len_,data,n,3);
    if (res != 0) {
      std::cout << "Bad compress " << res << std::endl;
      std::cout << "buf: " << Z_BUF_ERROR << " mem: " << Z_MEM_ERROR << " stream: " << Z_STREAM_ERROR << std::endl;
      return false;
    }
    send_len_ += sizeof(send_len_) + sizeof(orig_len_);
    //std::cout << "send_len_ " << send_len_ << " orig_len_ " << orig_len_  << std::endl;
    return true;
  }

  bool sendMessage(tcp::socket &sock, unsigned char *data, unsigned long n) {
    long ret;

    if (!preSend(data,n))
      return false;
    //ret = send(sockTCP,(const char *)this,send_len_,0);
    //std::cout << "before write" << std::endl << std::flush;
    try {
      ret = boost::asio::write(sock,boost::asio::buffer((const char*)this,send_len_));
    } catch (...) {
      std::cout << "Write threw error :(" << std::endl;
      return false;
    }
    if (ret <= 0) {
      std::cout << "Failure with send" << std::endl;
      return false;
    }
    if (ret != (long)send_len_) {
      std::cout << "Questionable send " << ret << " " << send_len_ << std::endl;
      return false;
    }
    return true;
  }

  unsigned char* postReceive(long read_size) {
    //memcpy(&send_len_,data_,sizeof(send_len_));
    //memcpy(&orig_len_,data_+sizeof(send_len_),sizeof(orig_len_));
    if (read_size != (long)(send_len_ - 2 * sizeof(send_len_))) {
      std::cout << "Bad TCP Message " << read_size << " " << send_len_ << std::endl;
      return NULL;
    }

    unsigned char* buffer = new unsigned char[orig_len_];
    unsigned long orig_len = orig_len_;
    int ret = uncompress(buffer,&orig_len,data_,send_len_);
    if (ret != Z_OK) {
      std::cout << "BAD UNCOMPRESS of tcp message " << ret << std::endl;
      delete []buffer;
      return NULL;
    }
    //std::cout << "UNCOMPRESS " << orig_len << std::endl << std::flush;
    if (orig_len != orig_len_) {
      std::cout << "LEN MISMATCH " << orig_len << " " << orig_len_ << std::endl;
      delete []buffer;
      return NULL;
    }
    return buffer;
  }

public:
  unsigned long send_len_;
  unsigned long orig_len_;
  unsigned char data_[MAX_STREAMING_MESSAGE_LEN];
};

///@}

#endif /* end of include guard: STREAMINGMESSAGE_9P0HZ3LW */
