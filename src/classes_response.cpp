#include "classes_response.h"

namespace classes_response
{
	//------------ class Response--------------

	std::string Response::MakeStringResponse(const std::string&) const noexcept
	{
		return {};
	}

	http::status Response::GetStatus() const noexcept
	{
		return http::status::ok;
	}

	void Response::SetContentType(StringResponse& res) const noexcept
	{
		res.insert(http::field::content_type, ContentType::APPLICATION_JSON);
	}

	StringResponse Response::GetStringResponse(const TypeClassResponse& req) const noexcept
	{
		StringResponse res;
		res.version(11);
		res.result(GetStatus());
		SetContentType(res);
		if (req.method == http::verb::get)
		{
			http::string_body::value_type str_body{ MakeStringResponse(req.data) };
			res.body() = std::move(str_body);
		}
		res.prepare_payload();
		return res;
	}

	void Response::SetContentType(FileResponse& res, const TypeClassResponse& req) const noexcept
	{}

	FileResponse Response::GetFileResponse(const TypeClassResponse & req) const noexcept
	{
		FileResponse res;
		res.version(11);  // HTTP/1.1
		res.result(GetStatus());
		SetContentType(res, req);
		if (req.method == http::verb::get)
		{
			http::file_body::value_type file;
			sys::error_code ec;

			file.open(req.data.c_str(), beast::file_mode::read, ec);
			res.body() = std::move(file);
		}
		res.prepare_payload();
		return res;
	}


	//--------------------class ResponseCleare-----------

	StringResponse ResponseClear::GetStringResponse(const TypeClassResponse& req) const noexcept
	{
		return {};
	}

	Responses ResponseClear::GetResponses(const TypeClassResponse& r) const noexcept
	{
		return GetStringResponse(r);
	}

	//------------class ResponseMaps-------------------

	ResponseMaps::ResponseMaps(const model::Game& game)
		:game_(game)
	{}

	std::string ResponseMaps::MakeStringResponse(const std::string&) const noexcept
	{
		return boost::json::serialize(json_loader::MakeJsonResponseMaps(game_.GetMaps()));
	}

	void ResponseMaps::SetContentType(StringResponse& res) const noexcept
	{
		res.insert(http::field::content_type, ContentType::APPLICATION_JSON);
	}

	Responses ResponseMaps::GetResponses(const TypeClassResponse& req) const noexcept
	{
		Responses res{ GetStringResponse(req) };
		return res;
	}

	//-------------classResponseMapId-------------------


	ResponseMapId::ResponseMapId(const model::Game& game)
		:game_(game)
	{}

	std::string ResponseMapId::MakeStringResponse(const std::string & id) const noexcept
	{
		util::Tagged<std::string, model::Map> id_map{ id };
		auto map = game_.FindMap(id_map);

		return boost::json::serialize(json_loader::MakeJsonResponseMapId(*map));
	}

	void ResponseMapId::SetContentType(StringResponse& res) const noexcept
	{
		res.insert(http::field::content_type, ContentType::APPLICATION_JSON);
	}

	Responses ResponseMapId::GetResponses(const TypeClassResponse& map_id) const noexcept
	{
		return GetStringResponse(map_id);
	}

	//--------------class ResponseErrorVersion--------------


	std::string ResponseErrorVersion::MakeStringResponse(const std::string&) const noexcept
	{
		return  "{\n  \"code\": \"badRequest\",\n  \"message\": \"Bad request\"\n}";
	}

	http::status ResponseErrorVersion::GetStatus() const noexcept
	{
		return http::status::bad_request;
	}

	void ResponseErrorVersion::SetContentType(StringResponse& res) const noexcept
	{
		res.insert(http::field::content_type, ContentType::APPLICATION_JSON);
	}

	Responses ResponseErrorVersion::GetResponses(const TypeClassResponse& map_id) const noexcept
	{
		return GetStringResponse(map_id);
	}

	//--------class ResponseErrorFiandIdMap-----------------

	std::string ResponseErrorFindIdMap::MakeStringResponse(const std::string&) const noexcept
	{
		return  "{\n  \"code\": \"mapNotFound\",\n  \"message\": \"Map not found\"\n}";
	}

