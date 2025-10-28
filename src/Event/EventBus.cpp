#include "EventBus.h"
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace EventBus
{

namespace
{

struct Subscription
{
	GenericSubscriber subscriber;
	SubscriptionHandle handle;
};

std::unordered_map<EventId, std::vector<Subscription>> subscriptions_;

}

void subscribe(GenericSubscriber subscriber, SubscriptionHandle handle, EventId eventId)
{
	auto& subs = subscriptions_[eventId];
	for (auto& sub : subs)
	{
		if (sub.handle == handle)
		{
			sub.subscriber = subscriber;
			return;
		}
	}
	subs.emplace_back(subscriber, handle);
}

void unsubscribe(SubscriptionHandle handle, EventId eventId)
{
	auto& subs = subscriptions_[eventId];
	for (auto it = subs.begin(); it != subs.end(); ++it)
	{
		if (it->handle == handle)
		{
			subs.erase(it);
			return;
		}
	}
}

void publish(EventId eventId, const void* eventData)
{
	auto& subs = subscriptions_[eventId];
	for (auto& sub : subs)
	{
		sub.subscriber(eventData);
	}
}

}

