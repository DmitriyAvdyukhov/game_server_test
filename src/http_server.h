#pragma once
#ifdef WIN32
#include <sdkddkver.h>
#endif
#include "sdk.h"
// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <iostream>
#include <string_view>
#include <memory>

using namespace std::literals;

namespace http_server
{
    namespace net = boost::asio;
    using tcp = net::ip::tcp;
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace sys = boost::system;

    void ReportError(beast::error_code ec, std::string_view what);

    class SessionBase
    {
    public:

        SessionBase(const SessionBase&) = delete;
        SessionBase& operator=(const SessionBase&) = delete;

        void Run();

    protected:
        using HttpRequest = http::request<http::string_body>;

        explicit SessionBase(tcp::socket&& socket);
        ~SessionBase() = default;

        template<typename Body, typename Fields>
        void Write(http::response<Body, Fields>&& response)
        {
            auto safe_response = std::make_shared<http::response<Body, Fields>>(std::move(response));
            auto self = GetSharedThis();
            http::async_write(stream_, *safe_response,
                [safe_response, self](beast::error_code ec, std::size_t bytes_written)
                {
                    self->OnWrite(safe_response->need_eof(), ec, bytes_written);
                });
        }

    private:
        beast::tcp_stream stream_;
        beast::flat_buffer buffer_;
        HttpRequest request_;

        void Read();
        virtual std::shared_ptr<SessionBase> GetSharedThis() = 0;
        virtual void HandleRequest(HttpRequest&& request) = 0;
        void OnRead(beast::error_code ec, [[maybe_unused]] std::size_t bytes_read);
        void Close();
        void OnWrite(bool close, beast::error_code ec, [[maybe_unused]] std::size_t bytes_written);
    };

    template <typename RequestHandler>
    class Session : public SessionBase, public std::enable_shared_from_this<Session<RequestHandler>>
    {
    public:
        template<typename Handler>
        Session(tcp::socket&& socket, Handler&& request_handler)
            : SessionBase(std::move(socket))
            , request_handler_(std::forward<Handler>(request_handler))
        {}
    private:
        RequestHandler request_handler_;

        std::shared_ptr<SessionBase> GetSharedThis() override
        {
            return this->shared_from_this();
        }

        void HandleRequest(HttpRequest&& request) override
        {
            request_handler_(std::move(request), [self = this->shared_from_this()](auto&& response)
            {
                self->Write(std::move(response));
            });
        }
    };

    template <typename RequestHandler>
    class Listener : public std::enable_shared_from_this<Listener<RequestHandler>>
    {
    public:
        Listener(net::io_context& io, const tcp::endpoint& endpoint, RequestHandler&& request_handler)
            : ioc_(io)
            , acceptor_(net::make_strand(io))
            , request_handler_(std::forward<RequestHandler>(request_handler))
        {
            acceptor_.open(endpoint.protocol());
            acceptor_.set_option(net::socket_base::reuse_address(true));
            acceptor_.bind(endpoint);
            acceptor_.listen(net::socket_base::max_listen_connections);
        }

        void Run()
        {
            DoAccept();
        }

    private:
        net::io_context& ioc_;
        tcp::acceptor acceptor_;
        RequestHandler request_handler_;

        void DoAccept()
        {
            acceptor_.async_accept(
                net::make_strand(ioc_),
                beast::bind_front_handler(&Listener::OnAccept, this->shared_from_this())
            );
        }

        void OnAccept(sys::error_code ec, tcp::socket socket)
        {
            if (ec)
            {
                return ReportError(ec, "accept"sv);
            }
            AsyncRunSession(std::move(socket));

            DoAccept();
        }

        void AsyncRunSession(tcp::socket&& socket)
        {
            std::make_shared<Session<RequestHandler>>(std::move(socket), request_handler_)->Run();
        }
    };

    template <typename RequestHandler>
    void ServeHttp(net::io_context& ioc, const tcp::endpoint& endpoint, RequestHandler&& handler)
    {
        using MyListener = Listener<std::decay_t<RequestHandler>>;
        std::make_shared<MyListener>(ioc, endpoint, std::forward<RequestHandler>(handler))->Run();
    }
}  // namespace http_server
