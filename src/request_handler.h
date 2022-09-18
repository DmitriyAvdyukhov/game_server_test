#pragma once
#include "http_server.h"
#include "model.h"
#include <sstream>
#include <memory>
#include <unordered_map>
#include <variant>
#include <filesystem>

namespace http_handler
{
    namespace sys = boost::system;
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace fs = std::filesystem;
    using namespace http;

    // Запрос, тело которого представлено в виде строки
    using StringRequest = http::request<http::string_body>;
    // Ответ, тело которого представлено в виде строки
    using StringResponse = http::response<http::string_body>;
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

        // При необходимости внутрь ContentType можно добавить и другие типы контента
    };

    enum class Extension
    {
        HTM, HTML, CSS, TXT, JS, JSON, XML, PNG, JPG, JPE, JPEG, GIF, BMP, ICO, TIFF, TIF, SVG, SVGZ, MP3, EMP
    };

    // Создаёт StringResponse с заданными параметрами
    StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
        bool keep_alive,
        std::string_view content_type = ContentType::APPLICATION_JSON);
   
    struct BeforeParse
    {
        std::string name{};
        std::string parametr{};
        Extension file_extension{};
        http::verb method{};
    };

    class Response
    {
    public:      
       
        virtual std::string MakeStringResponse(const std::string&) 
        {
            return {};
        }
        virtual status GetStatus() { return status::ok; };

        virtual void SetContentType(StringResponse& res)
        {
            res.insert(field::content_type, ContentType::APPLICATION_JSON);
        }
        
        virtual StringResponse GetStringResponse(const BeforeParse& req)
        {
            StringResponse res;
            res.version(11);
            res.result(GetStatus());
            SetContentType(res);
            if (req.method == http::verb::get)
            {
                string_body::value_type str_body{ MakeStringResponse(req.parametr) };
                res.body() = std::move(str_body);
            }
            res.prepare_payload();
            return res;
        }
    
        virtual void SetContentType(FileResponse& res, const BeforeParse& req)
        {}

        virtual FileResponse GetFileResponse(const BeforeParse& req)
        {
            FileResponse res;
            res.version(11);  // HTTP/1.1
            res.result(GetStatus());
            SetContentType(res, req);
            if (req.method == http::verb::get)
            {
                file_body::value_type file;
                sys::error_code ec;
                file.open(req.parametr.c_str(), beast::file_mode::read, ec);
                res.body() = std::move(file);
            }
            res.prepare_payload();
            return res;
        }

        virtual Responses GetResponses(const BeforeParse&) = 0;
    };

    class ResponseClear : public Response
    {
    public:
        StringResponse GetStringResponse(const BeforeParse& req) override
        {
            return {};
        }
        Responses GetResponses(const BeforeParse& r) override
        {
            return GetStringResponse(r);
        }
    };

    class ResponseMaps : public Response
    {
    private:
        const model::Game& game_;
    public:
        ResponseMaps(const model::Game& game)
            :game_(game)
        {}

        std::string MakeStringResponse(const std::string&) override
        {
            std::stringstream str;
            bool is_first = true;
            str << "[";
            for (const auto& map : game_.GetMaps())
            {
                if (is_first)
                {
                    str << "{\"id\": \"";
                }
                else
                {
                    str << ",\n{\"id\": \"";
                }
                str << *map.GetId() + "\", \"name\": \"" + map.GetName() + "\"}";

            }
            str << "]";
            return str.str();
        }

        status GetStatus() override
        {
            return status::ok;
        }

        void SetContentType(StringResponse& res) override
        {
            res.insert(field::content_type, ContentType::TEXT_PLAIN);
        }

        Responses GetResponses(const BeforeParse& req) override
        {
            Responses res{ GetStringResponse(req) };
            return res;
        }
       
    };

    class ResponseMapId : public Response
    {
    private:
        const model::Game& game_;
    public:
        ResponseMapId(const model::Game& game)
            :game_(game)
        {}
        std::string MakeStringResponse(const std::string& id) override
        {
            util::Tagged<std::string, model::Map> id_map{ id };
            auto map = game_.FindMap(id_map);
            std::stringstream str;
            bool is_first = true;
            str << "{\n  \"id\": \"" << *map->GetId() + "\",\n  \"name\": \"" << map->GetName() + "\",\n  \"roads\": [";
            bool is_first_road = true;
            for (const auto& road : map->GetRoads())
            {
                if (is_first_road)
                {
                    str << "\n    { \"x0\": ";
                    is_first_road = false;
                }
                else
                {
                    str << ",\n    { \"x0\": ";
                }
                str << road.GetStart().x;
                str << ", \"y0\": ";
                str << road.GetStart().y;
                str << ", ";
                if (road.GetEnd().x == road.GetStart().x)
                {
                    str << "\"y1\": ";
                    str << road.GetEnd().y;
                }
                else if (road.GetEnd().y == road.GetStart().y)
                {
                    str << "\"x1\": ";
                    str << road.GetEnd().x;
                }
                else
                {
                    str << "\"x1\": ";
                    str << road.GetEnd().x;
                    str << "\"y1\": ";
                    str << road.GetEnd().y;
                }
                str << " }";
            }
            bool is_first_building = true;
            str << "\n  ],\n  \"buildings\": [";
            for (const auto& b : map->GetBuildings())
            {
                if (is_first_building)
                {
                    str << "\n    { \"x\": ";
                    is_first_building = false;
                }
                else
                {
                    str << ",\n    { \"x\": ";
                }

                str << b.GetBounds().position.x;

                str << ", \"y\": ";
                str << b.GetBounds().position.y;

                str << ", \"w\": ";
                str << b.GetBounds().size.width;

                str << ", \"h\": ";
                str << b.GetBounds().size.height;

                str << " }";
            }
            bool is_first_office = true;
            str << "\n  ],\n  \"offices\": [";
            for (const auto& o : map->GetOffices())
            {
                if (is_first_office)
                {
                    str << "\n    { \"id\": \"";
                    is_first_office = false;
                }
                else
                {
                    str << ",\n    { \"id\": \"";
                }

                str << *o.GetId();
                str << "\", \"x\": ";
                str << o.GetPosition().x;
                str << ", \"y\": ";
                str << o.GetPosition().y;
                str << ", \"offsetX\": ";
                str << o.GetOffset().dx;
                str << ", \"offsetY\": ";
                str << o.GetOffset().dy;
                str << " }";
            }
            str << "\n  ]\n}";
            return str.str();
        }

        status GetStatus() override
        {
            return status::ok;
        }

        void SetContentType(StringResponse& res) override
        {
            res.insert(field::content_type, ContentType::TEXT_PLAIN);
        }

        Responses GetResponses(const BeforeParse& map_id) override
        {
            return GetStringResponse(map_id);
        }
    };

    class ResponseErrorVersion : public Response
    {
    
    public:       

        std::string MakeStringResponse(const std::string&) override
        {
            return  "{\n  \"code\": \"badRequest\",\n  \"message\": \"Bad request\"\n}";
        }
        status GetStatus() override
        {
            return http::status::bad_request;
        }

        Responses GetResponses(const BeforeParse& map_id) override
        {
            return GetStringResponse(map_id);
        }
       
    };

    class ResponseErrorFindIdMap : public Response
    {
    public:
        std::string MakeStringResponse(const std::string&) override
        {
            return  "{\n  \"code\": \"mapNotFound\",\n  \"message\": \"Map not found\"\n}";
        }
        status GetStatus() override
        {
            return http::status::not_found;
        }

        Responses GetResponses(const BeforeParse& map_id) override
        {
            return GetStringResponse(map_id);
        }
    };

    class ResponseFileNotFound : public Response
    {
    public:
        status GetStatus() override
        {
            return http::status::not_found;
        }
        void SetContentType(StringResponse& res) override
        {
            res.insert(field::content_type, ContentType::TEXT_PLAIN);
        }

        Responses GetResponses(const BeforeParse& map_id) override
        {
            return GetStringResponse(map_id);
        }

        std::string MakeStringResponse(const std::string& parametr) override
        {
            return  parametr;
        }
    };

    class ResponseFileOutside : public Response
    {
    public:
        status GetStatus() override
        {
            return http::status::bad_request;
        }
        void SetContentType(StringResponse& res) override
        {
            res.insert(field::content_type, ContentType::TEXT_PLAIN);
        }

        Responses GetResponses(const BeforeParse& map_id) override
        {
            return GetStringResponse(map_id);
        }

        std::string MakeStringResponse(const std::string& parametr) override
        {
            return  parametr;
        }
    };
    class ResponseFile : public Response
    {
    public:
        void SetContentType(FileResponse& res, const BeforeParse& req) override
        {
            switch (req.file_extension)
            {
            case Extension::HTM:
                res.insert(field::content_type, ContentType::TEXT_HTML);
                break;
            case Extension::HTML:
                res.insert(field::content_type, ContentType::TEXT_HTML);
                break;
            case Extension::CSS:
                res.insert(field::content_type, ContentType::TEXT_CSS);
                break;
            case Extension::JS:
                res.insert(field::content_type, ContentType::TEXT_JVASCRIPT);
                break;
            case Extension::JSON:
                res.insert(field::content_type, ContentType::APPLICATION_JSON);
                break;
            case Extension::XML:
                res.insert(field::content_type, ContentType::APPLICATION_XML);
                break;
            case Extension::PNG:
                res.insert(field::content_type, ContentType::IMAGE_PNG);
                break;
            case Extension::JPG:
                res.insert(field::content_type, ContentType::IMAGE_JPEG);
                break;
            case Extension::JPE:
                res.insert(field::content_type, ContentType::IMAGE_JPEG);
                break;
            case Extension::JPEG:
                res.insert(field::content_type, ContentType::IMAGE_JPEG);
                break;
            case Extension::GIF:
                res.insert(field::content_type, ContentType::IMAGE_GIF);
                break;
            case Extension::BMP:
                res.insert(field::content_type, ContentType::IMAGE_BMP);
                break;
            case Extension::ICO:
                res.insert(field::content_type, ContentType::IMAGE_VND);
                break;
            case Extension::TIFF:
                res.insert(field::content_type, ContentType::IMAGE_TIFF);
                break;
            case Extension::TIF:
                res.insert(field::content_type, ContentType::IMAGE_TIFF);
                break;
            case Extension::SVG:
                res.insert(field::content_type, ContentType::IMAGE_SVG_XML);
                break;
            case Extension::SVGZ:
                res.insert(field::content_type, ContentType::IMAGE_SVG_XML);
                break;
            case Extension::MP3:
                res.insert(field::content_type, ContentType::AUDIO_MPEG);
                break;
            default:
                res.insert(field::content_type, ContentType::APPLICATION_OCTET_STREAM);
            }
        }

        status GetStatus() override
        {
            return status::ok;;
        }
      
        Responses GetResponses(const BeforeParse& req) override
        {
            return GetFileResponse(req);
        }
    };

    class RequestHandler
    {
    public:
        explicit RequestHandler(model::Game& game);
           

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
        std::unordered_map<std::string, std::shared_ptr<Response>> responses_;

        
        // Возвращает true, если каталог p содержится внутри base_path.
        bool IsSubPath(fs::path path, fs::path base)
        {
            // Приводим оба пути к каноничному виду (без . и ..)
            path = fs::weakly_canonical(path);
            base = fs::weakly_canonical(base);

            // Проверяем, что все компоненты base содержатся внутри path
            for (auto b = base.begin(), p = path.begin(); b != base.end(); ++b, ++p)
            {
                if (p == path.end() || *p != *b)
                {
                    return false;
                }
            }
            return true;
        }

        void ChangeString(std::string& str)
        {
            size_t pos = 0;
            while (pos != std::string::npos)
            {
                pos = str.find_first_of('+');
                if (pos == std::string::npos)
                {
                    break;
                }
                str.erase(pos, 1);
                str.insert(pos, 1, ' ');
            }
            pos = 0;
            while (pos != std::string::npos)
            {
                pos = str.find_first_of('%');
                if (pos == std::string::npos)
                {
                    break;
                }
                unsigned char result = static_cast<unsigned char>(strtol(str.substr(pos + 1, 2).c_str(), NULL, 16));
                str.erase(pos, 3);
                str.insert(pos, 1, result);
            }
        }

        template <typename Body, typename Allocator>
        BeforeParse ParseRequest(http::request<Body, http::basic_fields<Allocator>>&& req)
        {
            BeforeParse result;
            result.method  = req.method();
            std::string target = std::string{ req.target() };
            ChangeString(target);
            std::string name = target.substr(0, 5);
            if (name != "/api/"s)
            {
                std::string name_file = target;
                if (name_file == "/"s || name_file == "/index.html"s)
                {
                    result.name = "file"s;
                    result.parametr = "/index.html"s;
                    result.file_extension = Extension::HTML;
                }
                else
                {
                    std::string base_path{ "../../wwwroot/" };
                    std::string path = name_file.substr(0, name_file.find_last_of('/'));
                }


            }
            else if (target == "/api/v1/maps"sv)
            {
                result.name = "maps";               
            }
            else if (target.substr(0, 12) == "/api/v1/maps" && target.size() > 12)
            {
                result.name = "find_id_map";
                result.parametr = target.substr(target.find_last_of('/') + 1);
            }
            else if (target.substr(0, 12) != "/api/v1/maps")
            {
                result.name = "error_version";              
            }


            return result;
        }

        template <typename Body, typename Allocator>
        Responses HandleRequest(http::request<Body, http::basic_fields<Allocator>>&& req)
        {          
            BeforeParse str = ParseRequest(std::move(req));
            return responses_[str.name]->GetResponses(str);           
        }
    };
}  // namespace http_handler
