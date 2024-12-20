/**
 * \file
 * \brief ChipUartLowLevel class implementation for USARTv2 in STM32
 *
 * \author Copyright (C) 2016-2024 Kamil Szczygiel https://distortec.com https://freddiechopin.info
 *
 * \par License
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "distortos/chip/ChipUartLowLevel.hpp"

#include "distortos/chip/getBusFrequency.hpp"
#include "distortos/chip/STM32-bit-banding.h"

#include "distortos/devices/communication/UartBase.hpp"

#ifndef DISTORTOS_BITBANDING_SUPPORTED

#include "distortos/InterruptMaskingLock.hpp"

#endif	// !def DISTORTOS_BITBANDING_SUPPORTED

#include <cerrno>

#if !defined(USART_CR1_M0)
#define USART_CR1_M0						USART_CR1_M
#endif	// !defined(USART_CR1_M0)
#if !defined(USART_CR1_M0_Pos)
#define USART_CR1_M0_Pos					__builtin_ctzl(USART_CR1_M0)
#endif	// !defined(USART_CR1_M0_Pos)
#if defined(DISTORTOS_CHIP_USART_HAS_CR1_M1_BIT) && !defined(USART_CR1_M1_Pos)
#define USART_CR1_M1_Pos					__builtin_ctzl(USART_CR1_M1)
#endif	// defined(DISTORTOS_CHIP_USART_HAS_CR1_M1_BIT) && !defined(USART_CR1_M1_Pos)
#if !defined(USART_CR1_RXNEIE) && defined(USART_CR1_RXNEIE_RXFNEIE)
#define USART_CR1_RXNEIE					USART_CR1_RXNEIE_RXFNEIE
#endif	// !defined(USART_CR1_RXNEIE) && define(USART_CR1_RXNEIE_RXFNEIE)
#if !defined(USART_CR1_TXEIE) && defined(USART_CR1_TXEIE_TXFNFIE)
#define USART_CR1_TXEIE						USART_CR1_TXEIE_TXFNFIE
#endif	// !defined(USART_CR1_TXEIE) && defined(USART_CR1_TXEIE_TXFNFIE)
#if !defined(USART_ISR_RXNE) && defined(USART_ISR_RXNE_RXFNE)
#define USART_ISR_RXNE						USART_ISR_RXNE_RXFNE
#endif	// !defined(USART_ISR_RXNE) && defined(USART_ISR_RXNE_RXFNE)
#if !defined(USART_ISR_TXE) && defined(USART_ISR_TXE_TXFNF)
#define USART_ISR_TXE						USART_ISR_TXE_TXFNF
#endif	// !defined(USART_ISR_TXE) && defined(USART_ISR_TXE_TXFNF)
#if !defined(USART_BRR_DIV_FRACTION_Pos)
#define USART_BRR_DIV_FRACTION_Pos			0
#endif	// !defined(USART_BRR_DIV_FRACTION_Pos)
#if !defined(USART_BRR_DIV_MANTISSA_Pos)
#define USART_BRR_DIV_MANTISSA_Pos			4
#endif	// !defined(USART_BRR_DIV_MANTISSA_Pos)
#if !defined(USART_BRR_DIV_MANTISSA)
#define USART_BRR_DIV_MANTISSA				(0xFFF << USART_BRR_DIV_MANTISSA_Pos)
#endif	// !defined(USART_BRR_DIV_MANTISSA)

namespace distortos
{

namespace chip
{

namespace
{

/*---------------------------------------------------------------------------------------------------------------------+
| local functions
+---------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Decode value of USART_ISR register to devices::UartBase::ErrorSet
 *
 * \param [in] isr is the value of USART_ISR register that will be decoded
 *
 * \return devices::UartBase::ErrorSet with errors decoded from \a isr
 */

devices::UartBase::ErrorSet decodeErrors(const uint32_t isr)
{
	devices::UartBase::ErrorSet errorSet {};
	errorSet[devices::UartBase::framingError] = (isr & USART_ISR_FE) != 0;
	errorSet[devices::UartBase::noiseError] = (isr & USART_ISR_NE) != 0;
	errorSet[devices::UartBase::overrunError] = (isr & USART_ISR_ORE) != 0;
	errorSet[devices::UartBase::parityError] = (isr & USART_ISR_PE) != 0;
	return errorSet;
}

/*---------------------------------------------------------------------------------------------------------------------+
| local objects
+---------------------------------------------------------------------------------------------------------------------*/

#ifdef DISTORTOS_CHIP_LPUART1_ENABLE
/// true if LPUART1 has RX/TX FIFO, false otherwise
#ifdef DISTORTOS_CHIP_LPUART1_HAS_FIFO
constexpr bool lpuart1HasFifo {true};
#else	// !def DISTORTOS_CHIP_LPUART1_HAS_FIFO
constexpr bool lpuart1HasFifo {false};
#endif	// !def DISTORTOS_CHIP_LPUART1_HAS_FIFO
#endif	// def DISTORTOS_CHIP_LPUART1_ENABLE

#ifdef DISTORTOS_CHIP_LPUART2_ENABLE
/// true if LPUART2 has RX/TX FIFO, false otherwise
#ifdef DISTORTOS_CHIP_LPUART2_HAS_FIFO
constexpr bool lpuart2HasFifo {true};
#else	// !def DISTORTOS_CHIP_LPUART2_HAS_FIFO
constexpr bool lpuart2HasFifo {false};
#endif	// !def DISTORTOS_CHIP_LPUART2_HAS_FIFO
#endif	// def DISTORTOS_CHIP_LPUART2_ENABLE

