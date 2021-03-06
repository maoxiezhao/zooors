#pragma once

#include"movements\straightMovement.h"
#include"game\gameCommands.h"
#include"game\timer.h"

/**
*	\brief 角色的移动类
*
*	角色的移动类，获取gameCommand的移动方向和角色的移动速度
*	设置移动或者停止, 同时Player可以设置一个快速位移
*/

class PlayerMovement : public StraightMovement
{
public:
	PlayerMovement(int speed);

	virtual void Update();
	virtual const string GetLuaObjectName()const;

	virtual int GetDirection()const;
	virtual int GetDirection8()const;
	void ComputeMovement();
	int GetMovingSpeed()const;

	void StartShift(int shiftSpeed);
	void StopShift();
	bool IsShifting()const;

	virtual void NotifyObstacleReached();
private:
	int mMovingSpeed;
	bool mIsShift;
	Direction8 mDirection;
	Timer mShiftTimer;
};

inline int PlayerMovement::GetMovingSpeed()const
{
	return mMovingSpeed;
}

