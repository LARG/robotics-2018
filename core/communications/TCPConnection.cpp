#include <communications/TCPConnection.h>
#include <communications/StreamingMessage.h>
#include <memory/StreamBuffer.h>
#include <boost/asio/error.hpp>

#define DEBUG_TCP false
#define ERROR_END_OF_FILE 2

using error_code = TCPConnection::error_code;

TCPConnection::TCPConnection(port server_port, ConnectionType type) 
  : server_port_(server_port), type_(type) {
  if(DEBUG_TCP) printf("Initializing TCP %s on port %i.\n", type_string().c_str(), server_port);
}

TCPConnection::~TCPConnection() {
  if(DEBUG_TCP) printf("Ending TCP %s on port %i.\n", type_string().c_str(), server_port_);
  stop_loop();
  stop_listener();
  stop_client();
  stop_server();
  stop_receiver();
  if(DEBUG_TCP) printf("All resources released for TCP %s on port %i.\n", type_string().c_str(), server_port_);
}

void TCPConnection::listener_handler(result_callback callback, error_code ec) {
  if(ec == 0) {
    if(DEBUG_TCP) printf("TCP Server connected successfully.\n");
    connection_established_ = true;
    accepting_connections_ = false;
    server_thread_ = make_thread(&TCPConnection::server_handler, this);
  }
  else if(DEBUG_TCP) 
    printf("TCP Server encountered an error during startup, code %i: '%s'\n", ec.value(), ec.message().c_str());
  receiver_cv_.notify_one();
  callback(ec);
}

void TCPConnection::start_io() {
  io_thread_ = make_thread([this](){
    if(DEBUG_TCP) printf("%s creating work object...\n", type_string().c_str());
    work_ = std::make_unique<io_work>(*io_);
    if(DEBUG_TCP) printf("%s running io...\n", type_string().c_str());
    io_->run();
    if(DEBUG_TCP) printf("%s ending io work.\n", type_string().c_str());
  });
}

void TCPConnection::stop_io() {
  if(DEBUG_TCP) printf("TCP %s killing io/work\n", type_string().c_str());
  work_.reset();
  if(io_ != nullptr)
    io_->stop();
  if(io_thread_ != nullptr)
    io_thread_->join();
  io_thread_.reset();
  io_.reset();
}

void TCPConnection::stop_listener() {
  if(DEBUG_TCP) printf("TCP Listener shutdown requested.\n");
  error_code ec;
  try {
    acceptor_.reset();
    stop_io();
    if(listener_thread_ != nullptr) {
      listener_thread_->join();
      listener_thread_.reset();
    }
  }
  catch (...) { }
  if(DEBUG_TCP) printf("TCP listener stopped.\n");
}

void TCPConnection::loop_server(result_callback callback) {
  if(DEBUG_TCP) printf("Starting a TCP %s loop.\n", type_string().c_str());
  using namespace std::literals;
  looping_ = true;
  auto looping_handler = [=]() {
    if(DEBUG_TCP) printf("-------START TCP SERVER LOOP -----------------\n");
    while(looping_) {
      if(DEBUG_TCP) printf("------------------LOOP 1\n");
      register_receiver();
      start_server(callback);
      if(DEBUG_TCP) printf("------------------LOOP 2\n");
      std::unique_lock<std::mutex> lock(looping_mutex_);
      if(DEBUG_TCP) printf("------------------LOOP 3\n");
      looping_cv_.wait(lock, [this] {
        // If we're still accepting connections then we 
        // don't need to restart the server.
        if(accepting_connections_) return false;

        // If our connection has died, relock so we can restart the server.
        // Or, if looping has been cancelled, relock so we can break out of
        // the looping thread.
        return !connection_established_ || !looping_; });
      if(DEBUG_TCP) printf("------------------LOOP 4%s\n", looping_ ? "" : ": LOOP FINISHED");
      if(!looping_) return;
      stop_server();
      clear_socket();
    }
  };
  looping_thread_ = make_thread(looping_handler);
}