#ifdef DISTORTOS_CHIP_USART1_ENABLE
/// true if USART1 has RX/TX FIFO, false otherwise
#ifdef DISTORTOS_CHIP_USART1_HAS_FIFO
constexpr bool usart1HasFifo {true};
#else	// !def DISTORTOS_CHIP_USART1_HAS_FIFO
constexpr bool usart1HasFifo {false};
#endif	// !def DISTORTOS_CHIP_USART1_HAS_FIFO
#endif	// def DISTORTOS_CHIP_USART1_ENABLE

#ifdef DISTORTOS_CHIP_USART2_ENABLE
/// true if USART2 has RX/TX FIFO, false otherwise
#ifdef DISTORTOS_CHIP_USART2_HAS_FIFO
constexpr bool usart2HasFifo {true};
#else	// !def DISTORTOS_CHIP_USART2_HAS_FIFO
constexpr bool usart2HasFifo {false};
#endif	// !def DISTORTOS_CHIP_USART2_HAS_FIFO
#endif	// def DISTORTOS_CHIP_USART2_ENABLE

#ifdef DISTORTOS_CHIP_USART3_ENABLE
/// true if USART3 has RX/TX FIFO, false otherwise
#ifdef DISTORTOS_CHIP_USART3_HAS_FIFO
constexpr bool usart3HasFifo {true};
#else	// !def DISTORTOS_CHIP_USART3_HAS_FIFO
constexpr bool usart3HasFifo {false};
#endif	// !def DISTORTOS_CHIP_USART3_HAS_FIFO
#endif	// def DISTORTOS_CHIP_USART3_ENABLE

#ifdef DISTORTOS_CHIP_UART4_ENABLE
/// true if UART4 has RX/TX FIFO, false otherwise
#ifdef DISTORTOS_CHIP_UART4_HAS_FIFO
constexpr bool uart4HasFifo {true};
#else	// !def DISTORTOS_CHIP_UART4_HAS_FIFO
constexpr bool uart4HasFifo {false};
#endif	// !def DISTORTOS_CHIP_UART4_HAS_FIFO
#endif	// def DISTORTOS_CHIP_UART4_ENABLE

#ifdef DISTORTOS_CHIP_USART4_ENABLE
/// true if USART4 has RX/TX FIFO, false otherwise
#ifdef DISTORTOS_CHIP_USART4_HAS_FIFO
constexpr bool usart4HasFifo {true};
#else	// !def DISTORTOS_CHIP_USART4_HAS_FIFO
constexpr bool usart4HasFifo {false};
#endif	// !def DISTORTOS_CHIP_USART4_HAS_FIFO
#endif	// def DISTORTOS_CHIP_USART4_ENABLE

#ifdef DISTORTOS_CHIP_UART5_ENABLE
/// true if UART5 has RX/TX FIFO, false otherwise
#ifdef DISTORTOS_CHIP_UART5_HAS_FIFO
constexpr bool uart5HasFifo {true};
#else	// !def DISTORTOS_CHIP_UART5_HAS_FIFO
constexpr bool uart5HasFifo {false};
#endif	// !def DISTORTOS_CHIP_UART5_HAS_FIFO
#endif	// def DISTORTOS_CHIP_UART5_ENABLE

#ifdef DISTORTOS_CHIP_USART5_ENABLE
/// true if USART5 has RX/TX FIFO, false otherwise
#ifdef DISTORTOS_CHIP_USART5_HAS_FIFO
constexpr bool usart5HasFifo {true};
#else	// !def DISTORTOS_CHIP_USART5_HAS_FIFO
constexpr bool usart5HasFifo {false};
#endif	// !def DISTORTOS_CHIP_USART5_HAS_FIFO
#endif	// def DISTORTOS_CHIP_USART5_ENABLE

#ifdef DISTORTOS_CHIP_USART6_ENABLE
/// true if USART6 has RX/TX FIFO, false otherwise
#ifdef DISTORTOS_CHIP_USART6_HAS_FIFO
constexpr bool usart6HasFifo {true};
#else	// !def DISTORTOS_CHIP_USART6_HAS_FIFO
constexpr bool usart6HasFifo {false};
#endif	// !def DISTORTOS_CHIP_USART6_HAS_FIFO
#endif	// def DISTORTOS_CHIP_USART6_ENABLE

#ifdef DISTORTOS_CHIP_UART7_ENABLE
/// true if UART7 has RX/TX FIFO, false otherwise
#ifdef DISTORTOS_CHIP_UART7_HAS_FIFO
constexpr bool uart7HasFifo {true};
#else	// !def DISTORTOS_CHIP_UART7_HAS_FIFO
constexpr bool uart7HasFifo {false};
#endif	// !def DISTORTOS_CHIP_UART7_HAS_FIFO
#endif	// def DISTORTOS_CHIP_UART7_ENABLE

#ifdef DISTORTOS_CHIP_USART7_ENABLE
/// true if USART7 has RX/TX FIFO, false otherwise
#ifdef DISTORTOS_CHIP_USART7_HAS_FIFO
constexpr bool usart7HasFifo {true};
#else	// !def DISTORTOS_CHIP_USART7_HAS_FIFO
constexpr bool usart7HasFifo {false};
#endif	// !def DISTORTOS_CHIP_USART7_HAS_FIFO
#endif	// def DISTORTOS_CHIP_USART7_ENABLE

