#include "UDPWrapper.h"
#include <iostream>

#define inbound() (direction_ == Inbound || direction_ == Bidirectional)
#define outbound() (direction_ == Outbound || direction_ == Bidirectional)

UDPWrapper::UDPWrapper (unsigned short port, bool broadcast, std::string dest_ip, Direction direction):
  in_sock_(io_service_),
  out_sock_(io_service_),
  port_(port),
  direction_(direction),
  destruct_(false),
  listen_thread_(NULL),
  destination_(NULL)
{

  if(inbound()) {
    in_sock_.open(udp::v4());
    try {
      // allow multiple binds
      boost::asio::socket_base::reuse_address option(true);
      in_sock_.set_option(option);

      in_sock_.bind(udp::endpoint(udp::v4(),port));
    } catch (std::exception &e) {
      std::cout << "UDPWrapper: Warning, not binding in socket" << std::endl;
    }
  }

  if(outbound()) {
    try {
      destination_ = new udp::endpoint(boost::asio::ip::address_v4::from_string(dest_ip),port);
    } catch (std::exception &e) {
      std::cerr << "UDPWrapper::UDPWrapper Error setting destination ip" << std::endl;
      destination_ = NULL;
    }
    out_sock_.open(udp::v4());
    if (broadcast) {
      boost::asio::socket_base::broadcast option(true);
      out_sock_.set_option(option);
    }
  }
}

// This is a hack that sends empty data to the address we're listening to. This is required
// to get around an issue with boost wherein the call to socket::receive_from only returns
// once a packet is received.
void UDPWrapper::receiveEmptyPacket() {
  if(!inbound()) return;
  // Create a socket to the local address
  udp::socket close_sock(io_service_);
  close_sock.open(udp::v4());
  auto dest = udp::endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"),port_);

  // Send an empty packet to ourselves
  close_sock.send_to(boost::asio::buffer("",1),dest);

  // Join the listener
  if(listen_thread_)
    listen_thread_->join();
  close_sock.close();
}

UDPWrapper::~UDPWrapper() {
  destruct_ = true;

  // Cleanup
  if(inbound()) {
    receiveEmptyPacket();
    if(listen_thread_)
      delete listen_thread_;
    in_sock_.close();
  }
  if(outbound()) {
    if(destination_)
      delete destination_;
    out_sock_.close();
  }
}

void UDPWrapper::startListenThread(std::function<void(void*)> method, void *core) {
  if(!inbound()) return;
  auto func = [=] {
    while(true) {
      method(core);
      if(destruct_) break;
    }
  };
  listen_thread_ = new std::thread(func);
}

boost::asio::ip::address UDPWrapper::senderAddress() {
  return sender_.address();
}

bool UDPWrapper::sendToSender(const boost::asio::const_buffers_1 &buffers,std::size_t size) {
  if(!outbound()) return false;
  boost::asio::ip::address previous = destination_->address();
  destination_->address(senderAddress());
  send(buffers,size);
  destination_->address(previous);

  // If you're reading this it's probably time to delete it:
  std::cout << "Sent UDP response to " << senderAddress() << "\n";
  return true;
}

bool UDPWrapper::send(const boost::asio::const_buffers_1 &buffers,std::size_t size) {
  if(!outbound()) return false;
  if(!destination_) return false;
  try {
    std::size_t res = out_sock_.send_to(buffers,*destination_);
    if (res != size) {
      //std::cerr << "UDPWrapper::send Error sending message, expected send size: " << size << " got " << res << std::endl;
      return false;
    }
  } catch (std::exception &e) {
    //std::cerr << "UDPWrapper::send Error sending message: " << e.what() << std::endl;
    return false;
  }
  return true;
}

bool UDPWrapper::recv(boost::asio::mutable_buffers_1 buffers, std::size_t size, bool acceptLowerSize) {
  if(!inbound()) return false;
  try {
    std::size_t res = in_sock_.receive_from(buffers,sender_);
    if (acceptLowerSize) {
      if ((res > size) || (res <= 0)) {
        //std::cerr << "UDPWrapper::recv Error receiving message, expected receive size <= " << size << " got " << res << std::endl;
        return false;
      }
    } else {
      if (res != size) {
        //std::cerr << "UDPWrapper::recv Error receiving message, expected receive size: " << size << " got " << res << std::endl;
        return false;
      }
    }
  } catch (std::exception &e) {
    //std::cerr << "UDPWrapper::recv Error receiving message: " << e.what() << std::endl;
    return false;
  }
  return true;
}
