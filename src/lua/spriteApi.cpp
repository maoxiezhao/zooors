#include"lua\luaContext.h"
#include"lua\luaTools.h"
#include"lua\luaBinder.h"
#include"game\sprite.h"
#include"game\enumInfo.h"

const string LuaContext::module_sprite_name = "Sprite";

void LuaContext::RegisterSpriteModule()
{
	LuaBindClass<Sprite> spriteClass(l, module_sprite_name);
	spriteClass.AddDefaultMetaFunction();
	spriteClass.AddFunction("create", sprite_api_create);
	// method
	spriteClass.AddMethod("SetSize",	 sprite_api_set_size);
	spriteClass.AddMethod("GetSize",	 sprite_api_get_size);
	spriteClass.AddMethod("SetRotation", sprite_api_set_rotation);
	spriteClass.AddMethod("GetRotation", sprite_api_get_rotation);
	spriteClass.AddMethod("SetColor",	 sprite_api_set_color);
	spriteClass.AddMethod("SetBlend",	 sprite_api_set_blend);
	spriteClass.AddMethod("GetBlend",	 sprite_api_get_blend);
	spriteClass.AddMethod("SetOpacity",	 sprite_api_set_opacity);
	spriteClass.AddMethod("GetOpacity",	 sprite_api_get_opacity);
	spriteClass.AddMethod("SetAnchor",	 sprite_api_set_anchor);
	spriteClass.AddMethod("SetOutLined", sprite_api_set_outLined);
	spriteClass.AddMethod("SetBlinking", sprite_api_set_blinking);
	spriteClass.AddMethod("SetFlipX",	 sprite_api_set_flip_x);
	spriteClass.AddMethod("SetFlipY",	 sprite_api_set_flip_y);
	spriteClass.AddMethod("SetDeferredDraw", sprite_api_set_deferred);
	spriteClass.AddMethod("Draw",	     sprite_api_draw);
	spriteClass.AddMethod("SetRotateAnchor", &Sprite::SetRotateAnchor);
	spriteClass.AddMethod("SetWhiteBlink", &Sprite::SetWhite);

	// 下面的方法应该在drawapi实现，派生给sprtie,暂未实现
	spriteClass.AddMethod("GetPos",		 drawable_api_get_pos);
	spriteClass.AddMethod("SetPos",		 drawable_api_set_pos);
	spriteClass.AddMethod("RunMovement", drawable_api_run_movement);
	spriteClass.AddMethod("GetMovment",  drawable_api_get_movement);
	spriteClass.AddMethod("StopMovement",drawable_api_stop_movment);
}

/**
*	\brief 将sprite压栈
*/
void LuaContext::PushSprite(lua_State*l, Sprite& sprite)
{
	PushUserdata(l, sprite);
}

/**
*	\brief 检查栈上index索引处是否为sprite userdata
*	\return the sprite
*/
SpritePtr LuaContext::CheckSprite(lua_State*l, int index)
{
	return std::static_pointer_cast<Sprite>(CheckUserdata(l, index, module_sprite_name));
}

/**
*	\brief 返回栈上index索引处是否sprite
*	\return true 如果是sprite
*/
bool LuaContext::IsSprite(lua_State*l, int index)
{
	return IsUserdata(l, index, module_sprite_name);
}

/**
*	\brief 实现cjing.Sprite.create()
*
*	这里存在2中创建精灵的方法
*	1.cjing.Sprite.create(pathStr)   创建指定的图片精灵
*	2.cjing.Sprite.create({r,g,b,a}) 创建指定颜色的图片块
*/
int LuaContext::sprite_api_create(lua_State*l)
{
	return LuaTools::ExceptionBoundary(l, [&] {
		if (lua_type(l, 1) != LUA_TTABLE && lua_type(l, 1) != LUA_TSTRING)
		{
			LuaTools::Error(l, "Invalid param in sprite::create.");
			return 0;
		}

		SpritePtr sprite = nullptr;
		if (lua_type(l, 1) == LUA_TSTRING)
		{
			const string spriteName = LuaTools::CheckString(l, 1);
			sprite = std::make_shared<Sprite>(spriteName);
		}
		else
		{
			const Color4B& color = LuaTools::CheckColor(l, 1);
			sprite = std::make_shared<Sprite>(color, Size(0, 0));
		}
		GetLuaContext(l).AddDrawable(sprite);
		PushSprite(l, *sprite);

		return 1;
	});
}

/**
*	\brief 实现sprite:SetSize(x,y)
*/
int LuaContext::sprite_api_set_size(lua_State*l)
{
	return LuaTools::ExceptionBoundary(l, [&] {
		Sprite& sprite = *CheckSprite(l, 1);
		int x = (int)LuaTools::CheckNumber(l, 2);
		int y = (int)LuaTools::CheckNumber(l, 3);

		sprite.SetSize(Size(x, y));

		return 0;
	});
}

/**
*	\brief 实现w,h = sprite:GetSize()
*/
int LuaContext::sprite_api_get_size(lua_State*l)
{
	return LuaTools::ExceptionBoundary(l, [&] {
		Sprite& sprite = *CheckSprite(l, 1);

		const Size size = sprite.GetSize();
		lua_pushinteger(l, size.width);
		lua_pushinteger(l, size.height);

		return 2;
	});
}

/**
*	\brief 实现sprite:SetBlend(srcBlend, dstBlend)
*/
int LuaContext::sprite_api_set_blend(lua_State*l)
{
	return LuaTools::ExceptionBoundary(l, [&] {
		Sprite& sprite = *CheckSprite(l, 1);
		GLenum srcBlend = LuaTools::CheckEnum<GLenum>(l, 2);
		GLenum dstBlend = LuaTools::CheckEnum<GLenum>(l, 3);

		sprite.SetBlendFunc(BlendFunc(srcBlend, dstBlend));

		return 0;
	});
}

