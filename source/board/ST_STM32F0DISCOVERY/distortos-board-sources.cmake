#
# file: distortos-board-sources.cmake
#
# author: Copyright (C) 2018-2024 Kamil Szczygiel https://distortec.com https://freddiechopin.info
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
# distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Automatically generated file - do not edit!
#

distortosSetFixedConfiguration(BOOLEAN
		DISTORTOS_BOARD_ST_STM32F0DISCOVERY
		ON)

distortosSetFixedConfiguration(STRING
		DISTORTOS_BOARD
		"ST,STM32F0DISCOVERY")

target_include_directories(distortos PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(distortos PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/ST_STM32F0DISCOVERY-buttons.cpp
		${CMAKE_CURRENT_LIST_DIR}/ST_STM32F0DISCOVERY-dmas.cpp
		${CMAKE_CURRENT_LIST_DIR}/ST_STM32F0DISCOVERY-leds.cpp
		${CMAKE_CURRENT_LIST_DIR}/ST_STM32F0DISCOVERY-spis.cpp
		${CMAKE_CURRENT_LIST_DIR}/ST_STM32F0DISCOVERY-uarts.cpp
		${CMAKE_CURRENT_LIST_DIR}/ST_STM32F0DISCOVERY-vectorTable.cpp)

doxygen(INPUT ${CMAKE_CURRENT_LIST_DIR} INCLUDE_PATH ${CMAKE_CURRENT_LIST_DIR}/include)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/10-buttons.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/10-leds.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/50-STM32-SPIv2.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/50-STM32-USARTv2.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/60-STM32-GPIOv2.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/60-STM32-device-electronic-signature.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/70-STM32-DMAv1.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/80-STM32.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/90-ARMv6-M-ARMv7-M-ARMv8-M.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/distortos-board-sources.extension.cmake OPTIONAL)

set(DISTORTOS_BOARD_VERSION 14)
