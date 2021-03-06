#include"game\app.h"
#include"game\gameResources.h"
#include"core\debug.h"
#include"core\logger.h"
#include"core\system.h"
#include"core\fileData.h"
#include"core\configData.h"
#include"core\video.h"
#include"core\renderer.h"
#include"core\fontAtlas.h"
#include"core\perf.h"

void CustomTest()
{
	// test
	//sprite = std::make_shared<Sprite>("test.png");
	//sprite->SetSize({ 200, 600 });
	//sprite->SetPos({ 500, 0 });

	//sprite_2 = std::make_shared<Sprite>("test.png");
	//sprite_2->SetSize({ 200, 600 });
	//sprite_2->SetPos({ 500, 0 });
	//	sprite->SetProgramState(ResourceCache::GetInstance().GetGLProgramState(GLProgramState::DEFAULT_G_BUFFER_PROGRAMSTATE_NAME));
	//	mLight = std::make_shared<PointLight>(Vec3i(500,330,50), 100, Color4B(55,55,255,255), 0.001, 0.0);
	//	Renderer::GetInstance().GetAreaLight()->SetColor({250, 250, 250, 255});
}

App::App() :
	mLuaContext(nullptr),
	mExiting(false),
	mCurrGame(nullptr),
	mNextGame(nullptr),
	mLight(nullptr)
{
	Logger::PrintConsoleHeader();
	Logger::Info("Cjing start initializing.");

	Logger::Info("Open data file");
	string dataPath = ".";
	if (!FileData::OpenData("", dataPath))
		Debug::Die("No data file was found int the direcion:" + dataPath);

	Logger::Info("Load config.");
	ConfigData::LoadConfig("config.dat");
	FileData::SetDataWriteDir(ConfigData::GetConfigProperties().GetWriteDir());

	// initialize game 
	Logger::Info("Load game resource info.");
	GameResource::GetGameResource().ImportFromFile("resources.dat");

	// initialize system
	Logger::Info("Initialize system modules");
	System::Initialize();

	// initialize gui main stage
	Logger::Info("Initialize GUI main stage");
	auto& mainStage = UIStage::GetInstance();
	mainStage.Initiazlize();

	// initialize lua
	Logger::Info("Initialize Lua context");
	mLuaContext = std::unique_ptr<LuaContext>(new LuaContext(*this));
	mLuaContext->Initialize();
}

App::~App()
{
	if (mCurrGame != nullptr)
	{
		mCurrGame->Stop();
		mCurrGame.reset();
	}
	if (mLuaContext != nullptr)
	{
		mLuaContext->Exit();
	}

	UIStage::GetInstance().Quit();
	System::Quit();
	FileData::CloseData();
}

/**
*	\brief 主循环函数

*	在该函数中根据固定的频率循环刷新
*/
void App::Run()
{
	Logger::Info("Simulation started");

	uint32_t lastFrameDate = System::RealTime();
	uint32_t lag = 0;
	uint32_t timeDropped = 0;

	while (!IsExiting())
	{
		uint32_t now = System::RealTime() - timeDropped;
		uint32_t lastFrameDuration = now - lastFrameDate;
		lastFrameDate = now;
		lag += lastFrameDuration;

		// 计算帧
		if (lastFrameDuration > 0)
		{
			Video::SetFrameDelat(lastFrameDuration);
			//std::cout << Video::GetFPS() << std::endl;
		}
		// 有可能因为一些情况（比如加载大文件)导致这一帧时间
		// 特别长，则放弃追回，将时间记录在TimeDropped
		if (lag >= 300)
		{
			timeDropped += lag - System::timeStep;
			lag = System::timeStep;
			lastFrameDate = System::RealTime() - timeDropped;
		}

		// 1.check input
		glfwPollEvents();
		CheckInput();

		// 2.update	对于慢的机器，需要循环多次而跳过一些绘制过程
		int updates = 0;
		while (lag >= System::timeStep && updates < 10 && !IsExiting())
		{
			Step();
			lag -= System::timeStep;
			++updates;
		}

		// 3.render
		if(updates > 0)
			Render();

		// 4.sleep
		lastFrameDuration = (System::RealTime() - timeDropped) - lastFrameDate;
		if (lastFrameDuration < System::timeStep)
			System::Sleeps((System::timeStep - lastFrameDuration));
	}

	Logger::Info("Simulation finished");
}

/**
*	\brief 步骤用于控制update过程
*/
void App::Step()
{
	Update();
}

void App::Update()
{
	// game update
	if (mCurrGame != nullptr)
	{
		mCurrGame->Update();
	}
	mLuaContext->Update();
	System::Update();

	if (mNextGame != mCurrGame.get())
	{
		// 切换当前game
		mCurrGame = std::unique_ptr<Game>(mNextGame);
		if (mCurrGame != nullptr)
		{
			mCurrGame->Start();
		}
		else
		{
			mLuaContext->Exit();
		}
	}

	// ui update
	UIStage::GetInstance().Update();
}

void App::SetExiting(bool isexit)
{
	mExiting = isexit;
}

bool App::IsExiting()
{
	return mExiting || Video::IsExiting();
}

/**
*	\brief 获取inputEvent
*/
void App::CheckInput()
{
	std::unique_ptr<InputEvent> ent = InputEvent::GetEvent();
	while (ent != nullptr)
	{
		NotifyInput(*ent);
		ent = InputEvent::GetEvent();
	}
}

/**
*	\brief 执行绘制操作
*/
void App::Render()
{
	Video::CleanCanvas();

	//sprite->MultiplyDraw(Point2(0, 0), 0.0f);
	//sprite_2->Draw();
	//Renderer::GetInstance().PushLight(mLight);
	//mLight->PushObstacles({ Rect(200,200, 200, 100) });
	//mLight->Update();
	if (mCurrGame != nullptr)
	{
		mCurrGame->Draw();
	}

	mLuaContext->Draw();
	mLuaContext->OnMainDraw();

	Video::Rendercanvas(UIStage::GetInstance());
}

/**
*	\brief 响应inputEvent，将event分发给外部
*/
void App::NotifyInput(const InputEvent & ent)
{
	if (ent.IsWindowClosing())
		SetExiting(true);

	if (!UIStage::GetInstance().NotifyInput(ent))
		return;

	bool handle = mLuaContext->NotifyInput(ent);
	if (!handle && mCurrGame != nullptr)
	{
		mCurrGame->NotifyInput(ent);
	}
}

/**
*	\brief 设置当前游戏
*   \param game 当game为nullptr则退出游戏
*/
void App::SetGame(Game* game)
{
	mNextGame = game;
}

Game* App::GetGame()
{
	return mCurrGame.get();
}

LuaContext & App::GetLuaContext()
{
	return *mLuaContext;
}