#ifdef DISTORTOS_CHIP_UART8_ENABLE
/// true if UART8 has RX/TX FIFO, false otherwise
#ifdef DISTORTOS_CHIP_UART8_HAS_FIFO
constexpr bool uart8HasFifo {true};
#else	// !def DISTORTOS_CHIP_UART8_HAS_FIFO
constexpr bool uart8HasFifo {false};
#endif	// !def DISTORTOS_CHIP_UART8_HAS_FIFO
#endif	// def DISTORTOS_CHIP_UART8_ENABLE

#ifdef DISTORTOS_CHIP_USART8_ENABLE
/// true if USART8 has RX/TX FIFO, false otherwise
#ifdef DISTORTOS_CHIP_USART8_HAS_FIFO
constexpr bool usart8HasFifo {true};
#else	// !def DISTORTOS_CHIP_USART8_HAS_FIFO
constexpr bool usart8HasFifo {false};
#endif	// !def DISTORTOS_CHIP_USART8_HAS_FIFO
#endif	// def DISTORTOS_CHIP_USART8_ENABLE

}	// namespace

/*---------------------------------------------------------------------------------------------------------------------+
| public types
+---------------------------------------------------------------------------------------------------------------------*/

/// parameters for construction of UART low-level drivers
class ChipUartLowLevel::Parameters
{
public:

#ifdef DISTORTOS_BITBANDING_SUPPORTED

	/**
	 * \brief Parameters's constructor
	 *
	 * \param [in] uartBase is a base address of UART peripheral
	 * \param [in] hasFifo selects whether this UART peripheral has RX/TX FIFO (true) or not (false)
	 * \param [in] rccEnBb is an address of bitband alias of appropriate U[S]ARTxEN bit in RCC register
	 * \param [in] rccRstBb is an address of bitband alias of appropriate U[S]ARTxRST bit in RCC register
	 */

	constexpr Parameters(const uintptr_t uartBase, const bool hasFifo, const uintptr_t rccEnBbAddress,
			const uintptr_t rccRstBbAddress) :
					uartBase_{uartBase},
					peripheralFrequency_{getBusFrequency(uartBase)},
					rxneieBbAddress_{STM32_BITBAND_IMPLEMENTATION(uartBase, USART_TypeDef, CR1, USART_CR1_RXNEIE)},
					tcieBbAddress_{STM32_BITBAND_IMPLEMENTATION(uartBase, USART_TypeDef, CR1, USART_CR1_TCIE)},
					txeieBbAddress_{STM32_BITBAND_IMPLEMENTATION(uartBase, USART_TypeDef, CR1, USART_CR1_TXEIE)},
					rccEnBbAddress_{rccEnBbAddress},
					rccRstBbAddress_{rccRstBbAddress},
					hasFifo_{hasFifo}
	{

	}

#else	// !def DISTORTOS_BITBANDING_SUPPORTED

	/**
	 * \brief Parameters's constructor
	 *
	 * \param [in] uartBase is a base address of UART peripheral
	 * \param [in] hasFifo selects whether this UART peripheral has RX/TX FIFO (true) or not (false)
	 * \param [in] rccEnOffset is the offset of RCC register with appropriate U[S]ARTxEN bit, bytes
	 * \param [in] rccEnBitmask is the bitmask of appropriate U[S]ARTxEN bit in RCC register at \a rccEnOffset offset
	 * \param [in] rccRstOffset is the offset of RCC register with appropriate U[S]ARTxRST bit, bytes
	 * \param [in] rccRstBitmask is the bitmask of appropriate U[S]ARTxRST bit in RCC register at \a rccRstOffset offset
	 */

	constexpr Parameters(const uintptr_t uartBase, const bool hasFifo, const size_t rccEnOffset,
			const uint32_t rccEnBitmask, const size_t rccRstOffset, const uint32_t rccRstBitmask) :
					uartBase_{uartBase},
					peripheralFrequency_{getBusFrequency(uartBase)},
					rccEnBitmask_{rccEnBitmask},
					rccEnOffset_{rccEnOffset},
					rccRstBitmask_{rccRstBitmask},
					rccRstOffset_{rccRstOffset},
					hasFifo_{hasFifo}
	{

	}

#endif	// !def DISTORTOS_BITBANDING_SUPPORTED

	/**
	 * \brief Enables or disables peripheral clock in RCC.
	 *
	 * \param [in] enable selects whether the clock will be enabled (true) or disabled (false)
	 */

	void enablePeripheralClock(const bool enable) const
	{
#ifdef DISTORTOS_BITBANDING_SUPPORTED
		*reinterpret_cast<volatile unsigned long*>(rccEnBbAddress_) = enable;
#else	// !def DISTORTOS_BITBANDING_SUPPORTED
		auto& rccEn = *reinterpret_cast<volatile uint32_t*>(RCC_BASE + rccEnOffset_);
		const InterruptMaskingLock interruptMaskingLock;
		rccEn = (rccEn & ~rccEnBitmask_) | (enable == true ? rccEnBitmask_ : 0);
#endif	// !def DISTORTOS_BITBANDING_SUPPORTED
	}

	/**
	 * \brief Enables or disables RXNE interrupt of UART.
	 *
	 * \param [in] enable selects whether the interrupt will be enabled (true) or disabled (false)
	 */

