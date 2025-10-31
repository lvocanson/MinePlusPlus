#include "EventBus.h"

void EventBus::subscribe(GenericSubscriber subscriber, SubscriptionHandle handle, EventId eventId)
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

void EventBus::unsubscribe(SubscriptionHandle handle, EventId eventId)
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

void EventBus::publish(EventId eventId, const void* eventData)
{
	auto& subs = subscriptions_[eventId];
	for (auto& sub : subs)
	{
		sub.subscriber(eventData);
	}
}
