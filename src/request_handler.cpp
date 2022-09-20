#include "request_handler.h"

namespace http_handler
{
    std::size_t HasherPath::operator()(const fs::path& p) const noexcept
    {
        return fs::hash_value(p);
    }

    using namespace classes_response;
	RequestHandler::RequestHandler(model::Game& game, const fs::path& wwwroot)
		: game_{ game }
        , wwwroot_{wwwroot}
	{         
        for (auto const& dir_entry : std::filesystem::recursive_directory_iterator{ wwwroot_ })
        {
            if (fs::is_regular_file(dir_entry.symlink_status()))
            {
                files_.insert(dir_entry);
            }
        }
        responses_.insert({ ResponseType::MAPS, std::make_shared<ResponseMaps>(game_) });
        responses_.insert({ ResponseType::ERROR_TYPE_REQUEST, std::make_shared<ResponseErrorVersion>() });
        responses_.insert({ ResponseType::ERROR_FIND_MAP_ID, std::make_shared<ResponseErrorFindIdMap>() });
        responses_.insert({ ResponseType::FIND_MAP_ID, std::make_shared<ResponseMapId>(game_) });
        responses_.insert({ ResponseType::FILE, std::make_shared<ResponseFile>() });
        responses_.insert({ ResponseType::FILE_NOT_FOUND, std::make_shared<ResponseFileNotFound>() });
        responses_.insert({ ResponseType::FILE_OUTSIDE, std::make_shared<ResponseFileOutside>() });
        responses_.insert({ "", std::make_shared<ResponseClear>() });
    }

    classes_response::Extension RequestHandler::GetExtension(const std::string& extension)
    {
        using namespace classes_response;

        if (extension == ".htm" || extension == ".html")
        {
            return Extension::HTM;
        }
        else if (extension == ".css")
        {
            return Extension::CSS;
        }
        else if (extension == ".jpg" || extension == ".jpe" || extension == ".jpeg")
        {
            return Extension::JPEG;
        }
        else if (extension == ".tiff" || extension == ".tif")
        {
            return Extension::TIFF;
        }
        else if (extension == ".svg" || extension == ".svgz")
        {
            return Extension::SVG;
        }
        else if (extension == ".txt")
        {
            return Extension::TXT;
        }
        else if (extension == ".js")
        {
            return Extension::JS;
        }
        else if (extension == ".json")
        {
            return Extension::JSON;
        }
        else if (extension == ".xml")
        {
            return Extension::XML;
        }
        else if (extension == ".png")
        {
            return Extension::PNG;
        }
        else if (extension == ".gif")
        {
            return Extension::GIF;
        }
        else if (extension == ".bmp")
        {
            return Extension::BMP;
        }
        else if (extension == ".ico")
        {
            return Extension::ICO;
        }
        else if (extension == ".mp3")
        {
            return Extension::MP3;
        }
        else
        {
            return  Extension::EMP;
        }
    }
    bool RequestHandler::IsSubPath(fs::path path, fs::path base)
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
    void RequestHandler::CutOutPlus(std::string& str)
    {
        std::size_t pos = 0;
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
    }
    void RequestHandler::CutOutPercentage(std::string& str)
    {
        std::size_t pos = 0;;
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
    void RequestHandler::ConversionExtension(std::string& str)
    {
        if (str.find_last_of('.') != std::string::npos)
        {
            std::string extension = str.substr(str.find_last_of('.'));
            std::transform(extension.begin(), extension.end(), extension.begin(), tolower);
            str.erase(str.find_last_of('.'));
            str += extension;
        }
    }
    void RequestHandler::ConversionNormalTypeTarget(std::string& str)
    {
        CutOutPlus(str);
        CutOutPercentage(str);
        ConversionExtension(str);
    }
    std::string RequestHandler::PathToString(const fs::path& path)
    {
        std::string result;
        std::stringstream str;
        str << path;
        str >> result;
        result.erase(0, 1);
        result.erase(result.size() - 1, 1);
        return result;
    }
    classes_response::TypeClassResponse RequestHandler::CreateResponseFileRoot(const http::verb& method)
    {
        classes_response::TypeClassResponse result;
        result.method = method;
        result.name = classes_response::ResponseType::FILE;
        fs::path requested_file_path = wwwroot_;
        requested_file_path += fs::path("/index.html");
        result.data = PathToString(requested_file_path);
        result.file_extension = classes_response::Extension::HTML;
        return result;
    }
    classes_response::TypeClassResponse RequestHandler::CreateResponseFileOutside(const http::verb& method)
    {
        classes_response::TypeClassResponse result;
        result.method = method;
        result.name = classes_response::ResponseType::FILE_OUTSIDE;
        result.data = "Your file is outside root category";
        return result;
    }
    classes_response::TypeClassResponse RequestHandler::CreateResponseFileOther(const fs::path& path, const http::verb& method)
    {
        classes_response::TypeClassResponse result;
        result.method = method;
        result.name = classes_response::ResponseType::FILE;
        result.data = PathToString(path);
        if (result.data.find_last_of('.') == std::string::npos)
        {
            result.file_extension = classes_response::Extension::EMP;
        }
        else
        {
            result.file_extension = GetExtension(result.data.substr(result.data.find_last_of('.')));
        }
        return result;
    }
    classes_response::TypeClassResponse RequestHandler::CreateResponseFileNotFound(const http::verb& method)
    {
        classes_response::TypeClassResponse result;
        result.method = method;
        result.name = classes_response::ResponseType::FILE_NOT_FOUND;
        result.data = "Your file not found";
        return result;
    }
    classes_response::TypeClassResponse RequestHandler::CreateResponseFile(std::string&& target, const http::verb& method)
    {
        std::string name_file = std::move(target);
        if (name_file == "/"s || name_file == "/index.html"s)
        {
            return CreateResponseFileRoot(method);
        }
        else
        {
            fs::path requested_file_path = wwwroot_;
            requested_file_path += fs::path(name_file);
            if (!IsSubPath(requested_file_path, wwwroot_))
            {
                return CreateResponseFileOutside(method);
            }
            else if (files_.find(requested_file_path) != files_.end())
            {
                return CreateResponseFileOther(requested_file_path, method);
            }
            else
            {
                return CreateResponseFileNotFound(method);
            }
        }
    }
    classes_response::TypeClassResponse RequestHandler::CreateResponseMaps(const http::verb& method)
    {
        classes_response::TypeClassResponse result;
        result.method = method;
        result.name = classes_response::ResponseType::MAPS;
        result.file_extension = classes_response::Extension::JSON;
        return result;
    }
    classes_response::TypeClassResponse RequestHandler::CreateResponseMapId(std::string&& target, const http::verb& method)
    {
        classes_response::TypeClassResponse result;
        result.method = method;
        std::string map = target.substr(target.find_last_of('/') + 1);
        util::Tagged<std::string, model::Map> id_map{ map };
        if (game_.FindMap(id_map))
        {
            result.name = classes_response::ResponseType::FIND_MAP_ID;
            result.data = target.substr(target.find_last_of('/') + 1);
            result.file_extension = classes_response::Extension::JSON;
        }
        else
        {
            result.name = classes_response::ResponseType::ERROR_FIND_MAP_ID;
        }
        return result;
    }
    classes_response::TypeClassResponse RequestHandler::CreateResponseErrorTypeRequest(const http::verb& method)
    {
        classes_response::TypeClassResponse result;
        result.method = method;
        result.name = classes_response::ResponseType::ERROR_TYPE_REQUEST;
        result.file_extension = classes_response::Extension::JSON;
        return result;
    }
    
}  // namespace http_handler