void TCPConnection::start_server(result_callback callback) {
  init_socket();
  if(DEBUG_TCP) printf("TCP Server starting up, listening on port %i.\n", server_port_);
  acceptor_ = std::make_unique<tcp_acceptor>(*io_, tcp_endpoint(tcp::v4(), server_port_));
  if(DEBUG_TCP) printf(":::::::created acceptor, binding handler\n");
  auto handler = std::bind(&TCPConnection::listener_handler, this, callback, std::placeholders::_1);
  if(DEBUG_TCP) printf("handler bound, starting async accept\n");
  accepting_connections_ = true;
  acceptor_->async_accept(*socket_, handler);
  if(DEBUG_TCP) printf("async accept started, creating work\n");
  start_io();
}

void TCPConnection::server_handler() {
  if(DEBUG_TCP) printf("TCP Server thread started.\n");
  server_running_ = true;
  std::unique_lock<std::mutex> lock(server_mutex_);
  server_cv_.wait(lock, [this] { return !server_running_; });
  if(DEBUG_TCP) printf("TCP Server no longer running, thread stopping.\n");
  acceptor_.reset();
}

error_code TCPConnection::stop_server() {
  if(DEBUG_TCP) printf("TCP Server shutdown requested.\n");
  server_running_ = false;
  server_cv_.notify_one();
  if(server_thread_ != nullptr) {
    server_thread_->join();
    server_thread_.reset();
  }
  stop_io();
  error_code ec;
  clear_socket(ec);
  if(ec == 0 && DEBUG_TCP)
    printf("TCP Server stopped successfully.\n");
  else if(DEBUG_TCP) 
    printf("TCP Server encountered an error during shutdown, code %i: '%s'\n", ec.value(), ec.message().c_str());
  return ec;
}

void TCPConnection::stop_loop() {
  if(DEBUG_TCP) printf("TCP %s loop shutdown requested.\n", type_string().c_str());
  looping_ = false;
  accepting_connections_ = false;
  looping_cv_.notify_one();
  if(looping_thread_ != nullptr) {
    if(DEBUG_TCP) printf("TCP %s joining looping thread\n", type_string().c_str());
    looping_thread_->join();
    looping_thread_.reset();
  }
  if(DEBUG_TCP) printf("TCP %s loop shutdown completed.\n", type_string().c_str());
}

void TCPConnection::loop_client(std::string ip) {
  using namespace std::literals;
  looping_ = true;
  auto looping_handler = [=]() {
    if(DEBUG_TCP) printf("starting client loop handler\n");
    while(looping_) {
      if(DEBUG_TCP) printf("starting client connect to %s\n", ip.c_str());
      register_receiver();
      start_client(ip);
      if(DEBUG_TCP) printf("getting unique lock\n");
      std::unique_lock<std::mutex> lock(looping_mutex_);
      if(DEBUG_TCP) printf("waiting until connection failed or looping stopped\n");
      looping_cv_.wait(lock, [this] { return !connection_established_ || !looping_; });
      if(DEBUG_TCP) printf("conn: %i, looping: %i\n", connection_established_, looping_);
      if(!looping_) return;
      using namespace std::literals;
      if(DEBUG_TCP) printf("TCP Client sleeping before reconnect attempt.\n");
      std::this_thread::sleep_for(1s);
      stop_client();
    }
    if(DEBUG_TCP) printf("client loop handler break\n");
  };
  looping_thread_ = make_thread(looping_handler);
}

