#include "json_loader.h"
#include <fstream>
using namespace std::literals;

namespace json_loader
{
    void CutString(std::string& str)
    {
        str.erase(0, 1);
        str.erase(str.size() - 1);
    }

    void AddRoads(model::Map& map, const boost::json::array& roads)
    {
        for (const auto& road : roads)
        {
            model::Point start{ road.at("x0").as_int64(), road.at("y0").as_int64() };
            
            try
            {
                int end = 0;
                end = road.at("x1").as_int64();
                model::Road r{ model::Road::HORIZONTAL, start, end };
                map.AddRoad(r);
                continue;
            }
            catch (...)
            {
            }
            try
            {
                int end = 0;
                end = road.at("y1").as_int64();
                model::Road r{ model::Road::VERTICAL, start, end };
                map.AddRoad(r);
                continue;
            }
            catch (...)
            {
            }

        }       
    }

    void AddBuildings(model::Map& map, const boost::json::array& buildings)
    {
        for (const auto& building : buildings)
        {
            model::Point pos{ building.at("x").as_int64(), building.at("y").as_int64() };
            model::Size size{ building.at("w").as_int64(), building.at("h").as_int64() };           
            model::Building b({ pos, size });
            map.AddBuilding(b);
        }
    }

    void AddOffices(model::Map& map, const boost::json::array& offices)
    {
        for (const auto& office : offices)
        {
            auto id = boost::json::serialize(office.at("id").as_string());
            CutString(id);
            util::Tagged<std::string, model::Office> id_tag{ id };
            model::Point position{ office.at("x").as_int64() , office.at("y").as_int64() };           
            model::Offset offset{ office.at("offsetX").as_int64() , office.at("offsetY").as_int64() };        
            model::Office o(id_tag, position, offset);
            map.AddOffice(o);
        }
    }

    model::Map CreateMap(const boost::json::value& value)
    {
        auto id = boost::json::serialize(value.at("id"));
        CutString(id);
        auto name = boost::json::serialize(value.at("name"s));
        CutString(name);
        util::Tagged<std::string, model::Map> id_tag{ id };
        model::Map map(id_tag, name);
        auto roads = value.at("roads").as_array();
        AddRoads(map, roads);
        auto buildings = value.at("buildings").as_array();
        AddBuildings(map, buildings);
        auto offices = value.at("offices").as_array();
        AddOffices(map, offices);
        return map;
    }

    model::Game LoadGame(const std::filesystem::path& json_path)
    {
        // Загрузить содержимое файла json_path, например, в виде строки
        // Распарсить строку как JSON, используя boost::json::parse
        // Загрузить модель игры из файла
        model::Game game;
        std::ifstream in(json_path);
        if (in)
        {
            std::string json_model_game;
            while (in)
            {
                std::string tmp;
                std::getline(in, tmp);
                json_model_game += tmp;
            }

            auto model_game = boost::json::parse(json_model_game);
            auto maps = model_game.as_object().at("maps"s).as_array();
            for (const auto& map : maps)
            {
                game.AddMap(CreateMap(map));
            }
        }

        return game;
    }

    boost::json::object MakeJson(const model::Building& model)
    {
        auto bounds = model.GetBounds();
        json::object jv;
        jv["x"] = bounds.position.x;
        jv["y"] = bounds.position.y;
        jv["h"] = bounds.size.height;
        jv["w"] = bounds.size.width;
        return jv;
    }

    boost::json::object MakeJson(const model::Office& model)
    {
        auto pos = model.GetPosition();
        auto offset = model.GetOffset();
        auto id = model.GetId();

        json::object jv;

        jv["id"] = id.operator*();
        jv["x"] = pos.x;
        jv["y"] = pos.y;
        jv["offsetX"] = offset.dx;
        jv["offsetY"] = offset.dy;
        return jv;
    }

    boost::json::object MakeJson(const model::Road& model)
    {
        auto s = model.GetStart();
        auto e = model.GetEnd();
        json::object jv;
        jv["x0"] = s.x;
        jv["y0"] = s.y;
        if (model.IsHorizontal()) jv["x1"] = e.x;
        else jv["y1"] = e.y;
        return jv;
    }

    boost::json::object MakeJsonResponseMapId(const model::Map& model)
    {
        json::object jv;
        jv["id"] = model.GetId().operator*();
        jv["name"] = model.GetName();
        jv["roads"] = AddJsonArray(model.GetRoads());
        jv["buildings"] = AddJsonArray(model.GetBuildings());
        jv["offices"] = AddJsonArray(model.GetOffices());
        return jv;
    }

    boost::json::array MakeJsonResponseMaps(const std::vector<model::Map>& model)
    {
        json::array jv;
        for (const auto& map : model)
        {
            json::object json_map;
            json_map["id"] = map.GetId().operator*();
            json_map["name"] = map.GetName();
            jv.emplace_back(std::move(json_map));
        }
        return jv;
    }
   
}  // namespace json_loader
