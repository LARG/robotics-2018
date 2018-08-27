#include <communications/StreamingMessage.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <zlib.h>
#include <iostream>

#define DEBUG_SM false
  
bool StreamingMessage::preSend(const unsigned char *data,int32_t n) {
  if(DEBUG_SM) printf("pre-sending data of size %i (max %i)\n", n, MAX_STREAMING_MESSAGE_LEN);
  send_len_ = MAX_STREAMING_MESSAGE_LEN;
  orig_len_ = n;
  unsigned long ul_send_len = send_len_;
  int res = compress2(data_.data(),&ul_send_len,data,n,3);
  if(DEBUG_SM) printf("compressed to size %lu\n", ul_send_len);
  send_len_ = ul_send_len;
  if (res != 0) {
    std::cout << "Bad compress " << res << std::endl;
    std::cout << "buf: " << Z_BUF_ERROR << " mem: " << Z_MEM_ERROR << " stream: " << Z_STREAM_ERROR << std::endl;
    return false;
  }
  send_len_ += sizeof(send_len_) + sizeof(orig_len_);
  //std::cout << "send_len_ " << send_len_ << " orig_len_ " << orig_len_  << std::endl;
  return true;
}

std::vector<unsigned char> StreamingMessage::postReceive(int32_t read_size) {
  if(DEBUG_SM) printf("Allocating %i bytes for receiving message\n", orig_len_);
  auto buffer = std::vector<unsigned char>(orig_len_);
  if (read_size != (int32_t)(send_len_ - 2 * sizeof(send_len_))) {
    std::cout << "Bad TCP Message " << read_size << " " << send_len_ << std::endl;
    return decltype(buffer)();
  }

  unsigned long ul_orig_len = orig_len_;
  if(DEBUG_SM) printf("Buffer allocated, uncompressing %i bytes to %i bytes\n", send_len_, orig_len_);
  int ret = uncompress(buffer.data(),&ul_orig_len,data_.data(),send_len_);
  if (ret != Z_OK) {
    std::cout << "BAD UNCOMPRESS of tcp message " << ret << std::endl;
    return decltype(buffer)();
  }
  //std::cout << "UNCOMPRESS " << orig_len << std::endl << std::flush;
  if (ul_orig_len != orig_len_) {
    std::cout << "LEN MISMATCH " << ul_orig_len << " " << orig_len_ << std::endl;
    return decltype(buffer)();
  }
  if(DEBUG_SM) printf("Buffer uncompressed successfully\n");
  return buffer;
}
  
boost::system::error_code StreamingMessage::sendMessage(tcp::socket &sock, const unsigned char *data, int32_t n) {
    auto success = boost::system::errc::make_error_code(boost::system::errc::success);
    long ret;

    if (!preSend(data,n))
      return success;
    //ret = send(sockTCP,(const char *)this,send_len_,0);
    //std::cout << "before write" << std::endl << std::flush;
    try {
      boost::system::error_code ec;
      ret = boost::asio::write(sock,boost::asio::buffer((const char*)this,send_len_), ec);
      if(DEBUG_SM) printf("wrote streaming message to socket\n");
      return ec;
    } catch (boost::system::system_error& e) {
      if(DEBUG_SM) printf("error writing streaming message to socket: %s\n", e.what());
      return e.code();
    }
    if (ret <= 0) {
      auto error = boost::system::errc::make_error_code(boost::system::errc::result_out_of_range);
      if(DEBUG_SM) printf("error writing streaming message to socket: returned length is negative\n");
      return error;
    }
    if (ret != (long)send_len_) {
      auto error = boost::system::errc::make_error_code(boost::system::errc::result_out_of_range);
      if(DEBUG_SM) printf("error writing streaming message to socket: returned length not equal to send length\n");
      return error;
    }
    return success;
  }
