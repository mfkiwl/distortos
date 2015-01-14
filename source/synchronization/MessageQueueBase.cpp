/**
 * \file
 * \brief MessageQueueBase class implementation
 *
 * \author Copyright (C) 2015 Kamil Szczygiel http://www.distortec.com http://www.freddiechopin.info
 *
 * \par License
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \date 2015-01-14
 */

#include "distortos/synchronization/MessageQueueBase.hpp"

#include "distortos/architecture/InterruptMaskingLock.hpp"

namespace distortos
{

namespace synchronization
{

namespace
{

/*---------------------------------------------------------------------------------------------------------------------+
| local types
+---------------------------------------------------------------------------------------------------------------------*/

/// PopInternalFunctor class is a MessageQueueBase::InternalFunctor used for popping of elements from the queue
class PopInternalFunctor : public MessageQueueBase::InternalFunctor
{
public:

	/**
	 * \brief PopInternalFunctor's constructor
	 *
	 * \param [out] priority is a reference to variable that will be used to return priority of popped value
	 * \param [in] functor is a reference to Functor which will execute actions related to popping - it will get a
	 * pointer to storage with element
	 */

	constexpr PopInternalFunctor(uint8_t& priority, const MessageQueueBase::Functor& functor) :
			priority_(priority),
			functor_(functor)
	{

	}

	/**
	 * \brief PushInternalFunctor's function call operator
	 *
	 * Pops oldest entry with highest priority from \a entryList, passes the storage to \a functor_ and pushes this (now
	 * free) entry to \a freeEntryList.
	 *
	 * \param [in] entryList is a reference to EntryList of MessageQueueBase
	 * \param [in] freeEntryList is a reference to FreeEntryList of MessageQueueBase
	 */

	virtual void operator()(MessageQueueBase::EntryList& entryList, MessageQueueBase::FreeEntryList& freeEntryList)
			const override
	{
		const auto entry = *entryList.begin();
		entryList.pop_front();

		priority_ = entry.priority;

		functor_(entry.storage);

		freeEntryList.emplace_front(entry);
	}

private:

	/// reference to variable that will be used to return priority of popped value
	uint8_t& priority_;

	/// reference to Functor which will execute actions related to popping
	const MessageQueueBase::Functor& functor_;
};

/// PushInternalFunctor class is a MessageQueueBase::InternalFunctor used for pushing of elements to the queue
class PushInternalFunctor : public MessageQueueBase::InternalFunctor
{
public:

	/**
	 * \brief PushInternalFunctor's constructor
	 *
	 * \param [in] priority is the priority of new element
	 * \param [in] functor is a reference to Functor which will execute actions related to pushing - it will get a
	 * pointer to storage for element
	 */

	constexpr PushInternalFunctor(const uint8_t priority, const MessageQueueBase::Functor& functor) :
			functor_(functor),
			priority_{priority}
	{

	}

	/**
	 * \brief PushInternalFunctor's function call operator
	 *
	 * Pops one entry from \a freeEntryList, passes the storage to \a functor_ and pushes this entry to \a entryList.
	 *
	 * \param [in] entryList is a reference to EntryList of MessageQueueBase
	 * \param [in] freeEntryList is a reference to FreeEntryList of MessageQueueBase
	 */

	virtual void operator()(MessageQueueBase::EntryList& entryList, MessageQueueBase::FreeEntryList& freeEntryList)
			const override
	{
		auto entry = *freeEntryList.begin();
		freeEntryList.pop_front();

		entry.priority = priority_;

		functor_(entry.storage);

		entryList.sortedEmplace(entry);
	}

private:

	/// reference to Functor which will execute actions related to pushing
	const MessageQueueBase::Functor& functor_;

	/// priority of new element
	const uint8_t priority_;
};

}	// namespace

/*---------------------------------------------------------------------------------------------------------------------+
| public functions
+---------------------------------------------------------------------------------------------------------------------*/

int MessageQueueBase::pop(const SemaphoreFunctor& waitSemaphoreFunctor, uint8_t& priority, const Functor& functor)
{
	const PopInternalFunctor popInternalFunctor {priority, functor};
	return popPush(waitSemaphoreFunctor, popInternalFunctor, popSemaphore_, pushSemaphore_);
}

int MessageQueueBase::push(const SemaphoreFunctor& waitSemaphoreFunctor, const uint8_t priority, const Functor& functor)
{
	const PushInternalFunctor pushInternalFunctor {priority, functor};
	return popPush(waitSemaphoreFunctor, pushInternalFunctor, pushSemaphore_, popSemaphore_);
}

/*---------------------------------------------------------------------------------------------------------------------+
| private functions
+---------------------------------------------------------------------------------------------------------------------*/

MessageQueueBase::MessageQueueBase(const size_t maxElements) :
		popSemaphore_{0, maxElements},
		pushSemaphore_{maxElements, maxElements},
		pool_{},
		poolAllocator_{pool_},
		entryList_{poolAllocator_},
		freeEntryList_{poolAllocator_}
{

}

int MessageQueueBase::popPush(const SemaphoreFunctor& waitSemaphoreFunctor, const InternalFunctor& internalFunctor,
		Semaphore& waitSemaphore, Semaphore& postSemaphore)
{
	architecture::InterruptMaskingLock interruptMaskingLock;

	const auto ret = waitSemaphoreFunctor(waitSemaphore);
	if (ret != 0)
		return ret;

	internalFunctor(entryList_, freeEntryList_);

	return postSemaphore.post();
}

}	// namespace synchronization

}	// namespace distortos
