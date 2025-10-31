#pragma once
#include <cstddef>
#include <functional>
#include <unordered_map>
#include <vector>

struct EventBus
{
	/**
	 * @brief Type alias for an event subscriber.
	 *
	 * A subscriber is a callable (e.g., lambda or function) that receives
	 * a constant reference to an event of type @p Event.
	 */
	template <class Event>
	using Subscriber = std::function<void(const Event&)>;

	/**
	 * @brief Type alias for a generic, type-erased subscriber.
	 *
	 * This is used internally to store subscribers without knowing
	 * their concrete event type.
	 */
	using GenericSubscriber = std::function<void(const void*)>;

	/**
	 * @brief Unique identifier for a subscription instance.
	 */
	using SubscriptionHandle = std::size_t;

	/**
	 * @brief Unique identifier for an event type.
	 */
	using EventId = std::size_t;

	/**
	 * @brief Retrieves a unique identifier for the given event type.
	 *
	 * @tparam Event Type of the event.
	 * @return EventId A hash code uniquely identifying the event type.
	 */
	template <class Event>
	static EventId getId()
	{
		return typeid(Event).hash_code();
	}

	/**
	 * @brief Registers a generic subscriber to a specific event type.
	 *
	 * @param subscriber The type-erased subscriber function.
	 * @param handle The unique subscription handle.
	 * @param eventId The ID of the event type to subscribe to.
	 */
	void subscribe(GenericSubscriber subscriber, SubscriptionHandle handle, EventId eventId);

	/**
	 * @brief Registers a subscriber to a specific event type.
	 *
	 * Wraps the typed subscriber into a type-erased one and delegates
	 * to the generic subscribe function.
	 *
	 * @tparam Event The type of the event to subscribe to.
	 * @param subscriber The subscriber function to call when the event is published.
	 * @param handle The unique subscription handle for this subscriber.
	 */
	template <class Event>
	void subscribe(Subscriber<Event> subscriber, SubscriptionHandle handle)
	{
		subscribe([subscriber](const void* eventData)
		{
			subscriber(*static_cast<const Event*>(eventData));
		}, handle, getId<Event>());
	}

	/**
	 * @brief Registers a subscriber and automatically generates its handle.
	 *
	 * @tparam Event The type of the event to subscribe to.
	 * @param subscriber The subscriber function to call when the event is published.
	 * @return SubscriptionHandle The generated handle for this subscription.
	 */
	template <class Event>
	SubscriptionHandle subscribe(Subscriber<Event> subscriber)
	{
		SubscriptionHandle handle = typeid(subscriber).hash_code();
		subscribe<Event>(subscriber, handle);
		return handle;
	}

	/**
	 * @brief Unsubscribes a subscriber from a specific event type.
	 *
	 * @param handle The subscription handle to remove.
	 * @param eventId The ID of the event type to unsubscribe from.
	 */
	void unsubscribe(SubscriptionHandle handle, EventId eventId);

	/**
	 * @brief Unsubscribes a subscriber from the specified event type.
	 *
	 * @tparam Event The event type to unsubscribe from.
	 * @param handle The subscription handle to remove.
	 */
	template <class Event>
	void unsubscribe(SubscriptionHandle handle)
	{
		unsubscribe(handle, getId<Event>());
	}

	/**
	 * @brief Publishes an event to all subscribers of the specified event type.
	 *
	 * @param eventId The ID of the event type.
	 * @param eventData Pointer to the event data (type-erased).
	 */
	void publish(EventId eventId, const void* eventData);

	/**
	 * @brief Publishes an event to all subscribers.
	 *
	 * @tparam Event The type of the event to publish.
	 * @param event The event instance to send to all subscribers.
	 */
	template <class Event>
	void publish(const Event& event)
	{
		publish(getId<Event>(), &event);
	}

private:

	struct Subscription
	{
		GenericSubscriber subscriber;
		SubscriptionHandle handle;
	};

	std::unordered_map<EventId, std::vector<Subscription>> subscriptions_;
};