	void enableRxneInterrupt(const bool enable) const
	{
#ifdef DISTORTOS_BITBANDING_SUPPORTED
		*reinterpret_cast<volatile unsigned long*>(rxneieBbAddress_) = enable;
#else	// !def DISTORTOS_BITBANDING_SUPPORTED
		auto& uart = getUart();
		const InterruptMaskingLock interruptMaskingLock;
		uart.CR1 = (uart.CR1 & ~USART_CR1_RXNEIE) | (enable == true ? USART_CR1_RXNEIE : 0);
#endif	// !def DISTORTOS_BITBANDING_SUPPORTED
	}

	/**
	 * \brief Enables or disables TC interrupt of UART.
	 *
	 * \param [in] enable selects whether the interrupt will be enabled (true) or disabled (false)
	 */

	void enableTcInterrupt(const bool enable) const
	{
#ifdef DISTORTOS_BITBANDING_SUPPORTED
		*reinterpret_cast<volatile unsigned long*>(tcieBbAddress_) = enable;
#else	// !def DISTORTOS_BITBANDING_SUPPORTED
		auto& uart = getUart();
		const InterruptMaskingLock interruptMaskingLock;
		uart.CR1 = (uart.CR1 & ~USART_CR1_TCIE) | (enable == true ? USART_CR1_TCIE : 0);
#endif	// !def DISTORTOS_BITBANDING_SUPPORTED
	}

	/**
	 * \brief Enables or disables TXE interrupt of UART.
	 *
	 * \param [in] enable selects whether the interrupt will be enabled (true) or disabled (false)
	 */

	void enableTxeInterrupt(const bool enable) const
	{
#ifdef DISTORTOS_BITBANDING_SUPPORTED
		*reinterpret_cast<volatile unsigned long*>(txeieBbAddress_) = enable;
#else	// !def DISTORTOS_BITBANDING_SUPPORTED
		auto& uart = getUart();
		const InterruptMaskingLock interruptMaskingLock;
		uart.CR1 = (uart.CR1 & ~USART_CR1_TXEIE) | (enable == true ? USART_CR1_TXEIE : 0);
#endif	// !def DISTORTOS_BITBANDING_SUPPORTED
	}

	/**
	 * \return character length, bits
	 */

	uint8_t getCharacterLength() const
	{
		const auto cr1 = getUart().CR1;
		const auto realCharacterLength =
#ifdef DISTORTOS_CHIP_USART_HAS_CR1_M1_BIT
				(cr1 & USART_CR1_M1) != 0 ? 7 :
#endif	// def DISTORTOS_CHIP_USART_HAS_CR1_M1_BIT
				(cr1 & USART_CR1_M0) != 0 ? 9 : 8;
		const auto parityControlEnabled = (cr1 & USART_CR1_PCE) != 0;
		return realCharacterLength - parityControlEnabled;
	}

	/**
	 * \return peripheral clock frequency, Hz
	 */

	uint32_t getPeripheralFrequency() const
	{
		return peripheralFrequency_;
	}

	/**
	 * \return reference to USART_TypeDef object
	 */

	USART_TypeDef& getUart() const
	{
		return *reinterpret_cast<USART_TypeDef*>(uartBase_);
	}

	/**
	 * \return true if this UART peripheral has RX/TX FIFO, false otherwise
	 */

	bool hasFifo() const
	{
		return hasFifo_;
	}

	/**
	 * \brief Resets all peripheral's registers via RCC
	 *
	 * \note Peripheral clock must be enabled in RCC for this operation to work.
	 */

	void resetPeripheral() const
	{
#ifdef DISTORTOS_BITBANDING_SUPPORTED
		*reinterpret_cast<volatile unsigned long*>(rccRstBbAddress_) = 1;
		*reinterpret_cast<volatile unsigned long*>(rccRstBbAddress_) = 0;
#else	// !def DISTORTOS_BITBANDING_SUPPORTED
		auto& rccRst = *reinterpret_cast<volatile uint32_t*>(RCC_BASE + rccRstOffset_);
		const InterruptMaskingLock interruptMaskingLock;
		rccRst |= rccRstBitmask_;
		rccRst &= ~rccRstBitmask_;
#endif	// !def DISTORTOS_BITBANDING_SUPPORTED
	}

private:

	/// base address of UART peripheral
	uintptr_t uartBase_;

	/// peripheral clock frequency, Hz
	uint32_t peripheralFrequency_;

#ifdef DISTORTOS_BITBANDING_SUPPORTED

	/// address of bitband alias of RXNEIE bit in USART_CR1 register
	uintptr_t rxneieBbAddress_;

	/// address of bitband alias of TCIE bit in USART_CR1 register
	uintptr_t tcieBbAddress_;

	/// address of bitband alias of TXEIE bit in USART_CR1 register
	uintptr_t txeieBbAddress_;

	/// address of bitband alias of appropriate U[S]ARTxEN bit in RCC register
	uintptr_t rccEnBbAddress_;

	/// address of bitband alias of appropriate U[S]ARTxRST bit in RCC register
	uintptr_t rccRstBbAddress_;

#else	// !def DISTORTOS_BITBANDING_SUPPORTED

	/// bitmask of appropriate U[S]ARTxEN bit in RCC register at \a rccEnOffset_ offset
	uint32_t rccEnBitmask_;

	/// offset of RCC register with appropriate U[S]ARTxEN bit, bytes
	size_t rccEnOffset_;

	/// bitmask of appropriate U[S]ARTxRST bit in RCC register at \a rccRstOffset_ offset
	uint32_t rccRstBitmask_;

