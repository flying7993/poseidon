// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014 - 2017, LH_Mouse. All wrongs reserved.

#ifndef POSEIDON_SINGLETONS_EVENT_DISPATCHER_HPP_
#define POSEIDON_SINGLETONS_EVENT_DISPATCHER_HPP_

#include "../cxx_ver.hpp"
#include <typeinfo>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/utility/enable_if.hpp>
#include <vector>
#include "../exception.hpp"
#include "../log.hpp"

namespace Poseidon {

class EventBase;
class EventListener;

typedef boost::function<
	void (const boost::shared_ptr<EventBase> &event)
	> EventListenerCallback;

class EventDispatcher {
private:
	EventDispatcher();

public:
	static void start();
	static void stop();

	// 返回的 shared_ptr 是该响应器的唯一持有者。
	static boost::shared_ptr<EventListener> register_listener_explicit(const std::type_info &type_info, EventListenerCallback callback);

	template<typename EventT>
	static boost::shared_ptr<EventListener> register_listener(boost::function<void (const boost::shared_ptr<EventT> &)> callback){
		struct Helper {
			static void safe_fwd(boost::function<void (const boost::shared_ptr<EventT> &)> &callback, const boost::shared_ptr<EventBase> &event){
				AUTO(derived, boost::dynamic_pointer_cast<EventT>(event));
				if(!derived){
					LOG_POSEIDON_ERROR("Incorrect dynamic event type: expecting ", typeid(EventT).name(), ", got ", typeid(*event).name());
					DEBUG_THROW(Exception, sslit("Incorrect dynamic event type"));
				}
				callback(STD_MOVE(derived));
			}
		};
		return register_listener_explicit(typeid(EventT), boost::bind(&Helper::safe_fwd, STD_MOVE_IDN(callback), _1));
	}

	static void sync_raise(const boost::shared_ptr<EventBase> &event);
	static void async_raise(const boost::shared_ptr<EventBase> &event, const boost::shared_ptr<const bool> &withdrawn);
};

}

#endif