/**
*	\brief 实现sprite:GetBlend,依次将srcBLend和dstBlend压入栈
*/
int LuaContext::sprite_api_get_blend(lua_State*l)
{
	return LuaTools::ExceptionBoundary(l, [&] {
		Sprite& sprite = *CheckSprite(l, 1);
		BlendFunc blend = sprite.GetBlendFunc();

		lua_pushstring(l, EnumToString(blend.srcBlend).c_str());
		lua_pushstring(l, EnumToString(blend.dstBlend).c_str());

		return 2;
	});
}

/**
*	\brief 实现sprite:SetOpacity(alpha)
*/
int LuaContext::sprite_api_set_opacity(lua_State*l)
{
	return LuaTools::ExceptionBoundary(l, [&] {
		Sprite& sprite = *CheckSprite(l, 1);
		int alpha = LuaTools::CheckInt(l, 2);
		sprite.SetOpacity(alpha);

		return 0;
	});
}

/**
*	\brief 返回透明度sprite:GetOpacity()
*/
int LuaContext::sprite_api_get_opacity(lua_State*l)
{
	return LuaTools::ExceptionBoundary(l, [&] {
		Sprite& sprite = *CheckSprite(l, 1);
		int alpha = sprite.GetOpacity();
		lua_pushinteger(l, alpha);

		return 1;
	});
}

int LuaContext::sprite_api_draw(lua_State*l)
{
	return LuaTools::ExceptionBoundary(l, [&] {
		Sprite& sprite = *CheckSprite(l, 1);
		/*int x = LuaTools::CheckInt(l, 2);
		int y = LuaTools::CheckInt(l, 3);*/
		sprite.Draw();

		return 0;
	});
}

int LuaContext::sprite_api_set_rotation(lua_State*l)
{
	return LuaTools::ExceptionBoundary(l, [&] {
		Sprite& sprite = *CheckSprite(l, 1);
		float angle = LuaTools::CheckFloat(l, 2);
		angle = Geometry::Degree(angle);
		sprite.SetRotated(angle);

		return 0;
	});
}

int LuaContext::sprite_api_get_rotation(lua_State*l)
{
	return LuaTools::ExceptionBoundary(l, [&] {
		Sprite& sprite = *CheckSprite(l, 1);
		float angle = sprite.GetRotated();
		lua_pushnumber(l, angle);

		return 1;
	});
}

/**
*	\brief 实现cjing.Sprite:setColor({r,g,b,a})
*/
int LuaContext::sprite_api_set_color(lua_State*l)
{
	return LuaTools::ExceptionBoundary(l, [&] {
		Sprite& sprite = *CheckSprite(l, 1);
		const Color4B color = LuaTools::CheckColor(l, 2);
		sprite.SetColor(color);

		return 0;
	});
}

int LuaContext::sprite_api_set_anchor(lua_State*l)
{
	return LuaTools::ExceptionBoundary(l, [&] {
		Sprite& sprite = *CheckSprite(l, 1);
		float x = LuaTools::CheckFloat(l, 2);
		float y = LuaTools::CheckFloat(l, 3);
		sprite.SetAnchorFloat(x, y );

		return 0;
	});
}

/**
*	\brief 实现cjing.Sprite:setOutLined(lineWidth)
*
*	当lineWidth大于0时，开启描边，小于等于0时关闭描边
*/
int LuaContext::sprite_api_set_outLined(lua_State*l)
{
	return LuaTools::ExceptionBoundary(l, [&] {
		Sprite& sprite = *CheckSprite(l, 1);
		float lineWidth = LuaTools::CheckFloat(l, 2);
		sprite.SetOutLine(lineWidth);

		return 0;
	});
}

/**
*	\brief 实现cjing.Sprite:setBlinking(blinkDelay)
*/
int LuaContext::sprite_api_set_blinking(lua_State*l)
{
	return LuaTools::ExceptionBoundary(l, [&] {
		Sprite& sprite = *CheckSprite(l, 1);
		uint32_t blinkDelay = LuaTools::CheckInt(l, 2);
		sprite.SetBlinking(blinkDelay);

		return 0;
	});
}

/**
*	\brief 实现cjing.Sprite:setFlipX(bool)
*/
int LuaContext::sprite_api_set_flip_x(lua_State*l)
{
	return LuaTools::ExceptionBoundary(l, [&] {
		Sprite& sprite = *CheckSprite(l, 1);
		bool fliped = LuaTools::CheckBoolean(l, 2);
		sprite.SetFlipX(fliped);

		return 0;
	});
}

/**
*	\brief 实现cjing.Sprite:setFlipY(bool)
*/
int LuaContext::sprite_api_set_flip_y(lua_State*l)
{
	return LuaTools::ExceptionBoundary(l, [&] {
		Sprite& sprite = *CheckSprite(l, 1);
		uint32_t blinkDelay = LuaTools::CheckInt(l, 2);
		bool fliped = LuaTools::CheckBoolean(l, 2);
		sprite.SetFlipY(fliped);

		return 0;
	});
}


/**
*	\brief 实现cjing.Sprite:setDeferredDraw(bool)
*/
int LuaContext::sprite_api_set_deferred(lua_State*l)
{
	return LuaTools::ExceptionBoundary(l, [&] {
		Sprite& sprite = *CheckSprite(l, 1);
		bool deferred = LuaTools::CheckBoolean(l, 2);
		sprite.SetDeferredDraw(deferred);

		return 0;
	});
}




