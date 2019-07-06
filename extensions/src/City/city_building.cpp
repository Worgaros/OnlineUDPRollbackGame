/*
 MIT License

 Copyright (c) 2019 SAE Institute Switzerland AG

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include <City/city_building.h>
#include <engine/engine.h>

namespace neko
{

void CityBuildingManager::Update(const CityZoneManager& zoneManager, CityBuilderMap& cityMap, const float dt)
{

	spawnTimer_.Update(dt);
	if (spawnTimer_.IsOver())
	{
		//Add residential building
		{
			const auto& zones = zoneManager.GetZoneVector();
			std::vector<Zone> residentialZones;
			residentialZones.reserve(zones.size());
			std::copy_if(zones.begin(), zones.end(), std::back_inserter(residentialZones), [&](const Zone& zone)
			{
				const auto buildingAtPos = std::find_if(buildings_.begin(), buildings_.end(), [&zone](const Building& building) {
					return building.position == zone.position;
				});
				return zone.zoneType == ZoneType::RESIDENTIAL && buildingAtPos == buildings_.end();
			});
			if (!residentialZones.empty())
			{
				auto& newHousePlace = residentialZones[rand() % residentialZones.size()];
				AddBuilding({
					newHousePlace.position,
					sf::Vector2i(1, 1),
					CityTileType(
						(rand() % (Index(CityTileType::HOUSE4) - Index(CityTileType::HOUSE1))) +
						Index(CityTileType::HOUSE1)),
					(rand() % (10 - 5)) + 5
					},
					zoneManager,
					cityMap);
			}
		}
		//Add commercial building
		{
			const auto& zones = zoneManager.GetZoneVector();
			std::vector<Zone> commercialZones;
			commercialZones.reserve(zones.size());
			std::copy_if(zones.begin(), zones.end(), std::back_inserter(commercialZones), [&](const Zone& zone)
			{
				const auto buildingAtPos = std::find_if(buildings_.begin(), buildings_.end(), [&zone](const Building& building) {
					return building.position == zone.position;
				});
				return zone.zoneType == ZoneType::COMMERCIAL && buildingAtPos == buildings_.end();
			});
			if (!commercialZones.empty())
			{
				auto& newHousePlace = commercialZones[rand() % commercialZones.size()];
				AddBuilding({
					newHousePlace.position,
					sf::Vector2i(1, 1),
					CityTileType::OFFICE1,
				(rand() % (20 - 10)) + 10
					}, zoneManager, cityMap);
			}
		}
		spawnTimer_.Reset();
	}

}

void
CityBuildingManager::AddBuilding(Building building, const CityZoneManager& zoneManager, CityBuilderMap& cityMap)
{
	cityMap.RemoveCityElement(building.position);
	buildings_.push_back(building);
	std::sort(buildings_.begin(), buildings_.end(), [](const Building& b1, const Building& b2) {return (b1.position.y < b2.position.y); });
}


const std::vector<Building>& CityBuildingManager::GetBuildingsVector() const
{
	return buildings_;
}

void CityBuildingManager::RemoveBuilding(sf::Vector2i position)
{
	const auto buildingIt = std::find_if(buildings_.begin(), buildings_.end(), [&position](const Building& building)
	{
		for (int dx = 0; dx < building.size.x; dx++)
		{
			for (int dy = 0; dy < building.size.y; dy++)
			{
				if (position == building.position + sf::Vector2i(dx, -dy))
				{
					return true;
				}
			}
		}
		return false;
	});
	if (buildingIt != buildings_.end())
	{
		buildings_.erase(buildingIt);
	}
}

sf::Vector2i CityBuildingManager::FindHouse(ZoneType zoneType)
{
	switch (zoneType)
	{
	case ZoneType::RESIDENTIAL:
	{
		const auto result = std::find_if(buildings_.begin(), buildings_.end(), [](const Building& building)
		{
			if (building.buildingType != CityTileType::HOUSE1 && 
				building.buildingType != CityTileType::HOUSE2 && 
				building.buildingType != CityTileType::HOUSE3 && 
				building.buildingType != CityTileType::HOUSE4)
				return false;

			return building.occupancy < building.capacity;
		});
		if (result != buildings_.end())
		{
			return result->position;
		}
		break;
	}
	case ZoneType::COMMERCIAL:
		const auto result = std::find_if(buildings_.begin(), buildings_.end(), [](const Building& building)
		{
			if (building.buildingType != CityTileType::OFFICE1)
				return false;
			return building.occupancy < building.capacity;
		});
		break;
	case ZoneType::INDUSTRY:
		break;
	default:
		break;
	}
	return INVALID_TILE_POS;
}

Building* CityBuildingManager::GetBuildingAt(sf::Vector2i position)
{
	const auto result = std::find_if(buildings_.begin(), buildings_.end(), [&position](const Building& building)
	{
		return building.position == position;
	});
	if(result == buildings_.end())
	{
		return nullptr;
	}
	return &*result;
}
}