	/// offset of RCC register with appropriate U[S]ARTxRST bit, bytes
	size_t rccRstOffset_;

#endif	// !def DISTORTOS_BITBANDING_SUPPORTED

	/// selects whether this UART peripheral has RX/TX FIFO (true) or not (false)
	bool hasFifo_;
};

/*---------------------------------------------------------------------------------------------------------------------+
| public static objects
+---------------------------------------------------------------------------------------------------------------------*/

#ifdef DISTORTOS_BITBANDING_SUPPORTED

#ifdef DISTORTOS_CHIP_USART1_ENABLE
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart1Parameters {USART1_BASE, usart1HasFifo,
		STM32_BITBAND_ADDRESS(RCC, APB2ENR, USART1EN), STM32_BITBAND_ADDRESS(RCC, APB2RSTR, USART1RST)};
#endif	// def DISTORTOS_CHIP_USART1_ENABLE

#ifdef DISTORTOS_CHIP_USART2_ENABLE
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart2Parameters {USART2_BASE, usart2HasFifo,
		STM32_BITBAND_ADDRESS(RCC, APB1ENR1, USART2EN), STM32_BITBAND_ADDRESS(RCC, APB1RSTR1, USART2RST)};
#endif	// def DISTORTOS_CHIP_USART2_ENABLE

#ifdef DISTORTOS_CHIP_USART3_ENABLE
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart3Parameters {USART3_BASE, usart3HasFifo,
		STM32_BITBAND_ADDRESS(RCC, APB1ENR1, USART3EN), STM32_BITBAND_ADDRESS(RCC, APB1RSTR1, USART3RST)};
#endif	// def DISTORTOS_CHIP_USART3_ENABLE

#ifdef DISTORTOS_CHIP_UART4_ENABLE
const ChipUartLowLevel::Parameters ChipUartLowLevel::uart4Parameters {UART4_BASE, uart4HasFifo,
		STM32_BITBAND_ADDRESS(RCC, APB1ENR1, UART4EN), STM32_BITBAND_ADDRESS(RCC, APB1RSTR1, UART4RST)};
#endif	// def DISTORTOS_CHIP_UART4_ENABLE

#ifdef DISTORTOS_CHIP_UART5_ENABLE
const ChipUartLowLevel::Parameters ChipUartLowLevel::uart5Parameters {UART5_BASE, uart5HasFifo,
		STM32_BITBAND_ADDRESS(RCC, APB1ENR1, UART5EN), STM32_BITBAND_ADDRESS(RCC, APB1RSTR1, UART5RST)};
#endif	// def DISTORTOS_CHIP_UART5_ENABLE

#else	// !def DISTORTOS_BITBANDING_SUPPORTED

#ifdef DISTORTOS_CHIP_LPUART1_ENABLE
#if defined(RCC_APB1ENR_LPUART1EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::lpuart1Parameters {LPUART1_BASE, lpuart1HasFifo,
		offsetof(RCC_TypeDef, APB1ENR), RCC_APB1ENR_LPUART1EN, offsetof(RCC_TypeDef, APB1RSTR),
		RCC_APB1RSTR_LPUART1RST};
#elif defined(RCC_APB3ENR_LPUART1EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::lpuart1Parameters {LPUART1_BASE, lpuart1HasFifo,
		offsetof(RCC_TypeDef, APB3ENR), RCC_APB3ENR_LPUART1EN, offsetof(RCC_TypeDef, APB3RSTR),
		RCC_APB3RSTR_LPUART1RST};
#elif defined(RCC_APBENR1_LPUART1EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::lpuart1Parameters {LPUART1_BASE, lpuart1HasFifo,
		offsetof(RCC_TypeDef, APBENR1), RCC_APBENR1_LPUART1EN, offsetof(RCC_TypeDef, APBRSTR1),
		RCC_APBRSTR1_LPUART1RST};
#else
	#error "Unsupported LPUART1 variant!"
#endif
#endif	// def DISTORTOS_CHIP_LPUART1_ENABLE

#ifdef DISTORTOS_CHIP_LPUART2_ENABLE
const ChipUartLowLevel::Parameters ChipUartLowLevel::lpuart2Parameters {LPUART2_BASE, lpuart2HasFifo,
		offsetof(RCC_TypeDef, APBENR1), RCC_APBENR1_LPUART2EN, offsetof(RCC_TypeDef, APBRSTR1),
		RCC_APBRSTR1_LPUART2RST};
#endif	// def DISTORTOS_CHIP_LPUART2_ENABLE

#ifdef DISTORTOS_CHIP_USART1_ENABLE
#if defined(RCC_APB2ENR_USART1EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart1Parameters {USART1_BASE, usart1HasFifo,
		offsetof(RCC_TypeDef, APB2ENR), RCC_APB2ENR_USART1EN, offsetof(RCC_TypeDef, APB2RSTR), RCC_APB2RSTR_USART1RST};
#elif defined(RCC_APBENR2_USART1EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart1Parameters {USART1_BASE, usart1HasFifo,
		offsetof(RCC_TypeDef, APBENR2), RCC_APBENR2_USART1EN, offsetof(RCC_TypeDef, APBRSTR2),
		RCC_APBRSTR2_USART1RST};
#else
	#error "Unsupported USART1 variant!"
#endif
#endif	// def DISTORTOS_CHIP_USART1_ENABLE

