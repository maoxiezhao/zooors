#pragma once

// gui core/8.8.2017 zy

#include<functional>
#include<queue>
#include<list>
#include<map>

#include"common\common.h"
#include"core\inputEvent.h"
#include"utils\point.h"
#include"utils\size.h"
#include"gui\core\handler.h"
#include"gui\core\message.h"

namespace gui
{

class Widget;
class Dispatcher;

/** 事件对应的回调函数 */
using SignalFunction = std::function<void(Dispatcher& dispatcher, 
										const ui_event event, 
										bool&handle, 
										bool& halt)>;

using SignalFunctionMouse = std::function<void(Dispatcher& dispatcher, 
												const ui_event event, 
												bool&handle,
												bool& halt ,
												const Point2& coords)>;

using SignalFunctionKeyboard = std::function<void(Dispatcher& dispatcher,
												const ui_event event,
												bool& handle,
												bool& halt,
												const InputEvent::KeyboardKey key,
												const string & unicode)>;

using SignalFunctionMessage = std::function<void(Dispatcher& dispatcher,
												const ui_event event,
												bool& handle,
												bool& halt,
												Message& message)>;

using SignalFunctionNotification = std::function<void(Dispatcher& dispatcher,
												const ui_event event,
												bool& handle,
												bool& halt,
											    void*)>;
/**
*	\brief widget的基类
*/
class Dispatcher
{
	friend struct Dispatcher_implementation;
public:
	Dispatcher();
	~Dispatcher();

	void Connect();

	virtual bool IsAt(const Point2& pos)const;

	/** 鼠标处理行为,决定该dispathcer对鼠标event的响应行为*/
	enum mouse_behavior
	{
		all,
		hit,
		none,
	};

	void CaptureMouse()
	{
		gui::CaptureMouse(this);
	}

	void ReleaseMouse()
	{
		gui::ReleaseMouse(this);
	}

	/******  set/get *******/

	bool GetWantKeyboard()const
	{
		return mWantKeyboard;
	}
	void SetWantKeyboard(bool wantKeyboard)
	{
		mWantKeyboard = wantKeyboard;
	}

	mouse_behavior GetMouseBehavior()const
	{
		return  mMouseBehavior;
	}

	void SetMouseBehavior(const mouse_behavior mouseBehavior)
	{
		mMouseBehavior = mouseBehavior;
	}

	/******  触发各类事件 *******/

	/** 通用事件 */
	bool Fire(const ui_event event, Widget& widget);

	/** 鼠标事件 */
	bool Fire(const ui_event event, Widget& widget, const Point2& pos);

	/** 键盘事件 */
	bool Fire(const ui_event event, Widget& widget, const InputEvent::KeyboardKey key, const string & unicode);

	/** 消息事件 */
	bool Fire(const ui_event event, Widget& widget, Message& message);

	/** 通知响应事件 */
	bool Fire(const ui_event event, Widget& widget, void*data);

	/**
	*	\brief 信号添加到信号队列中的位置
	*/
	enum queue_position
	{
		front_pre_child,
		back_pre_child,
		front_child,
		back_child,
		front_post_child,
		back_post_child
	};

	/**
	*	\brief 信号队列类型
	*/
	enum event_queue_type
	{
		pre = 1,
		child = 2,
		post = 4
	};

	/** 对应信号的链接函数 */

	/**
	*	\brief set event 信号处理
	*/
	template<ui_event E>
	std::enable_if_t<util::typeset_exist<setEvent, int_<E> >::value>
	ConnectSignal(const SignalFunction& signal, const queue_position position = back_child)
	{
		mQueueSignal.ConnectSignal(E, position, signal);
	}

	template<ui_event E>
	std::enable_if_t<util::typeset_exist<setEvent, int_<E> >::value>
	DisconnectSignal(const SignalFunction& signal, const queue_position position = back_child)
	{
		mQueueSignal.DisconnectSignal(E, position, signal);
	}
	/**
	*	\brief mouse event 信号处理
	*/
	template<ui_event E>
	std::enable_if_t<util::typeset_exist<setEventMouse, int_<E> >::value>
		ConnectSignal(const SignalFunctionMouse& signal, const queue_position position = back_child)
	{
		mQueueSignalMouse.ConnectSignal(E, position, signal);
	}

	template<ui_event E>
	std::enable_if_t<util::typeset_exist<setEventMouse, int_<E> >::value>
		DisconnectSignal(const SignalFunctionMouse& signal, const queue_position position = back_child)
	{
		mQueueSignalMouse.DisconnectSignal(E, position, signal);
	}

	template<typename T>
	std::enable_if_t<std::is_same<setEventMouse, T>::value, bool>
		HasSignal( event_queue_type type = event_queue_type::child)
	{
		return mQueueSignalMouse.HasSignal(type);
	}

	/**
	*	\brief keyborad event 信号处理
	*/
	template<ui_event E>
	std::enable_if_t<util::typeset_exist<setEventKeyboard, int_<E> >::value>
		ConnectSignal(const SignalFunctionKeyboard& signal, const queue_position position = back_child)
	{
		mQueueSignalKeyboard.ConnectSignal(E, position, signal);
	}

	template<ui_event E>
	std::enable_if_t<util::typeset_exist<setEventKeyboard, int_<E> >::value>
		DisconnectSignal(const SignalFunctionKeyboard& signal, const queue_position position = back_child)
	{
		mQueueSignalKeyboard.DisconnectSignal(E, position, signal);
	}

