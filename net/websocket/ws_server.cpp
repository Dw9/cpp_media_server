#include "ws_server.hpp"
#include "tcp_server.hpp"
#include "ws_session.hpp"
#include "ws_session_pub.hpp"
#include "wsimple/flv_websocket.hpp"
#include "logger.hpp"
#include "net_pub.hpp"

websocket_server::websocket_server(boost::asio::io_context& io_context, uint16_t port, int imp_type):io_ctx_(io_context)
    , imp_type_(imp_type) {
    server_ = std::make_shared<tcp_server>(io_context, port, this);
    server_->accept();
}

websocket_server::~websocket_server()
{

}

void websocket_server::on_accept(int ret_code, boost::asio::ip::tcp::socket socket) {
    if (ret_code == 0) {
        std::string key;
        make_endpoint_string(socket.remote_endpoint(), key);
        log_infof("tcp accept key:%s", key.c_str());

        if (imp_type_ == WEBSOCKET_IMPLEMENT_FLV_TYPE) {
            std::shared_ptr<websocket_session> session_ptr = std::make_shared<websocket_session>(io_ctx_, std::move(socket), this, key);
            ws_session_callback* cb = new flv_websocket();
            session_ptr->set_websocket_callback(cb);
            session_ptr->run();
            sessions_.insert(std::make_pair(key, session_ptr));
        }
    }
    server_->accept();
}

void websocket_server::on_close(const std::string& session_key) {
    auto iter = sessions_.find(session_key);
    if (iter == sessions_.end()) {
        log_warnf("the session key doesn't exist:%s", session_key.c_str());
        return;
    }
    log_infof("remove websocket session id:%s", session_key.c_str());
    sessions_.erase(iter);
}