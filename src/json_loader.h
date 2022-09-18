#pragma once

#include <filesystem>
#include <boost/json.hpp>


#include "model.h"

namespace json_loader
{
	model::Game LoadGame(const std::filesystem::path& json_path);

	void CutString(std::string& str);

	void AddRoads(model::Map& map, const boost::json::array& roads);

	void AddBuildings(model::Map& map, const boost::json::array& buildings);

	void AddOffices(model::Map& map, const boost::json::array& offices);

	model::Map CreateMap(const boost::json::value& value);

}  // namespace json_loader