void TCPConnection::start_client(std::string ip) {
  init_socket();
  if(DEBUG_TCP) printf("TCP Client starting up, connecting to port %i.\n", server_port_);
  tcp_endpoint endpoint(ip_address::from_string(ip), server_port_);
  if(DEBUG_TCP) printf("TCP Client created endpoint\n");
  auto lock = new std::unique_lock<std::mutex>(looping_mutex_);
  auto handler = [this,lock](const boost::system::error_code& ec) {
    auto lock_ptr = std::unique_ptr<std::unique_lock<std::mutex>>(lock);
    if(DEBUG_TCP) printf("Client async_connect handler called\n");
    if(ec == 0) {
      connection_established_ = true;
      if(DEBUG_TCP) printf("TCP Client connected successfully.\n");
    } else if(DEBUG_TCP) {
      printf("TCP Client encountered an error during startup, code %i: '%s'\n", 
        ec.value(), ec.message().c_str()
      );
    }
    if(DEBUG_TCP) printf("TCP Client notifying receiver thread: socket created.\n");
    receiver_cv_.notify_one();
    looping_mutex_.unlock();
  };
  if(DEBUG_TCP) printf("TCP Client locked looping mutex, calling async_connect\n");
  socket_->async_connect(endpoint, handler);
  if(DEBUG_TCP) printf("TCP Client async_connect called, starting io thread\n");
  start_io();
}

error_code TCPConnection::stop_client() {
  if(DEBUG_TCP) printf("TCP Client shutdown requested.\n");
  connection_established_ = false;
  stop_io();
  error_code ec;
  clear_socket(ec);
  if(ec == 0 && DEBUG_TCP)
    printf("TCP Client stopped successfully.\n");
  else if(DEBUG_TCP) 
    printf("TCP Client encountered an error during shutdown, code %i: '%s'\n", ec.value(), ec.message().c_str());
  return ec;
}

bool TCPConnection::send_message(const StreamBuffer& buffer) {
  if(connection_established_) {
    if(DEBUG_TCP) printf("Sending TCP message (size %zd).\n", buffer.size);
    StreamingMessage smessage;
    bool success = false;
    try {
      auto result = smessage.sendMessage(*socket_, buffer.buffer, buffer.size);
      if(DEBUG_TCP) {
        if(result) printf("Message sent: failure - %s\n", result.message().c_str());
        else printf("Message sent: success!\n");
      }

      if(result == boost::system::errc::success) success = true;
      if(success) failed_messages_ = 0;
      else failed_messages_++;
      
      if(result == boost::system::errc::broken_pipe || failed_messages_ > MaximumFailedMessages) {
        // Reset to 0 and close the connection below
        failed_messages_ = 0;
      } else return success;
    } catch (...) { }
    if(!success) {
      if(DEBUG_TCP) printf("Error sending TCP message.\n");
      connection_ended();
      return false;
    }
  } else if(DEBUG_TCP) {
    printf("Could not send TCP message: connection failed.\n");
  }
  return false;
}

void TCPConnection::register_receiver() {
  if(callbacks_.size() == 0) return;
  if(DEBUG_TCP) printf("Registering TCP receiver.\n");
  if(receiver_thread_ != nullptr && receiver_running_) return;
  if(receiver_thread_ != nullptr && !receiver_running_) {
    receiver_thread_->join();
    receiver_thread_.reset();
  }
  receiver_thread_ = make_thread(&TCPConnection::receiver_handler, this);
}

