#pragma once
#include "http_server.h"
#include "model.h"
#include "classes_response.h"
#include <boost/json.hpp>
#include <sstream>
#include <memory>
#include <unordered_map>
#include <variant>
#include <filesystem>
#include <unordered_set>
#include <functional>

namespace http_handler
{
    namespace sys = boost::system;
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace fs = std::filesystem;
    namespace json = boost::json;
   

    // Запрос, тело которого представлено в виде строки
    using StringRequest = http::request<http::string_body>;

    // Ответ, тело которого представлено в виде строки
    using StringResponse = http::response<http::string_body>;

    // Ответ, тело которого представлено в виде файла
    using FileResponse = http::response<http::file_body>;

    using Responses = std::variant<StringResponse, FileResponse>;

    struct HasherPath
    {
        std::size_t operator()(const fs::path& p) const noexcept;
       
    };

    class RequestHandler
    {
    public:
        explicit RequestHandler(model::Game& game, const fs::path& wwwroot);

        RequestHandler(const RequestHandler&) = delete;
        RequestHandler& operator=(const RequestHandler&) = delete;

        template <typename Body, typename Allocator, typename Send>
        void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send)
        {
            auto answer = HandleRequest(std::move(req));
            if (std::holds_alternative<StringResponse>(answer))
            {               
                send(std::get<0>(answer));
            }
            else
            {
                send(std::get<1>(answer));
            }
            
        }

    private:
        model::Game& game_;
        fs::path wwwroot_;
        std::unordered_map<std::string_view, std::shared_ptr<classes_response::Response>> responses_;
        std::unordered_set<fs::path, HasherPath> files_;


        classes_response::Extension GetExtension(const std::string& extension);        
        
        // Возвращает true, если каталог p содержится внутри base_path.
        bool IsSubPath(fs::path path, fs::path base);        

        void CutOutPlus(std::string& str);       

        void CutOutPercentage(std::string& str);       

        void ConversionExtension(std::string& str);        

        void ConversionNormalTypeTarget(std::string& str);       

        std::string PathToString(const fs::path& path);        

        classes_response::TypeClassResponse CreateResponseFileRoot(const http::verb& method);       

        classes_response::TypeClassResponse CreateResponseFileOutside(const http::verb& method);       

        classes_response::TypeClassResponse CreateResponseFileOther(const fs::path& path, const http::verb& method);        

        classes_response::TypeClassResponse CreateResponseFileNotFound(const http::verb& method);        

        classes_response::TypeClassResponse CreateResponseFile(std::string&& target, const http::verb& method);        

        classes_response::TypeClassResponse CreateResponseMaps(const http::verb& method);        

        classes_response::TypeClassResponse CreateResponseMapId(std::string&& target, const http::verb& method);       

        classes_response::TypeClassResponse CreateResponseErrorTypeRequest(const http::verb& method);       

        template <typename Body, typename Allocator>
        classes_response::TypeClassResponse ParseRequest(http::request<Body, http::basic_fields<Allocator>>&& req);

        template <typename Body, typename Allocator>
        Responses HandleRequest(http::request<Body, http::basic_fields<Allocator>>&& req);     
    };

    template<typename Body, typename Allocator>
    inline classes_response::TypeClassResponse RequestHandler::ParseRequest(http::request<Body, http::basic_fields<Allocator>>&& req)
    {
        std::string target = std::string{ req.target() };
        ConversionNormalTypeTarget(target);
        std::string type = target.substr(0, 5);
        if (type != classes_response::RequestType::API)
        {
            return CreateResponseFile(std::move(target), req.method());
        }
        else if (target == classes_response::RequestType::API_V1_MAPS)
        {
            return CreateResponseMaps(req.method());
        }
        else if (target.substr(0, 12) == classes_response::RequestType::API_V1_MAPS && target.size() > 12)
        {
            return CreateResponseMapId(std::move(target), req.method());
        }
        else if (target.substr(0, 12) != classes_response::RequestType::API_V1_MAPS)
        {
            return CreateResponseErrorTypeRequest(req.method());
        }
        return {};
    }

    template<typename Body, typename Allocator>
    inline Responses RequestHandler::HandleRequest(http::request<Body, http::basic_fields<Allocator>>&& req)
    {
        classes_response::TypeClassResponse str = ParseRequest(std::move(req));
        return responses_[str.name]->GetResponses(str);
    }
}  // namespace http_handler