	template<typename T>
	std::enable_if_t<std::is_same<setEventKeyboard, T>::value, bool>
		HasSignal( event_queue_type type = event_queue_type::child)
	{
		return mQueueSignalKeyboard.HasSignal(type);
	}

	/**
	*	\brief message event 信号处理
	*/
	template<ui_event E>
	std::enable_if_t<util::typeset_exist<setEventMessage, int_<E> >::value>
	ConnectSignal(const SignalFunctionMessage& signal, const queue_position position = back_child)
	{
		mQueueSignalMessage.ConnectSignal(E, position, signal);
	}
	template<ui_event E>
	std::enable_if_t<util::typeset_exist<setEventMessage, int_<E> >::value>
	DisconnectSignal(const SignalFunctionMessage& signal, const queue_position position = back_child)
	{
		mQueueSignalMessage.DisconnectSignal(E, position, signal);
	}

	/**
	*	\brief notification event 信号处理
	*/
	template<ui_event E>
	std::enable_if_t<util::typeset_exist<setEventNotification, int_<E> >::value>
		ConnectSignal(const SignalFunctionNotification& signal, const queue_position position = back_child)
	{
		mQueueSignalNotification.ConnectSignal(E, position, signal);
	}
	template<ui_event E>
	std::enable_if_t<util::typeset_exist<setEventNotification, int_<E> >::value>
		DisconnectSignal(const SignalFunctionNotification& signal, const queue_position position = back_child)
	{
		mQueueSignalNotification.DisconnectSignal(E, position, signal);
	}

	/**
	*	\brief 信号类型
	*
	*	信号类型保存有该信号类型的3个回调队列
	*/
	template<typename T>
	struct SignalType
	{
		SignalType() :mPreChild(), mChild(), mPostChild(){}

		std::list<T> mPreChild;
		std::list<T> mChild;
		std::list<T> mPostChild;
	};

	/**
	*	\brief 信号类型的事件队列
	*
	*	存储一个信号类型对所有事件的回队列
	*/
	template<typename T>
	struct SignalQueue
	{
		SignalQueue() :mQueue()
		{
		}

		std::map<ui_event, SignalType<T> > mQueue;

		/** 添加一个对应信号的回调 */
		void ConnectSignal(const ui_event event, const queue_position position, const T& signal)
		{
			switch (position)
			{
			case front_pre_child:
				mQueue[event].mPreChild.push_front(signal);
				break;
			case back_pre_child:
				mQueue[event].mPreChild.push_back(signal);
				break;
			case front_child:
				mQueue[event].mChild.push_front(signal);
				break;
			case back_child:
				mQueue[event].mChild.push_back(signal);
				break;
			case front_post_child:
				mQueue[event].mPostChild.push_front(signal);
				break;
			case back_post_child:
				mQueue[event].mPostChild.push_back(signal);
				break;
			}
		}

		/** 删除一个对应信号的回调 */
		void DisconnectSignal(const ui_event event, const queue_position position, const T& singal)
		{
			switch (position)
			{
			case front_pre_child:
			case back_pre_child:
				mQueue[event].mPreChild.remove_if(
					[&singal](T& element){return singal.target_type() == element.target_type(); });
				break;
			case front_child:
			case back_child:
				mQueue[event].mChild.remove_if(
					[&singal](T& element){return singal.target_type() == element.target_type(); });
				break;
			case front_post_child:
			case back_post_child:
				mQueue[event].mPostChild.remove_if(
					[&singal](T& element){return singal.target_type() == element.target_type(); });
				break;
			}
		}

		/** 判断是否存在信号 */
		bool HasSignal(event_queue_type type)
		{
			bool result = false;
			switch (type)
			{
			case gui::Dispatcher::pre:
				for (auto& kvp : mQueue)
				{
					auto& eventList = kvp.second;
					if (!eventList.mPreChild.empty())
					{
						result = true;
						break;
					}
				}
				break;
			case gui::Dispatcher::child:
				for (auto& kvp : mQueue)
				{
					auto& eventList = kvp.second;
					if (!eventList.mChild.empty())
					{
						result = true;
						break;
					}
				}
				break;
			case gui::Dispatcher::post:
				for (auto& kvp : mQueue)
				{
					auto& eventList = kvp.second;
					if (!eventList.mPostChild.empty())
					{
						result = true;
						break;
					}
				}
				break;
			}
			return result;
		}
	};

	bool HasEvent(const ui_event event, const event_queue_type type);
private:
	bool mIsConnected;

	bool mWantKeyboard;

	mouse_behavior mMouseBehavior;

	/** 对应信号的事件队列 */
	SignalQueue<SignalFunction> mQueueSignal;

	SignalQueue<SignalFunctionMouse> mQueueSignalMouse;

	SignalQueue<SignalFunctionKeyboard> mQueueSignalKeyboard;

	SignalQueue<SignalFunctionMessage> mQueueSignalMessage;

	SignalQueue<SignalFunctionNotification> mQueueSignalNotification;
};


inline void ConnectSignalHandleNotifyModified(Dispatcher& dispatcher, const SignalFunctionNotification& signal)
{
	dispatcher.ConnectSignal<ui_event::UI_EVENT_NOTIFY_MODIFIED>(signal);
}

}