void TCPConnection::receiver_handler() {
  if(DEBUG_TCP) printf("TCP receiver thread started, waiting to initialize the socket.\n");
  receiver_running_ = true;
  std::unique_lock<std::mutex> lock(receiver_mutex_);
  receiver_cv_.wait(lock, [this] { return !receiver_running_ || (socket_ != nullptr && connection_established_); });
  if(DEBUG_TCP) printf("TCP receiver socket initialized.\n");
  StreamingMessage smessage;
  auto &compressed_len = smessage.send_len(), &message_len = smessage.orig_len();
  std::size_t expected_length_bytes = sizeof(compressed_len);
  int frameid = 0;
  while(true) { // Use a while loop here so that we can 'restart' this method in the event
                // of a non-critical error. For example, bad_file_descriptor errors are
                // occasionally thrown, but it is safe to resume listening after these errors
                // occur. Thus when we encounter bad_file_descriptor in the catch block below,
                // we `restart` the method with a `continue` .
    try {
      while(receiver_running_) {
        if(DEBUG_TCP) printf("TCP receiver running: waiting for message...\n");
        {
          // Read the size of the compressed streaming message
          if(socket_ == nullptr) break; //TODO: This is a race condition, fix with a lock.
          auto returned_len = boost::asio::read(*socket_, boost::asio::buffer(&compressed_len, expected_length_bytes));
          // Make sure we read the correct number of bytes
          if(returned_len != expected_length_bytes) {
            fprintf(stderr, "TCP error reading compressed length. Returned: %zu, expected: %zu\n", 
              returned_len, expected_length_bytes
            );
            break;
          }
          // Make sure the compressed length isn't too large
          if(compressed_len > MAX_STREAMING_MESSAGE_LEN) {
            fprintf(stderr, "TCP error reading compressed length. Returned: %i, maximum: %i\n", 
              compressed_len, MAX_STREAMING_MESSAGE_LEN
            );
            break;
          }
        }
        if(DEBUG_TCP) printf("TCP receiver running: message received.\n");
        {
          // Read the size of the uncompressed payload
          if(socket_ == nullptr) break;
          auto returned_len = boost::asio::read(*socket_, boost::asio::buffer(&message_len, expected_length_bytes));
          // Make sure we read the correct number of bytes
          if(returned_len != expected_length_bytes) {
            fprintf(stderr, "TCP error reading message length. Returned: %zu, expected: %zu\n",
              returned_len, expected_length_bytes
            );
            break;
          }
        }
        {
          // Read the compressed payload
          auto payload_len = compressed_len - 2 * expected_length_bytes;
          if(socket_ == nullptr) break;
          auto returned_len = boost::asio::read(*socket_, boost::asio::buffer(smessage.data(), compressed_len - 2 * expected_length_bytes));
          receive_cache_ = smessage.postReceive(returned_len);
          if(receive_cache_.size() != message_len) {
            fprintf(stderr, "TCP error reading streaming message: compressed payload is corrupt.\n");
            break;
          }
          auto buffer = StreamBuffer(receive_cache_.data(), message_len);
          for(auto& cb : callbacks_)
            cb(buffer);
        }
      }
    } catch (boost::system::system_error e) {
      if(DEBUG_TCP) fprintf(stderr, "TCP %s handling an error: '%s' (%i)\n", type_string().c_str(), e.what(), e.code().value());
      if(e.code() == boost::system::errc::bad_file_descriptor) continue;
      if(e.code().value() == boost::asio::error::eof) break;
      if(e.code().value() == boost::asio::error::connection_refused) break;
      if(e.code().value() == boost::asio::error::not_connected) break;
      if(e.code().value() == boost::asio::error::connection_reset) break;
      if(e.code().value() == boost::asio::error::host_unreachable) break;
      fprintf(stderr, "TCP %s disconnecting: '%s' (%i)\n", type_string().c_str(), e.what(), e.code().value());
    }
    break;
  }
  if(DEBUG_TCP && !receiver_running_) printf("TCP receiver stop completed.\n");
  receiver_running_ = false;
  connection_ended();
}

void TCPConnection::stop_receiver() {
  if(DEBUG_TCP) printf("TCP receiver stop requested.\n");
  receiver_running_ = false;
  receiver_cv_.notify_one();
  if(receiver_thread_ != nullptr) {
    receiver_thread_->join();
    receiver_thread_.reset();
  }
}

bool TCPConnection::established() const {
  return connection_established_;
}

void TCPConnection::connection_ended() {
  connection_established_ = false;
  looping_cv_.notify_one();
}

void TCPConnection::init_socket() {
  if(io_ == nullptr) io_ = std::make_unique<io_service>();
  if(socket_ == nullptr) socket_ = std::make_unique<tcp_socket>(*io_);
}

void TCPConnection::clear_socket(error_code& ec) {
  if(socket_ != nullptr) {
    socket_->shutdown(tcp_socket::shutdown_both, ec);
    socket_->close(ec);
  }
  socket_.reset();
  io_.reset();
}