#ifdef DISTORTOS_CHIP_USART2_ENABLE
#if defined(RCC_APB1ENR_USART2EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart2Parameters {USART2_BASE, usart2HasFifo,
		offsetof(RCC_TypeDef, APB1ENR), RCC_APB1ENR_USART2EN, offsetof(RCC_TypeDef, APB1RSTR), RCC_APB1RSTR_USART2RST};
#elif defined(RCC_APB1ENR1_USART2EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart2Parameters {USART2_BASE, usart2HasFifo,
		offsetof(RCC_TypeDef, APB1ENR1), RCC_APB1ENR1_USART2EN, offsetof(RCC_TypeDef, APB1RSTR1),
		RCC_APB1RSTR1_USART2RST};
#elif defined(RCC_APBENR1_USART2EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart2Parameters {USART2_BASE, usart2HasFifo,
		offsetof(RCC_TypeDef, APBENR1), RCC_APBENR1_USART2EN, offsetof(RCC_TypeDef, APBRSTR1), RCC_APBRSTR1_USART2RST};
#else
	#error "Unsupported USART2 variant!"
#endif
#endif	// def DISTORTOS_CHIP_USART2_ENABLE

#ifdef DISTORTOS_CHIP_USART3_ENABLE
#if defined(RCC_APB1ENR_USART3EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart3Parameters {USART3_BASE, usart3HasFifo,
		offsetof(RCC_TypeDef, APB1ENR), RCC_APB1ENR_USART3EN, offsetof(RCC_TypeDef, APB1RSTR), RCC_APB1RSTR_USART3RST};
#elif defined(RCC_APB1ENR1_USART3EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart3Parameters {USART3_BASE, usart3HasFifo,
		offsetof(RCC_TypeDef, APB1ENR1), RCC_APB1ENR1_USART3EN, offsetof(RCC_TypeDef, APB1RSTR1),
		RCC_APB1RSTR1_USART3RST};
#elif defined(RCC_APBENR1_USART3EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart3Parameters {USART3_BASE, usart3HasFifo,
		offsetof(RCC_TypeDef, APBENR1), RCC_APBENR1_USART3EN, offsetof(RCC_TypeDef, APBRSTR1), RCC_APBRSTR1_USART3RST};
#else
	#error "Unsupported USART3 variant!"
#endif
#endif	// def DISTORTOS_CHIP_USART3_ENABLE

#ifdef DISTORTOS_CHIP_UART4_ENABLE
#if defined(RCC_APB1ENR_UART4EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::uart4Parameters {UART4_BASE, uart4HasFifo,
		offsetof(RCC_TypeDef, APB1ENR), RCC_APB1ENR_UART4EN, offsetof(RCC_TypeDef, APB1RSTR), RCC_APB1RSTR_UART4RST};
#elif defined(RCC_APB1ENR1_UART4EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::uart4Parameters {UART4_BASE, uart4HasFifo,
		offsetof(RCC_TypeDef, APB1ENR1), RCC_APB1ENR1_UART4EN, offsetof(RCC_TypeDef, APB1RSTR1),
		RCC_APB1RSTR1_UART4RST};
#else
	#error "Unsupported UART4 variant!"
#endif
#endif	// def DISTORTOS_CHIP_UART4_ENABLE

#ifdef DISTORTOS_CHIP_USART4_ENABLE
#if defined(RCC_APB1ENR_USART4EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart4Parameters {USART4_BASE, usart4HasFifo,
		offsetof(RCC_TypeDef, APB1ENR), RCC_APB1ENR_USART4EN, offsetof(RCC_TypeDef, APB1RSTR), RCC_APB1RSTR_USART4RST};
#elif defined(RCC_APBENR1_USART4EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart4Parameters {USART4_BASE, usart4HasFifo,
		offsetof(RCC_TypeDef, APBENR1), RCC_APBENR1_USART4EN, offsetof(RCC_TypeDef, APBRSTR1), RCC_APBRSTR1_USART4RST};
#else
	#error "Unsupported USART4 variant!"
#endif
#endif	// def DISTORTOS_CHIP_USART4_ENABLE

#ifdef DISTORTOS_CHIP_UART5_ENABLE
#if defined(RCC_APB1ENR_UART5EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::uart5Parameters {UART5_BASE, uart5HasFifo,
		offsetof(RCC_TypeDef, APB1ENR), RCC_APB1ENR_UART5EN, offsetof(RCC_TypeDef, APB1RSTR), RCC_APB1RSTR_UART5RST};
#elif defined(RCC_APB1ENR1_UART5EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::uart5Parameters {UART5_BASE, uart5HasFifo,
		offsetof(RCC_TypeDef, APB1ENR1), RCC_APB1ENR1_UART5EN, offsetof(RCC_TypeDef, APB1RSTR1),
		RCC_APB1RSTR1_UART5RST};
#else
	#error "Unsupported UART5 variant!"
#endif
#endif	// def DISTORTOS_CHIP_UART5_ENABLE

#ifdef DISTORTOS_CHIP_USART5_ENABLE
#if defined(RCC_APB1ENR_USART5EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart5Parameters {USART5_BASE, usart5HasFifo,
		offsetof(RCC_TypeDef, APB1ENR), RCC_APB1ENR_USART5EN, offsetof(RCC_TypeDef, APB1RSTR), RCC_APB1RSTR_USART5RST};
#elif defined(RCC_APBENR1_USART5EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart5Parameters {USART5_BASE, usart5HasFifo,
		offsetof(RCC_TypeDef, APBENR1), RCC_APBENR1_USART5EN, offsetof(RCC_TypeDef, APBRSTR1), RCC_APBRSTR1_USART5RST};
