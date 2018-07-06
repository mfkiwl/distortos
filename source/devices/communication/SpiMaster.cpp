/**
 * \file
 * \brief SpiMaster class implementation
 *
 * \author Copyright (C) 2016-2018 Kamil Szczygiel http://www.distortec.com http://www.freddiechopin.info
 *
 * \par License
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "distortos/devices/communication/SpiMaster.hpp"

#include "distortos/devices/communication/SpiDevice.hpp"
#include "distortos/devices/communication/SpiDeviceProxy.hpp"
#include "distortos/devices/communication/SpiDeviceSelectGuard.hpp"
#include "distortos/devices/communication/SpiMasterLowLevel.hpp"
#include "distortos/devices/communication/SpiMasterOperation.hpp"
#include "distortos/devices/communication/SpiMasterProxy.hpp"

#include "distortos/internal/CHECK_FUNCTION_CONTEXT.hpp"

#include "distortos/assert.h"
#include "distortos/Semaphore.hpp"

#include <mutex>

#include <cerrno>

namespace distortos
{

namespace devices
{

/*---------------------------------------------------------------------------------------------------------------------+
| public functions
+---------------------------------------------------------------------------------------------------------------------*/

SpiMaster::~SpiMaster()
{
	CHECK_FUNCTION_CONTEXT();

	if (openCount_ == 0)
		return;

	const std::lock_guard<distortos::Mutex> lockGuard {mutex_};

	spiMaster_.stop();
}

int SpiMaster::close()
{
	const std::lock_guard<distortos::Mutex> lockGuard {mutex_};

	if (openCount_ == 0)	// device is not open anymore?
		return EBADF;

	if (openCount_ == 1)	// last close?
	{
		const auto ret = spiMaster_.stop();
		if (ret != 0)
			return ret;
	}

	--openCount_;
	return 0;
}

std::pair<int, size_t> SpiMaster::executeTransaction(SpiDevice& device, const SpiMasterOperationsRange operationsRange)
{
	CHECK_FUNCTION_CONTEXT();

	if (operationsRange.size() == 0)
		return {EINVAL, {}};

	const SpiDevice::Proxy spiDeviceProxy {device};
	Proxy proxy {spiDeviceProxy};

	{
		const auto ret = proxy.configure(device.getMode(), device.getMaxClockFrequency(), device.getWordLength(),
				device.getLsbFirst());
		if (ret.first != 0)
			return {ret.first, {}};
	}

	const SpiDeviceSelectGuard spiDeviceSelectGuard {proxy};

	return proxy.executeTransaction(operationsRange);
}

int SpiMaster::open()
{
	const std::lock_guard<distortos::Mutex> lockGuard {mutex_};

	if (openCount_ == std::numeric_limits<decltype(openCount_)>::max())	// device is already opened too many times?
		return EMFILE;

	if (openCount_ == 0)	// first open?
	{
		const auto ret = spiMaster_.start();
		if (ret != 0)
			return ret;
	}

	++openCount_;
	return 0;
}

/*---------------------------------------------------------------------------------------------------------------------+
| private functions
+---------------------------------------------------------------------------------------------------------------------*/

void SpiMaster::notifyWaiter(const int ret)
{
	ret_ = ret;
	const auto semaphore = semaphore_;
	assert(semaphore != nullptr);
	semaphore->post();
}

void SpiMaster::transferCompleteEvent(SpiMasterErrorSet errorSet, size_t bytesTransfered)
{
	assert(operationsRange_.size() != 0 && "Invalid range of operations!");

	{
		const auto previousTransfer = operationsRange_.begin()->getTransfer();
		assert(previousTransfer != nullptr && "Invalid type of previous operation!");
		previousTransfer->finalize(errorSet, bytesTransfered);
	}

	const auto error = errorSet.any();
	if (error == false)	// handling of last operation successful?
		operationsRange_ = {operationsRange_.begin() + 1, operationsRange_.end()};

	if (operationsRange_.size() == 0 || error == true)	// all operations are done or handling of last one failed?
	{
		notifyWaiter(error == false ? 0 : EIO);
		return;
	}

	{
		const auto nextTransfer = operationsRange_.begin()->getTransfer();
		assert(nextTransfer != nullptr && "Invalid type of next operation!");
		const auto ret = spiMaster_.startTransfer(*this, nextTransfer->getWriteBuffer(), nextTransfer->getReadBuffer(),
				nextTransfer->getSize());
		if (ret != 0)
			notifyWaiter(ret);
	}
}

}	// namespace devices

}	// namespace distortos
