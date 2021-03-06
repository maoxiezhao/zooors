#ifndef _MAP_H_
#define _MAP_H_

#include"common\common.h"
#include"lua\luaObject.h"
#include"game\drawable.h"
#include"game\mapGenerate.h"
#include"game\sprite.h"
#include"utils\size.h"
#include"utils\rectangle.h"
#include"entity\groundInfo.h"

class Game;
class Camera;
class LuaContext;
class Tileset;
class Entities;
class InputEvent;
class Tileset;
class Entity;
class Destination;
class Map;

/**
*	\brief 房间数据
*/
class MapRoom : public LuaObject
{
public:
	MapRoom(Map& map);

	void Initialize(const std::string& name, const MapData& data);
	void UnInitialize();
	void StartRoom();
	void StopRoom();

	/** Status */
	Map& GetMap();
	const Map& GetMap()const;
	std::string GetMapID()const;
	std::string GetMapPath()const;
	Point2 GetPos()const;
	Size GetSize()const;
	LuaContext& GetLuaContext();

	virtual const string GetLuaObjectName()const;

private:
	Map& mMap;
	std::string mMapName;
	std::string mMapPath;
	Point2 mPos;
	Size mSize;
	bool mIsStarted;
};
using MapRoomPtr = std::shared_ptr<MapRoom>;

/**
*	\brief map类
*
*	map类包含管理一下对象：
*	tileset		地图贴片合集
*	entities	地图实体合集
*	music		地图背景音乐
*	ground		地图地形合集
*	mapcamera	地图相机实体
*/
class Map : public LuaObject
{
public:
	explicit Map();
	explicit Map(const std::string& id);
	~Map();

	// system
	virtual void Load(Game* game);
	virtual void UnLoad();
	virtual void Start();
	virtual void Leave();
	virtual void Update();
	virtual bool NotifyInput(const InputEvent& event);
	virtual void Draw();
	virtual void DrawBackground();		// 绘制背景图（后景）
	virtual void DrawForeground();		// 绘制背景图（前景）

	void DrawOnMap(Drawable& drawabel);
	void DrawOnMap(Drawable& drawabel, const Point2& pos);

	// status
	bool IsLoaded()const;
	bool IsStarted()const;
	bool IsSuspended()const;
	void CheckSuspended();
	void SetSuspended(bool suspended);
	bool IsValidLayer(int layer)const;
	Ground GetGround(int layer, int x, int y);

	Game& GetGame();
	LuaContext& GetLuaContext();
	const string GetLuaObjectName()const;
	Entities& GetEntities();
	std::shared_ptr<Camera>& GetCamera();
	const std::shared_ptr<Camera>& GetCamera()const;
	std::vector<std::shared_ptr<Entity> > GetObstacles(const Rect& rect, Entity& src)const;

	// map property
	const string& GetMapID()const;
	const Tileset& GetTileset()const;
	void SetTileset(const string& tilesetID);
	const string& getTilesetID()const;
	int GetMinLayer()const;
	int GetMaxLayer()const;
	Size GetSize()const;
	int GetWidth()const;
	int GetHeight()const;
	void SetBackground(SpritePtr background);
	std::string GetDestination()const;
	void SetDestination(const std::string& name);
	Destination* GetDestination();
	Point2 GetCameraLeftTopPos()const;

	// test collison
	bool TestCollisionWithObstacle(const Rect& rect, Entity& entity);
	bool TestCollisionWithGround(int layer, int x, int y, Entity& entity);
	bool TestCollisionWithEntities(const Rect& rect, Entity& entity, int layer);
	
	void CheckCollisionWithEntities(Entity& entity);
	void CheckCollisionFromEntities(Entity& entity);
	void CheckCollisionWithEntities(Entity& entity, Sprite& sprite);
	void CheckCollisionFromEntities(Entity& entity, Sprite& sprite);
private:
	/** status */
	string mMapID;
	int mWidth;
	int mHeight;
	int mMinLayer;
	int mMaxLayer;
	std::string mDestinationName;
	Game* mGame;

	bool mIsLoaded;
	bool mIsStarted;
	bool mSuspended;

	/** entity */
	string mTilesetId;
	std::shared_ptr<Tileset> mTileset;
	std::shared_ptr<Camera> mCamera;
	std::unique_ptr<Entities> mEntities;

	/** background */
	SpritePtr mBackGround;
	SpritePtr mFrontGround;

	/** map generate */
	MapGenerate mMapGenerate;
	std::vector<MapRoomPtr> mRooms;
};

#endif