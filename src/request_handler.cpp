#include "request_handler.h"

namespace http_handler
{
	RequestHandler::RequestHandler(model::Game& game)
		: game_{ game }
	{ 
        responses_.insert({ "maps", std::make_shared<ResponseMaps>(game_) });
        responses_.insert({ "error_version", std::make_shared<ResponseErrorVersion>() });
        responses_.insert({ "error_find_id_map", std::make_shared<ResponseErrorFindIdMap>() });
        responses_.insert({ "find_id_map", std::make_shared<ResponseMapId>(game_) });
        responses_.insert({ "file", std::make_shared<ResponseFile>() });
        responses_.insert({ "file_not_found", std::make_shared<ResponseFileNotFound>() });
        responses_.insert({ "file_outside", std::make_shared<ResponseFileOutside>() });
        responses_.insert({ "", std::make_shared<ResponseClear>() });
    }

	StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version, bool keep_alive, std::string_view content_type)
    {
        StringResponse response(status, http_version);
        response.set(http::field::content_type, content_type);
        response.body() = body;
        response.content_length(body.size());
        response.keep_alive(keep_alive);
        return response;
    }
}  // namespace http_handler
