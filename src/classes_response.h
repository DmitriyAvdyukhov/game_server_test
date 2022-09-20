#pragma once 
#include <string>
#include <filesystem>
#include <boost/json.hpp>
#include <variant>
#include "http_server.h"
#include "model.h"
#include "json_loader.h"

namespace classes_response
{
    namespace sys = boost::system;
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace fs = std::filesystem;
    namespace json = boost::json;


    using StringResponse = http::response<http::string_body>;

    // Ответ, тело которого представлено в виде файла
    using FileResponse = http::response<http::file_body>;

    using Responses = std::variant<StringResponse, FileResponse>;

    struct ContentType
    {
        ContentType() = delete;
        constexpr static std::string_view TEXT_HTML = "text/html"sv;
        constexpr static std::string_view TEXT_CSS = "text/css"sv;
        constexpr static std::string_view TEXT_PLAIN = "text/plain"sv;
        constexpr static std::string_view TEXT_JVASCRIPT = "text/javascript"sv;
        constexpr static std::string_view APPLICATION_JSON = "application/json"sv;
        constexpr static std::string_view APPLICATION_XML = "application/xml"sv;
        constexpr static std::string_view APPLICATION_OCTET_STREAM = "application/octet-stream"sv;
        constexpr static std::string_view IMAGE_PNG = "image/png"sv;
        constexpr static std::string_view IMAGE_JPEG = "image/jpeg"sv;
        constexpr static std::string_view IMAGE_GIF = "image/gif"sv;
        constexpr static std::string_view IMAGE_BMP = "image/bmp"sv;
        constexpr static std::string_view IMAGE_VND = "image/vnd.microsoft.icon"sv;
        constexpr static std::string_view IMAGE_TIFF = "image/tiff"sv;
        constexpr static std::string_view IMAGE_SVG_XML = "image/svg+xml"sv;
        constexpr static std::string_view AUDIO_MPEG = "audio/mpeg"sv;
    };

    struct RequestType
    {
        RequestType() = delete;
        constexpr static std::string_view API = "/api/"sv;
        constexpr static std::string_view API_V1_MAPS = "/api/v1/maps"sv;
    };

    struct ResponseType
    {
        ResponseType() = delete;
        constexpr static std::string_view FILE = "file"sv;
        constexpr static std::string_view FILE_OUTSIDE = "file_outside"sv;
        constexpr static std::string_view FILE_NOT_FOUND = "file_not_found"sv;
        constexpr static std::string_view MAPS = "maps"sv;
        constexpr static std::string_view FIND_MAP_ID = "find_map_id"sv;
        constexpr static std::string_view ERROR_FIND_MAP_ID = "error_find_map_id"sv;
        constexpr static std::string_view ERROR_TYPE_REQUEST = "error_type_request"sv;
    };

    enum class Extension
    {
        HTM, HTML, CSS, TXT, JS, JSON, XML, PNG, JPG, JPE, JPEG, GIF, BMP, ICO, TIFF, TIF, SVG, SVGZ, MP3, EMP
    };

    struct TypeClassResponse
    {
        std::string name{};
        std::string data{};
        Extension file_extension{};
        http::verb method{};
    };

    class Response
    {
    public:

        virtual std::string MakeStringResponse(const std::string&) const noexcept;
       
        virtual http::status GetStatus() const noexcept;

        virtual void SetContentType(StringResponse& res) const noexcept;        

        virtual StringResponse GetStringResponse(const TypeClassResponse& req) const noexcept;       

        virtual void SetContentType(FileResponse& res, const TypeClassResponse& req) const noexcept;        

        virtual FileResponse GetFileResponse(const TypeClassResponse& req) const noexcept;        

        virtual Responses GetResponses(const TypeClassResponse&) const noexcept  = 0;
    };


    class ResponseClear : public Response
    {
    public:
        StringResponse GetStringResponse(const TypeClassResponse& req) const noexcept  override;
       
        Responses GetResponses(const TypeClassResponse& r) const noexcept  override;
        
    };


    class ResponseMaps : public Response
    {
    private:
        const model::Game& game_;
    public:
        ResponseMaps(const model::Game& game);    
        
        std::string MakeStringResponse(const std::string&) const noexcept override;    

        void SetContentType(StringResponse& res) const noexcept override;       

        Responses GetResponses(const TypeClassResponse& req) const noexcept override;
    };


    class ResponseMapId : public Response
    {
    private:
        const model::Game& game_;
    public:
        ResponseMapId(const model::Game& game);
        
        std::string MakeStringResponse(const std::string& id) const noexcept override;       

        void SetContentType(StringResponse& res) const noexcept override;       

        Responses GetResponses(const TypeClassResponse& map_id) const noexcept override;       
    };


    class ResponseErrorVersion : public Response
    {

    public:

        std::string MakeStringResponse(const std::string&) const noexcept override;
        
        http::status GetStatus() const noexcept override;       

        void SetContentType(StringResponse& res) const noexcept override;       

        Responses GetResponses(const TypeClassResponse& map_id) const noexcept override;
    };


    class ResponseErrorFindIdMap : public Response
    {
    public:
        std::string MakeStringResponse(const std::string&) const noexcept override;
       
        http::status GetStatus() const noexcept override;

        Responses GetResponses(const TypeClassResponse& map_id) const noexcept override;
    };


    class ResponseFileNotFound : public Response
    {
    public:
        http::status GetStatus() const noexcept override;
       
        void SetContentType(StringResponse& res) const noexcept override;       

        Responses GetResponses(const TypeClassResponse& map_id) const noexcept override;       

        std::string MakeStringResponse(const std::string& data) const noexcept override;       
    };


    class ResponseFileOutside : public Response
    {
    public:
        http::status GetStatus() const noexcept override;
       
        void SetContentType(StringResponse& res) const noexcept override;       

        Responses GetResponses(const TypeClassResponse& map_id) const noexcept override;        

        std::string MakeStringResponse(const std::string& data) const noexcept override;       
    };


    class ResponseFile : public Response
    {
    public:
        void SetContentType(FileResponse& res, const TypeClassResponse& req) const noexcept override;       

        http::status GetStatus() const noexcept override;       

        Responses GetResponses(const TypeClassResponse& req) const noexcept override;       
    };

}// end namespace classes_response