#else
	#error "Unsupported USART5 variant!"
#endif
#endif	// def DISTORTOS_CHIP_USART5_ENABLE

#ifdef DISTORTOS_CHIP_USART6_ENABLE
#if defined(RCC_APB2ENR_USART6EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart6Parameters {USART6_BASE, usart6HasFifo,
		offsetof(RCC_TypeDef, APB2ENR), RCC_APB2ENR_USART6EN, offsetof(RCC_TypeDef, APB2RSTR), RCC_APB2RSTR_USART6RST};
#elif defined(RCC_APBENR1_USART6EN)
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart6Parameters {USART6_BASE, usart6HasFifo,
		offsetof(RCC_TypeDef, APBENR1), RCC_APBENR1_USART6EN, offsetof(RCC_TypeDef, APBRSTR1), RCC_APBRSTR1_USART6RST};
#else
	#error "Unsupported USART6 variant!"
#endif
#endif	// def DISTORTOS_CHIP_USART6_ENABLE

#ifdef DISTORTOS_CHIP_UART7_ENABLE
const ChipUartLowLevel::Parameters ChipUartLowLevel::uart7Parameters {UART7_BASE, uart7HasFifo,
		offsetof(RCC_TypeDef, APB1ENR), RCC_APB1ENR_UART7EN, offsetof(RCC_TypeDef, APB1RSTR), RCC_APB1RSTR_UART7RST};
#endif	// def DISTORTOS_CHIP_UART7_ENABLE

#ifdef DISTORTOS_CHIP_USART7_ENABLE
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart7Parameters {USART7_BASE, usart7HasFifo,
		offsetof(RCC_TypeDef, APB2ENR), RCC_APB2ENR_USART7EN, offsetof(RCC_TypeDef, APB2RSTR), RCC_APB2RSTR_USART7RST};
#endif	// def DISTORTOS_CHIP_USART7_ENABLE

#ifdef DISTORTOS_CHIP_UART8_ENABLE
const ChipUartLowLevel::Parameters ChipUartLowLevel::uart8Parameters {UART8_BASE, uart8HasFifo,
		offsetof(RCC_TypeDef, APB1ENR), RCC_APB1ENR_UART8EN, offsetof(RCC_TypeDef, APB1RSTR), RCC_APB1RSTR_UART8RST};
#endif	// def DISTORTOS_CHIP_UART8_ENABLE

#ifdef DISTORTOS_CHIP_USART8_ENABLE
const ChipUartLowLevel::Parameters ChipUartLowLevel::usart8Parameters {USART8_BASE, usart8HasFifo,
		offsetof(RCC_TypeDef, APB2ENR), RCC_APB2ENR_USART8EN, offsetof(RCC_TypeDef, APB2RSTR), RCC_APB2RSTR_USART8RST};
#endif	// def DISTORTOS_CHIP_USART8_ENABLE

#endif	// !def DISTORTOS_BITBANDING_SUPPORTED

/*---------------------------------------------------------------------------------------------------------------------+
| public functions
+---------------------------------------------------------------------------------------------------------------------*/

ChipUartLowLevel::~ChipUartLowLevel()
{
	if (isStarted() == false)
		return;

	parameters_.resetPeripheral();
	parameters_.enablePeripheralClock(false);
}

void ChipUartLowLevel::interruptHandler()
{
	auto& uart = parameters_.getUart();
	const auto characterLength = parameters_.getCharacterLength();

	while (1)	// loop while there are enabled interrupt sources waiting to be served
	{
		const auto cr1 = uart.CR1;
		const auto isr = uart.ISR;

		if ((cr1 & USART_CR1_RXNEIE) != 0 && (isr & (USART_ISR_RXNE | USART_ISR_ORE)) != 0)		// read & receive errors
		{
			const uint16_t characterMask = (1 << characterLength) - 1;
			const uint16_t character = uart.RDR & characterMask;
			const auto readBuffer = readBuffer_;
			auto readPosition = readPosition_;
			readBuffer[readPosition++] = character;
			if (characterLength > 8)
				readBuffer[readPosition++] = character >> 8;
			readPosition_ = readPosition;
			const auto isrErrorFlags = isr & (USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE | USART_ISR_PE);
			if (isrErrorFlags != 0)
			{
				uart.ICR = isrErrorFlags;	// clear served error flags
				uartBase_->receiveErrorEvent(decodeErrors(isr));
			}
			if (readPosition == readSize_)
				uartBase_->readCompleteEvent(stopRead());
		}
		else if ((cr1 & USART_CR1_TXEIE) != 0 && (isr & USART_ISR_TXE) != 0)	// write
		{
			const auto writeBuffer = writeBuffer_;
			auto writePosition = writePosition_;
			const uint16_t characterLow = writeBuffer[writePosition++];
			const uint16_t characterHigh = characterLength > 8 ? writeBuffer[writePosition++] : 0;
			writePosition_ = writePosition;
			uart.TDR = characterLow | characterHigh << 8;
			if (writePosition == writeSize_)
				uartBase_->writeCompleteEvent(stopWrite());
		}
		else if ((cr1 & USART_CR1_TCIE) != 0 && (isr & USART_ISR_TC) != 0)	// transmit complete
		{
			parameters_.enableTcInterrupt(false);
			uartBase_->transmitCompleteEvent();
		}
		else	// nothing more to do
			return;
	}
}

