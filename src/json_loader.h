#pragma once

#include <filesystem>
#include <boost/json.hpp>

#include "model.h"

namespace json_loader
{
    namespace json = boost::json;

	model::Game LoadGame(const std::filesystem::path& json_path);

	void CutString(std::string& str);

	void AddRoads(model::Map& map, const boost::json::array& roads);

	void AddBuildings(model::Map& map, const boost::json::array& buildings);

	void AddOffices(model::Map& map, const boost::json::array& offices);

	model::Map CreateMap(const boost::json::value& value);

    boost::json::object MakeJson(const model::Building& model);    

    boost::json::object MakeJson(const model::Office& model);    

    boost::json::object MakeJson(const model::Road& model);   

    template <typename Arr>
    json::array AddJsonArray(const Arr& arr);    

    boost::json::object MakeJsonResponseMapId(const model::Map& model);    

    boost::json::array MakeJsonResponseMaps(const std::vector<model::Map>& model);   

}  // namespace json_loader


template<typename Arr>
boost::json::array json_loader::AddJsonArray(const Arr& arr)
{
    json::array result;
    result.reserve(arr.size());
    for (const auto& elem : arr)
    {
        result.emplace_back(MakeJson(elem));
    }
    return result;
}
