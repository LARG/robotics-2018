#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <functional>
#include <thread>
#include <common/CoreException.h>
#include <mutex>
#include <condition_variable>

class StreamBuffer;

class TCPConnection {
  public:
    enum class ConnectionType {
      Client,
      Server
    };
    
    bool send_message(const StreamBuffer& buffer);

    template<typename C, typename T, typename... Ts>
    void register_receiver(C callback, T* object, Ts... ts) {
      using namespace std::placeholders;
      auto bound_callback = std::bind(callback, object, _1, ts...);
      callbacks_.push_back(bound_callback);
    }
    template<typename C, typename... Ts>
    void register_receiver(C callback, Ts... ts) {
      using namespace std::placeholders;
      auto bound_callback = std::bind(callback, _1, ts...);
      callbacks_.push_back(bound_callback);
    }
    void stop_receiver();

    auto server_port() const { return server_port_; }
    bool established() const;

    void stop_loop();

    using tcp = boost::asio::ip::tcp;
    using ip_address = boost::asio::ip::address;
    using tcp_socket = boost::asio::ip::tcp::socket;
    using tcp_acceptor = boost::asio::ip::tcp::acceptor;
    using tcp_endpoint = boost::asio::ip::tcp::endpoint;
    using io_service = boost::asio::io_service;
    using error_code = boost::system::error_code;
    using port = uint16_t;
    using data_callback = std::function<void(const StreamBuffer& buffer)>;
    using result_callback = std::function<void(error_code ec)>;
    using io_work = io_service::work;

    static constexpr int MaximumFailedMessages = 10;
  
  protected:

    TCPConnection(port server_port, ConnectionType type);
    virtual ~TCPConnection();
   
    void loop_server(result_callback callback);
    void start_server(result_callback callback);
    error_code stop_server();
    void loop_client(std::string ip_string);
    void start_client(std::string ip_string);
    error_code stop_client();
    void start_io();
    void stop_io();
    
  private:
    template<typename... Ts>
    std::unique_ptr<std::thread> make_thread(Ts... ts) { 
      return std::make_unique<std::thread>(ts...);
    }
    std::string type_string() const { return type_ == ConnectionType::Client ? "Client" : "Server"; }
    
    void register_receiver();
    void stop_listener();

    void server_handler();
    void listener_handler(result_callback callback, error_code ec);
    void receiver_handler();
    void connection_ended();
    void init_socket();
    void clear_socket(error_code& ec);
    void clear_socket() { error_code ec; clear_socket(ec); }
    port server_port_;
    ip_address address_;
    std::vector<data_callback> callbacks_;
    std::unique_ptr<std::thread> 
      receiver_thread_, server_thread_, listener_thread_, looping_thread_, io_thread_;
    std::unique_ptr<tcp_acceptor> acceptor_;
    std::mutex server_mutex_, looping_mutex_, receiver_mutex_;
    std::condition_variable server_cv_, looping_cv_, receiver_cv_;
    std::unique_ptr<io_work> work_;
    std::unique_ptr<io_service> io_;
    std::unique_ptr<tcp_socket> socket_;
    bool 
      server_running_ = false, 
      receiver_running_ = false, 
      connection_established_ = false, 
      accepting_connections_ = false,
      looping_ = false
    ;
    ConnectionType type_;
    int8_t failed_messages_ = 0;
    std::vector<unsigned char> receive_cache_; 
};

class TCPClient : public TCPConnection {
  public:
    TCPClient(port server_port) : TCPConnection(server_port, ConnectionType::Client) { }
    using TCPConnection::start_client;
    using TCPConnection::loop_client;
    using TCPConnection::stop_client;
    using TCPConnection::stop_loop;
};

class TCPServer : public TCPConnection {
  public:
    TCPServer(port server_port) : TCPConnection(server_port, ConnectionType::Server) { }
    using TCPConnection::start_server;
    using TCPConnection::loop_server;
    using TCPConnection::stop_server;
    using TCPConnection::stop_loop;
};
