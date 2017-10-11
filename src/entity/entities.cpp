#include "entities.h"
#include "entity\player.h"
#include "core\video.h"
#include "lua\luaContext.h"

Entities::Entities(Game&game, Map&map):
	mGame(game),
	mMap(map),
	mEntityNamed(),
	mAllEntities(),
	mEntityToDraw(),
	mEntityToRemove(),
	mCamera(nullptr),
	mPlayer(game.GetPlayer())
{
	// 初始化每层的layer
	mMapCellWidth = mMap.GetWidth() / groundCellWidth;
	mMapCellHeight = mMap.GetHeight() / groundCellHeight;
	for (int layer = mMap.GetMinLayer(); layer <= mMap.GetMaxLayer(); layer++)
	{
		TileRegions* tileRegion = new TileRegions(map, layer);
		mTileRegions.emplace(layer, std::unique_ptr<TileRegions>(tileRegion));

		mGrounds[layer].resize(mMapCellWidth*mMapCellHeight, Ground::GROUND_EMPTY);
	}

	// 初始化四叉树管理entity
	const int margin = 64;
	mEntityTree.Initialize({-margin, -margin, mMap.GetWidth() + margin, mMap.GetHeight() + margin});

	// 添加相机,Test:临时设置为屏幕中点，视野为整个屏幕
	mCamera = std::make_shared<Camera>(map);
	mCamera->SetSize(Video::GetScreenSize() / 2);
	mCamera->SetPos({ Video::GetScreenSize().width / 2, Video::GetScreenSize().height / 2 });
	mCamera->TracingEntity(*mPlayer);
}

Entities::~Entities()
{
	mEntityTree.Clear();
	mEntityNamed.clear();
	mEntityToDraw.clear();
	mEntityToRemove.clear();
	mAllEntities.clear();
}

/**
*	\brief 更新entity
*/
void Entities::Update()
{
	// update player
	mPlayer->Update();

	// update all entites
	for (const auto& entity : mAllEntities)
	{
		entity->Update();
	}

	mCamera->Update();
	mEntityToDraw.clear();
}

/**
*	\brief 绘制entity
*/
void Entities::Draw()
{
	// 添加需要绘制的entity，这里是只要entity在
	// 在相机范围时才添加到绘制列表
	if (mEntityToDraw.empty())
	{
		// 应从4叉数中获取相机范围内的对象集合
		Rect aroundCamera(
			mCamera->GetPos().x - mCamera->GetSize().width,
			mCamera->GetPos().y - mCamera->GetSize().height,
			mCamera->GetSize().width * 2,
			mCamera->GetSize().height * 2);

		EntityVector entitiesInCamera;
		GetEntitiesInRect(aroundCamera, entitiesInCamera);
		for (const auto& entity : entitiesInCamera)
		{
			int layer = entity->GetLayer();
			if (entity->IsVisible())
			{
				mEntityToDraw[layer].push_back(entity);
			}
		}

		// 对entityToDraw进行排序
		for (int layer = mMap.GetMinLayer(); layer <= mMap.GetMaxLayer(); layer++) 
		{
			std::sort(mEntityToDraw[layer].begin(), mEntityToDraw[layer].end(), [](EntityPtr& entity1, EntityPtr& entity2) {
				// layer compare
				if (entity1->GetLayer() < entity1->GetLayer())
				{
					return true;
				}
				else if (entity1->GetLayer() > entity1->GetLayer())
				{
					return false;
				}
				else
				{	// 当layer相同时，则根据y坐标比较
					if (!entity1->IsDrawOnYOrder() && entity2->IsDrawOnYOrder())
					{
						return true;
					}
					else if (entity1->IsDrawOnYOrder() && !entity2->IsDrawOnYOrder())
					{
						return false;
					}
					else if (entity1->IsDrawOnYOrder() && entity2->IsDrawOnYOrder())
					{
						if (entity1->GetPos().y < entity2->GetPos().y)
						{
							return true;
						}
						else if (entity1->GetPos().y > entity2->GetPos().y)
						{
							return false;
						}
					}		
					// 当layer相同，y坐标也相同时
					return true;
				}
			});
		}
	}

	// 根据层级绘制entity
	for (int layer = mMap.GetMinLayer(); layer <= mMap.GetMaxLayer(); layer++)
	{
		// 绘制tile
		mTileRegions[layer]->Draw();

		// 绘制普通entity
		auto curLayerEntityToDraw = mEntityToDraw[layer];
		for (const auto& entity : curLayerEntityToDraw)
		{
			entity->Draw();
		}
	}
}

void Entities::SetSuspended(bool suspended)
{
}