std::pair<int, uint32_t> ChipUartLowLevel::start(devices::UartBase& uartBase, const uint32_t baudRate,
		const uint8_t characterLength, const devices::UartParity parity, const bool _2StopBits,
		const bool hardwareFlowControl)
{
	if (isStarted() == true)
		return {EBADF, {}};

	const auto peripheralFrequency = parameters_.getPeripheralFrequency();
	uint32_t brr;
	bool over8;
	uint32_t realBaudRate;
#ifdef IS_LPUART_INSTANCE
	if (IS_LPUART_INSTANCE(&parameters_.getUart()) == false)
#endif	// def IS_LPUART_INSTANCE
	{
		const auto divider = (peripheralFrequency + baudRate / 2) / baudRate;
		over8 = divider < 16;
		const auto mantissa = divider / (over8 == false ? 16 : 8);
		const auto fraction = divider % (over8 == false ? 16 : 8);

		if (mantissa == 0 || mantissa > (USART_BRR_DIV_MANTISSA >> USART_BRR_DIV_MANTISSA_Pos))
			return {EINVAL, {}};

		brr = mantissa << USART_BRR_DIV_MANTISSA_Pos | fraction << USART_BRR_DIV_FRACTION_Pos;
		realBaudRate = peripheralFrequency / divider;
	}
#ifdef IS_LPUART_INSTANCE
	else
	{
		over8 = {};	// LPUART doesn't have OVER8 bit
		brr = (256ull * peripheralFrequency + baudRate / 2) / baudRate;
		if (brr < 0x300 || brr >= (1 << 20))
			return {EINVAL, {}};

		realBaudRate = 256 * peripheralFrequency / brr;
	}
#endif	// def IS_LPUART_INSTANCE

	const auto realCharacterLength = characterLength + (parity != devices::UartParity::none);
	if (realCharacterLength < minCharacterLength + 1 || realCharacterLength > maxCharacterLength)
		return {EINVAL, {}};

	parameters_.enablePeripheralClock(true);
	parameters_.resetPeripheral();

	uartBase_ = &uartBase;
	auto& uart = parameters_.getUart();
	uart.BRR = brr;
	uart.CR2 = _2StopBits << (USART_CR2_STOP_Pos + 1);
	if (hardwareFlowControl == true)
		uart.CR3 = USART_CR3_CTSE | USART_CR3_RTSE;
	uart.CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_UE | over8 << USART_CR1_OVER8_Pos |
			(realCharacterLength == maxCharacterLength) << USART_CR1_M0_Pos |
#ifdef DISTORTOS_CHIP_USART_HAS_CR1_M1_BIT
			(realCharacterLength == minCharacterLength + 1) << USART_CR1_M1_Pos |
#endif	// def DISTORTOS_CHIP_USART_HAS_CR1_M1_BIT
#ifdef USART_CR1_FIFOEN_Pos
			(parameters_.hasFifo() << USART_CR1_FIFOEN_Pos) |
#endif	// def USART_CR1_FIFOEN_Pos
			(parity != devices::UartParity::none) << USART_CR1_PCE_Pos |
			(parity == devices::UartParity::odd) << USART_CR1_PS_Pos;
	return {{}, realBaudRate};
}

int ChipUartLowLevel::startRead(void* const buffer, const size_t size)
{
	if (buffer == nullptr || size == 0)
		return EINVAL;

	if (isStarted() == false)
		return EBADF;

	if (isReadInProgress() == true)
		return EBUSY;

	if (parameters_.getCharacterLength() > 8 && size % 2 != 0)
		return EINVAL;

	readBuffer_ = static_cast<uint8_t*>(buffer);
	readSize_ = size;
	readPosition_ = 0;
	parameters_.enableRxneInterrupt(true);
	return 0;
}

int ChipUartLowLevel::startWrite(const void* const buffer, const size_t size)
{
	if (buffer == nullptr || size == 0)
		return EINVAL;

	if (isStarted() == false)
		return EBADF;

	if (isWriteInProgress() == true)
		return EBUSY;

	if (parameters_.getCharacterLength() > 8 && size % 2 != 0)
		return EINVAL;

	writeBuffer_ = static_cast<const uint8_t*>(buffer);
	writeSize_ = size;
	writePosition_ = 0;
	parameters_.enableTcInterrupt(false);

	if ((parameters_.getUart().ISR & USART_ISR_TC) != 0)
		uartBase_->transmitStartEvent();

	parameters_.enableTxeInterrupt(true);
	return 0;
}

int ChipUartLowLevel::stop()
{
	if (isStarted() == false)
		return EBADF;

	if (isReadInProgress() == true || isWriteInProgress() == true)
		return EBUSY;

	parameters_.resetPeripheral();
	parameters_.enablePeripheralClock(false);
	uartBase_ = nullptr;
	return 0;
}

size_t ChipUartLowLevel::stopRead()
{
	if (isReadInProgress() == false)
		return 0;

	parameters_.enableRxneInterrupt(false);
	const auto bytesRead = readPosition_;
	readPosition_ = {};
	readSize_ = {};
	readBuffer_ = {};
	return bytesRead;
}

size_t ChipUartLowLevel::stopWrite()
{
	if (isWriteInProgress() == false)
		return 0;

	parameters_.enableTxeInterrupt(false);
	parameters_.enableTcInterrupt(true);
	const auto bytesWritten = writePosition_;
	writePosition_ = {};
	writeSize_ = {};
	writeBuffer_ = {};
	return bytesWritten;
}

}	// namespace chip

}	// namespace distortos