	http::status ResponseErrorFindIdMap::GetStatus() const noexcept
	{
		return http::status::not_found;
	}

	Responses ResponseErrorFindIdMap::GetResponses(const TypeClassResponse& map_id) const noexcept
	{
		return GetStringResponse(map_id);
	}

	//--------class ResponseFileNotFound-----------

	http::status ResponseFileNotFound::GetStatus() const noexcept
	{
		return http::status::not_found;
	}

	void ResponseFileNotFound::SetContentType(StringResponse& res) const noexcept
	{
		res.insert(http::field::content_type, ContentType::TEXT_PLAIN);
	}

	Responses ResponseFileNotFound::GetResponses(const TypeClassResponse& map_id) const noexcept
	{
		return GetStringResponse(map_id);
	}

	std::string ResponseFileNotFound::MakeStringResponse(const std::string& data) const noexcept
	{
		return  data;
	}

	//--------class ResponseFileOutside-----------

	http::status ResponseFileOutside::GetStatus() const noexcept
	{
		return http::status::bad_request;
	}

	void ResponseFileOutside::SetContentType(StringResponse& res) const noexcept
	{
		res.insert(http::field::content_type, ContentType::TEXT_PLAIN);
	}

	Responses ResponseFileOutside::GetResponses(const TypeClassResponse& map_id) const noexcept
	{
		return GetStringResponse(map_id);
	}

	std::string ResponseFileOutside::MakeStringResponse(const std::string& data) const noexcept
	{
		return  data;
	}

	//--------------class ResponseFile-----------------------

	void ResponseFile::SetContentType(FileResponse& res, const TypeClassResponse& req) const noexcept
	{
		switch (req.file_extension)
		{
		case Extension::HTM:
			res.insert(http::field::content_type, ContentType::TEXT_HTML);
			break;
		case Extension::HTML:
			res.insert(http::field::content_type, ContentType::TEXT_HTML);
			break;
		case Extension::CSS:
			res.insert(http::field::content_type, ContentType::TEXT_CSS);
			break;
		case Extension::JS:
			res.insert(http::field::content_type, ContentType::TEXT_JVASCRIPT);
			break;
		case Extension::JSON:
			res.insert(http::field::content_type, ContentType::APPLICATION_JSON);
			break;
		case Extension::XML:
			res.insert(http::field::content_type, ContentType::APPLICATION_XML);
			break;
		case Extension::PNG:
			res.insert(http::field::content_type, ContentType::IMAGE_PNG);
			break;
		case Extension::JPG:
			res.insert(http::field::content_type, ContentType::IMAGE_JPEG);
			break;
		case Extension::JPE:
			res.insert(http::field::content_type, ContentType::IMAGE_JPEG);
			break;
		case Extension::JPEG:
			res.insert(http::field::content_type, ContentType::IMAGE_JPEG);
			break;
		case Extension::GIF:
			res.insert(http::field::content_type, ContentType::IMAGE_GIF);
			break;
		case Extension::BMP:
			res.insert(http::field::content_type, ContentType::IMAGE_BMP);
			break;
		case Extension::ICO:
			res.insert(http::field::content_type, ContentType::IMAGE_VND);
			break;
		case Extension::TIFF:
			res.insert(http::field::content_type, ContentType::IMAGE_TIFF);
			break;
		case Extension::TIF:
			res.insert(http::field::content_type, ContentType::IMAGE_TIFF);
			break;
		case Extension::SVG:
			res.insert(http::field::content_type, ContentType::IMAGE_SVG_XML);
			break;
		case Extension::SVGZ:
			res.insert(http::field::content_type, ContentType::IMAGE_SVG_XML);
			break;
		case Extension::MP3:
			res.insert(http::field::content_type, ContentType::AUDIO_MPEG);
			break;
		default:
			res.insert(http::field::content_type, ContentType::APPLICATION_OCTET_STREAM);
		}
	}

	http::status ResponseFile::GetStatus() const noexcept
	{
		return http::status::ok;;
	}

	Responses ResponseFile::GetResponses(const TypeClassResponse& req) const noexcept
	{
		return GetFileResponse(req);
	}

}// end namespace classes_response