/**
*	\brief 响应地图创建成功
*/
void Entities::NotifyMapStarted()
{
	// 创建每层tileRegions的tile
	for (int layer = mMap.GetMinLayer(); layer <= mMap.GetMaxLayer(); layer++)
	{
		mTileRegions[layer]->Build();
	}

	// entity响应map startd
	for (const auto& entity : mAllEntities)
	{
		entity->NotifyMapStarted();
	}
}

/**
*	\brief 返回相机
*/
CameraPtr Entities::GetCamear() const
{
	return mCamera;
}

/**
*	\brief 返回entities的集合
*/
EntityList Entities::GetEntities()
{
	return mAllEntities;
}

/**
*	\brief 返回范围内的entities的集合
*/
void Entities::GetEntitiesInRect(const Rect & rect, EntityVector & entities) const
{
	entities = mEntityTree.GetElements(rect);
}

/**
*	\brief 添加tile图块
*/
void Entities::AddTile(const TileInfo & tileInfo)
{
	Debug::CheckAssertion(mMap.IsValidLayer(tileInfo.mLayer),"Invalid tile layer.");

	mTileRegions[tileInfo.mLayer]->AddTile(tileInfo);

	// add ground
	int tileLayer = tileInfo.mLayer;
	const Size tileSize = tileInfo.mSize;
	const Point2 tilePos = tileInfo.mPos;
	const Ground ground = tileInfo.mGround;

	switch (ground)
	{
	case Ground::GROUND_WALL:
		SetGround(ground, tileLayer, Rect(tilePos, tileSize));
		break;

	case Ground::GROUND_EMPTY:
		// empty do nothing.
		break;
	}

}

/**
*	\brief 设置地面地形
*	\param rect 设置的范围
*/
void Entities::SetGround(const Ground & ground, int layer, const Rect & rect)
{
	int cellBeginX = rect.x / groundCellWidth;
	int cellEndX = (rect.x + rect.width) / groundCellWidth;
	int cellBeginY = rect.y / groundCellHeight;
	int cellEndY = (rect.y + rect.height) / groundCellHeight;

	for (int cellY = cellBeginY; cellY < cellEndY; cellY++)
	{
		if (cellY < 0 || cellY >= mMapCellHeight)
		{
			continue;
		}
		for (int cellX = cellBeginX; cellX < cellEndX; cellX++)
		{
			SetGround(ground, layer, cellX, cellY);
		}
	}
}

/**
*	\brief 设置地面地形
*	\param cellX x方向的第n个cell
*	\param cellY y方向的第n个cell
*/
void Entities::SetGround(const Ground & ground, int layer, int cellX, int cellY)
{
	if (cellX >= 0 && cellX < mMapCellWidth && 
		cellY >= 0 && cellY < mMapCellHeight)
	{
		mGrounds[layer][cellY*mMapCellWidth + cellX] = ground;
	}
}


/**
*	\brief 获取指定位置ground的inline
*/
Ground Entities::GetGround(int layer, int cellX, int cellY)const
{
	if (cellX >= 0 && cellX < mMapCellWidth &&
		cellY >= 0 && cellY < mMapCellHeight)
	{
		return mGrounds.at(layer).at(cellY*mMapCellWidth + cellX);
	}
	return Ground::GROUND_EMPTY;
}

/**
*	\brief 根据entityData创建entities
*/
void Entities::InitEntities(const MapData& mapData)
{
	// 按照层级顺序创建entity
	for (int curLayer = mapData.GetMinLayer(); curLayer <= mapData.GetMaxLayer(); curLayer++)
	{
		for (int index = 0; index < mapData.GetEntityCountByLayer(curLayer); index++)
		{
			const EntityData& entityData = mapData.GetEntity(curLayer, index);
			if (!GetLuaContext().CreateEntity(entityData, mMap))
			{
				Debug::Error("Failed to create entity.");
			}
		}
	}
}

/**
*	\biref 添加一个新的entity
*/
void Entities::AddEntity(const EntityPtr& entity)
{
	if (entity == nullptr)
	{
		return;
	}
	Debug::CheckAssertion(mMap.IsValidLayer(entity->GetLayer()),
		"Invalid entity layer in adding entity.");

	mEntityTree.Add(entity, entity->GetRectBounding());

	mAllEntities.push_back(entity);
	entity->SetMap(&mMap);
}

void Entities::RemoveEntity(Entity& entity)
{
}

LuaContext & Entities::GetLuaContext()
{
	return mGame.GetLuaContext();
}

Map & Entities::GetMap()
{
	return mMap;
}

/**
*	\brief 响应entity的rect改变
*
*	当entity的大小和位置发生改变时，调用该函数来
*	维护quadTree结构的正确
*/
void Entities::NotifyEntityRectChanged(Entity & entity)
{
	auto& entityPtr = std::dynamic_pointer_cast<Entity>(entity.shared_from_this());	// get shared_ptr entity
	mEntityTree.Move(entityPtr, entityPtr->GetRectBounding());
}

