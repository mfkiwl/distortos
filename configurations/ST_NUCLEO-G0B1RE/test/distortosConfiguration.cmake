#
# \file
# \brief distortos configuration
#
# \warning
# Automatically generated file - do not edit!
#

if(DEFINED ENV{DISTORTOS_PATH})
	set(DISTORTOS_PATH "$ENV{DISTORTOS_PATH}")
else()
	set(DISTORTOS_PATH "../")
endif()

set("distortos_Build_00_Static_destructors"
		"OFF"
		CACHE
		"BOOL"
		"Enable static destructors.\n\nEnable destructors for objects with static storage duration. As embedded applications almost never \"exit\", these destructors are usually never executed, wasting ROM.")
set("distortos_Scheduler_00_Tick_frequency"
		"1000"
		CACHE
		"STRING"
		"System's tick frequency, Hz.\n\nAllowed range: [1; 2147483647]")
set("distortos_Scheduler_01_Round_robin_frequency"
		"10"
		CACHE
		"STRING"
		"Round-robin frequency, Hz.\n\nAllowed range: [1; 1000]")
set("distortos_Scheduler_02_Support_for_signals"
		"ON"
		CACHE
		"BOOL"
		"Enable support for signals.\n\nEnable namespaces, functions and classes related to signals:\n- ThisThread::Signals namespace;\n- Thread::generateSignal();\n- Thread::getPendingSignalSet();\n- Thread::queueSignal();\n- DynamicSignalsReceiver class;\n- SignalInformationQueueWrapper class;\n- SignalsCatcher class;\n- SignalsReceiver class;\n- StaticSignalsReceiver class;\n\nWhen this options is not selected, these namespaces, functions and classes are not available at all.")
set("distortos_Scheduler_03_Support_for_thread_detachment"
		"ON"
		CACHE
		"BOOL"
		"Enable support for thread detachment.\n\nEnable functions that \"detach\" dynamic threads:\n- ThisThread::detach();\n- Thread::detach();\n\nWhen this options is not selected, these functions are not available at all.\n\nWhen dynamic and detached thread terminates, it will be added to the global list of threads pending for deferred deletion. The thread will actually be deleted in idle thread, but only when two mutexes are successfully locked:\n- mutex that protects dynamic memory allocator;\n- mutex that synchronizes access to the list of threads pending for deferred deletion;")
set("distortos_Scheduler_04_Main_thread_stack_size"
		"4096"
		CACHE
		"STRING"
		"Size (in bytes) of stack used by thread with main() function.\n\nAllowed range: [1; 2147483647]")
set("distortos_Scheduler_05_Main_thread_priority"
		"127"
		CACHE
		"STRING"
		"Initial priority of main thread.\n\nAllowed range: [1; 255]")
set("distortos_Scheduler_06_Reception_of_signals_by_main_thread"
		"ON"
		CACHE
		"BOOL"
		"Enable reception of signals for main thread.")
set("distortos_Scheduler_07_Queued_signals_for_main_thread"
		"8"
		CACHE
		"STRING"
		"Maximal number of queued signals for main thread. 0 disables queuing of signals for main thread.\n\nAllowed range: [0; 2147483647]")
set("distortos_Scheduler_08_SignalAction_objects_for_main_thread"
		"8"
		CACHE
		"STRING"
		"Maximal number of different SignalAction objects for main thread. 0 disables catching of signals for main thread.\n\nAllowed range: [0; 32]")
set("distortos_Checks_00_Context_of_functions"
		"ON"
		CACHE
		"BOOL"
		"Check context of functions.\n\nSome functions may only be used from thread context, as using them from interrupt context results in undefined behaviour. There are several groups of functions to which this restriction applies (some functions fall into several categories at once):\n- all blocking functions, like callOnce(), FifoQueue::push(), Semaphore::wait(), ..., as an attempt to block current thread of execution (not to be confused with current thread) is not possible in interrupt context;\n- all mutex functions, as the concept of ownership by a thread - core feature of mutex - cannot be fulfilled in interrupt context;\n- all functions from ThisThread namespace (including ThisThread::Signals namespace), as in interrupt context they would access a random thread that happened to be executing at that particular moment;\n\nUsing such functions from interrupt context is a common bug in applications which can be easily introduced and very hard to find, as the symptoms may appear only under certain circumstances.\n\nSelecting this option enables context checks in all functions with such requirements. If any of them is used from interrupt context, FATAL_ERROR() will be called.")
set("distortos_Checks_01_Stack_pointer_range_during_context_switch"
		"ON"
		CACHE
		"BOOL"
		"Check stack pointer range during context switch.\n\nSimple range checking of preempted thread's stack pointer can be performed during context switches. It is relatively fast, but cannot detect all stack overflows. The check is done before the software stack frame is pushed on thread's stack, but the size of this pending stack frame is accounted for - the intent is to detect a stack overflow which is about to happen, before it can cause (further) data corrution. FATAL_ERROR() will be called if the stack pointer is outside valid range.")
set("distortos_Checks_02_Stack_pointer_range_during_system_tick"
		"ON"
		CACHE
		"BOOL"
		"Check stack pointer range during system tick.\n\nSimilar to \"distortos_Checks_01_Stack_pointer_range_during_context_switch\", but executed during every system tick.")
set("distortos_Checks_03_Stack_guard_contents_during_context_switch"
		"ON"
		CACHE
		"BOOL"
		"Check stack guard contents during context switch.\n\nSelecting this option extends stacks for all threads (including main() thread) with a \"stack guard\" at the overflow end. This \"stack guard\" - just as the whole stack - is filled with a sentinel value 0xed419f25 during thread initialization. The contents of \"stack guard\" of preempted thread are checked during each context switch and if any byte has changed, FATAL_ERROR() will be called.\n\nThis method is slower than simple stack pointer range checking, but is able to detect stack overflows much more reliably. It is still sufficiently fast, assuming that the size of \"stack guard\" is reasonable.\n\nBe advised that uninitialized variables on stack which are larger than size of \"stack guard\" can create \"holes\" in the stack, thus circumventing this detection mechanism. This especially applies to arrays used as buffers.")
set("distortos_Checks_04_Stack_guard_contents_during_system_tick"
		"ON"
		CACHE
		"BOOL"
		"Check stack guard contents during system tick.\n\nSimilar to \"distortos_Checks_03_Stack_guard_contents_during_context_switch\", but executed during every system tick.")
set("distortos_Checks_05_Stack_guard_size"
		"32"
		CACHE
		"STRING"
		"Size (in bytes) of \"stack guard\".\n\nAny value which is not a multiple of stack alignment required by architecture, will be rounded up.\n\nAllowed range: [1; 2147483647]")
set("distortos_Checks_06_Asserts"
		"ON"
		CACHE
		"BOOL"
		"Enable asserts.\n\nSome errors, which are clearly program bugs, are never reported using error codes. When this option is enabled, these preconditions, postconditions, invariants and assertions are checked with assert() macro. On the other hand - with this option disabled, they are completely ignored.\n\nIt is highly recommended to keep this option enabled until the application is thoroughly tested.")
set("distortos_Checks_07_Lightweight_assert"
		"OFF"
		CACHE
		"BOOL"
		"Use lightweight assert instead of the regular one.\n\nIf assertion fails, regular assert does the following:\n- calls optional assertHook(), passing the information about error location (strings with file and function names, line number) and failed expression (string);\n- blocks interrupts;\n- calls abort();\n\nLightweight assert doesn't pass any arguments to assertHook() (declaration of this function is different with this option enabled) and replaces abort() with a simple infinite loop. The lightweight version is probably only usable with a debugger or as a method to just reset/halt the chip.")
set("distortos_Checks_08_Lightweight_FATAL_ERROR"
		"OFF"
		CACHE
		"BOOL"
		"Use lightweight FATAL_ERROR instead of the regular one.\n\nIn case of fatal error, regular FATAL_ERROR does the following:\n- calls optional fatalErrorHook(), passing the information about error location (strings with file and function names, line number) and message (string);\n- blocks interrupts;\n- calls abort();\n\nLightweight FATAL_ERROR doesn't pass any arguments to fatalErrorHook() (declaration of this function is different with this option enabled) and replaces abort() with a simple infinite loop. The lightweight version is probably only usable with a debugger or as a method to just reset/halt the chip.")
set("distortos_buttons"
		"ON"
		CACHE
		"BOOL"
		"Enable buttons")
set("distortos_buttons_B1"
		"ON"
		CACHE
		"BOOL"
		"Enable B1 (User)")
set("distortos_leds"
		"ON"
		CACHE
		"BOOL"
		"Enable leds")
set("distortos_leds_Ld4"
		"ON"
		CACHE
		"BOOL"
		"Enable Ld4 (Green, User)")
set("distortos_Peripherals_LPUART1"
		"ON"
		CACHE
		"BOOL"
		"Enable LPUART1 low-level driver.")
set("distortos_Peripherals_LPUART2"
		"ON"
		CACHE
		"BOOL"
		"Enable LPUART2 low-level driver.")
set("distortos_Peripherals_USART1"
		"ON"
		CACHE
		"BOOL"
		"Enable USART1 low-level driver.")
set("distortos_Peripherals_USART2"
		"ON"
		CACHE
		"BOOL"
		"Enable USART2 low-level driver.")
set("distortos_Peripherals_USART3"
		"ON"
		CACHE
		"BOOL"
		"Enable USART3 low-level driver.")
set("distortos_Peripherals_USART4"
		"ON"
		CACHE
		"BOOL"
		"Enable USART4 low-level driver.")
set("distortos_Peripherals_USART5"
		"ON"
		CACHE
		"BOOL"
		"Enable USART5 low-level driver.")
set("distortos_Peripherals_USART6"
		"ON"
		CACHE
		"BOOL"
		"Enable USART6 low-level driver.")
set("distortos_Peripherals_GPIOA"
		"ON"
		CACHE
		"BOOL"
		"Enable GPIOA.")
set("distortos_Peripherals_GPIOB"
		"ON"
		CACHE
		"BOOL"
		"Enable GPIOB.")
set("distortos_Peripherals_GPIOC"
		"ON"
		CACHE
		"BOOL"
		"Enable GPIOC.")
set("distortos_Peripherals_GPIOD"
		"ON"
		CACHE
		"BOOL"
		"Enable GPIOD.")
set("distortos_Peripherals_GPIOF"
		"ON"
		CACHE
		"BOOL"
		"Enable GPIOF.")
set("distortos_Clocks_00_Standard_configuration_of_clocks"
		"ON"
		CACHE
		"BOOL"
		"Enable standard configuration of clocks.\n\nThis will set values selected below and additionally configure appropriate FLASH latency before switching system clock to selected source.\n\nIf disabled, no clock configuration will be done during chip initialization. The values entered below (frequencies, dividers, ...) will only be used to determine chip clocks. The user must configure the chip manually to match these settings.")
set("distortos_Clocks_01_Voltage_scaling_range"
		"1"
		CACHE
		"STRING"
		"Select voltage scaling range.\n\nAllowed range: [1; 2]")
set("distortos_Clocks_03_HSI16"
		"ON"
		CACHE
		"BOOL"
		"Enable HSI16.")
set("distortos_Clocks_04_HSIDIV"
		"1"
		CACHE
		"STRING"
		"HSIDIV value for HSI16.\n\nIt is used to divide HSI16 frequency to produce HSISYS clock.\n\nHSISYS = HSI16 / HSIDIV")
set("distortos_Clocks_05_LSE"
		"OFF"
		CACHE
		"BOOL"
		"Enable LSE crystal/ceramic resonator, 32768 Hz.")
set("distortos_Clocks_07_PLL"
		"ON"
		CACHE
		"BOOL"
		"Enable PLL.")
set("distortos_Clocks_08_Clock_source_of_PLL"
		"HSI16"
		CACHE
		"STRING"
		"Select clock source of PLL.")
set("distortos_Clocks_09_PLLM"
		"2"
		CACHE
		"STRING"
		"PLLM value for PLLs.\n\nIt is used to divide PLL input clock (PLLin) before it is fed to VCO. VCO input frequency (VCOin) must be in the range [2.66 MHz; 16 MHz].\n\nVCOin = PLLin / PLLM\n\nAllowed range: [1; 8]")
set("distortos_Clocks_10_PLLN"
		"16"
		CACHE
		"STRING"
		"PLLN value for main PLL.\n\nIt is used to multiply VCO input frequency (VCOin). Resulting VCO output frequency (VCOout) must be in the range [96 MHz; 344 MHz] in voltage scaling range 1 or [96 MHz; 128 MHz] in voltage scaling range 2.\n\nVCOout = VCOin * PLLN = PLLin / PLLM * PLLN\n\nAllowed range: [8; 86]")
set("distortos_Clocks_11_PLLP"
		"2"
		CACHE
		"STRING"
		"PLLP value for main PLL.\n\nIt is used to divide VCO output frequency (VCOout) to produce clock for ADC and I2S (PLLPout). PLL \"P\" output frequency (PLLPout) must be in the range [3.09 MHz; 122 MHz] in voltage scaling range 1 or [3.09 MHz; 40 MHz] in voltage scaling range 2.\n\nPLLPout = VCOout / PLLP = PLLin / PLLM * PLLN / PLLP\n\nAllowed range: [2; 32]")
set("distortos_Clocks_12_PLLQ"
		"2"
		CACHE
		"STRING"
		"PLLQ value for main PLL.\n\nIt is used to divide VCO output frequency (VCOout) to produce clock for FDCAN, RNG, timers and USB (PLLQout). PLL \"Q\" output frequency (PLLQout) must be in the range [12 MHz; 128 MHz] in voltage scaling range 1 or [12 MHz; 33 MHz] in voltage scaling range 2.\n\nPLLQout = VCOout / PLLQ = PLLin / PLLM * PLLN / PLLQ\n\nAllowed range: [2; 8]")
set("distortos_Clocks_13_PLLR"
		"2"
		CACHE
		"STRING"
		"PLLR value for main PLL.\n\nIt is used to divide VCO output frequency (VCOout) to produce system clock (PLLRout). PLL \"R\" output frequency (PLLRout) must be in the range [12 MHz; 64 MHz] in voltage scaling range 1 or [12 MHz; 16 MHz] in voltage scaling range 2.\n\nPLLRout = VCOout / PLLR = PLLin / PLLM * PLLN / PLLR\n\nAllowed range: [2; 8]")
set("distortos_Clocks_14_PLLP_output"
		"OFF"
		CACHE
		"BOOL"
		"Enable PLL's \"P\" output.")
set("distortos_Clocks_15_PLLQ_output"
		"OFF"
		CACHE
		"BOOL"
		"Enable PLL's \"Q\" output.")
set("distortos_Clocks_16_PLLR_output"
		"ON"
		CACHE
		"BOOL"
		"Enable PLL's \"R\" output.")
set("distortos_Clocks_17_System_clock_source"
		"PLLR"
		CACHE
		"STRING"
		"Select system clock source.")
set("distortos_Clocks_18_HPRE"
		"1"
		CACHE
		"STRING"
		"AHB clock division factor\n\nAHBclk = SYSclk / AHBdivider")
set("distortos_Clocks_19_PPRE"
		"1"
		CACHE
		"STRING"
		"APB clock division factor.\n\nAPBclk = AHBclk / APBdivider")
set("distortos_Memory_00_Flash_prefetch"
		"ON"
		CACHE
		"BOOL"
		"Enable flash prefetch option in FLASH->ACR register.")
set("distortos_Memory_01_Flash_instruction_cache"
		"ON"
		CACHE
		"BOOL"
		"Enable flash instruction cache option in FLASH->ACR register.")
set("distortos_Architecture_00_Interrupt_stack_size"
		"1024"
		CACHE
		"STRING"
		"Size (in bytes) of \"main\" stack used by core exceptions and interrupts in Handler mode.\n\nAllowed range: [8; 2147483647]")
set("distortos_Memory_regions_00_text_vectorTable"
		"flash"
		CACHE
		"STRING"
		"Memory region for .text.vectorTable section in linker script")
set("distortos_Memory_regions_01_text"
		"flash"
		CACHE
		"STRING"
		"Memory region for .text section in linker script")
set("distortos_Memory_regions_02_ARM_exidx"
		"flash"
		CACHE
		"STRING"
		"Memory region for .ARM.exidx section in linker script")
set("distortos_Memory_regions_03_Main_stack"
		"SRAM"
		CACHE
		"STRING"
		"Memory region for main stack in linker script")
set("distortos_Memory_regions_04_bss"
		"SRAM"
		CACHE
		"STRING"
		"Memory region for .bss section in linker script")
set("distortos_Memory_regions_05_data_VMA"
		"SRAM"
		CACHE
		"STRING"
		"VMA memory region for .data section in linker script")
set("distortos_Memory_regions_06_data_LMA"
		"flash"
		CACHE
		"STRING"
		"LMA memory region for .data section in linker script")
set("distortos_Memory_regions_07_noinit"
		"SRAM"
		CACHE
		"STRING"
		"Memory region for .noinit section in linker script")
set("distortos_Memory_regions_08_SRAM_data_LMA"
		"flash"
		CACHE
		"STRING"
		"LMA memory region for .SRAM.data section in linker script")
set("distortos_Memory_regions_09_Process_stack"
		"SRAM"
		CACHE
		"STRING"
		"Memory region for process stack in linker script")
set("distortos_Memory_regions_10_Heap"
		"SRAM"
		CACHE
		"STRING"
		"Memory region for heap in linker script")
set("distortos_FileSystems_00_Integration_with_standard_library"
		"ON"
		CACHE
		"BOOL"
		"Enable integration of file systems with standard library.\n\nEnables functionality for accessing multiple distortos::FileSystem objects via functions from standard library headers. When this option is enabled, following features are enabled:\n- global functions distortos::mount() and distortos::unmount() (which supports deferred unmount of busy file system);\n- support for (most likely) all functions from <stdio.h> header, like fopen(), fclose(), fread(), fwrite(), fprintf(), fscanf() and so on;\n- support for selected I/O-related functions from <fcntl.h>, <unistd.h> and <sys/stat.h> headers: open(), close(), read(), write(), isatty(), lseek(), fstat(), mkdir(), stat() and unlink() (which supports both files and directories);\n- support for selected functions from <dirent.h> header: opendir(), closedir(), readdir_r(), rewinddir(), seekdir() and telldir();\n- support for statvfs() function from <sys/statvfs.h> header;")
set("DISTORTOS_CONFIGURATION_VERSION"
		"4"
		CACHE
		"INTERNAL"
		"")
set("CMAKE_BUILD_TYPE"
		"RelWithDebInfo"
		CACHE
		"STRING"
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel ...")
set("CMAKE_CXX_FLAGS"
		"-fno-rtti -fno-exceptions -ffunction-sections -fdata-sections -Wall -Wextra -Wshadow -Wno-psabi -mcpu=cortex-m0plus -march=armv6s-m -mthumb -fno-use-cxa-atexit"
		CACHE
		"STRING"
		"Flags used by the CXX compiler during all build types.")
set("CMAKE_CXX_FLAGS_DEBUG"
		"-Og -g -ggdb3"
		CACHE
		"STRING"
		"Flags used by the CXX compiler during DEBUG builds.")
set("CMAKE_CXX_FLAGS_MINSIZEREL"
		"-Os"
		CACHE
		"STRING"
		"Flags used by the CXX compiler during MINSIZEREL builds.")
set("CMAKE_CXX_FLAGS_RELEASE"
		"-O2"
		CACHE
		"STRING"
		"Flags used by the CXX compiler during RELEASE builds.")
set("CMAKE_CXX_FLAGS_RELWITHDEBINFO"
		"-O2 -g -ggdb3"
		CACHE
		"STRING"
		"Flags used by the CXX compiler during RELWITHDEBINFO builds.")
set("CMAKE_C_FLAGS"
		"-ffunction-sections -fdata-sections -Wall -Wextra -Wshadow -mcpu=cortex-m0plus -march=armv6s-m -mthumb"
		CACHE
		"STRING"
		"Flags used by the C compiler during all build types.")
set("CMAKE_C_FLAGS_DEBUG"
		"-Og -g -ggdb3"
		CACHE
		"STRING"
		"Flags used by the C compiler during DEBUG builds.")
set("CMAKE_C_FLAGS_MINSIZEREL"
		"-Os"
		CACHE
		"STRING"
		"Flags used by the C compiler during MINSIZEREL builds.")
set("CMAKE_C_FLAGS_RELEASE"
		"-O2"
		CACHE
		"STRING"
		"Flags used by the C compiler during RELEASE builds.")
set("CMAKE_C_FLAGS_RELWITHDEBINFO"
		"-O2 -g -ggdb3"
		CACHE
		"STRING"
		"Flags used by the C compiler during RELWITHDEBINFO builds.")
set("CMAKE_EXE_LINKER_FLAGS"
		"-Wl,--gc-sections"
		CACHE
		"STRING"
		"Flags used by the linker during all build types.")
set("CMAKE_EXE_LINKER_FLAGS_DEBUG"
		""
		CACHE
		"STRING"
		"Flags used by the linker during DEBUG builds.")
set("CMAKE_EXE_LINKER_FLAGS_MINSIZEREL"
		""
		CACHE
		"STRING"
		"Flags used by the linker during MINSIZEREL builds.")
set("CMAKE_EXE_LINKER_FLAGS_RELEASE"
		""
		CACHE
		"STRING"
		"Flags used by the linker during RELEASE builds.")
set("CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO"
		""
		CACHE
		"STRING"
		"Flags used by the linker during RELWITHDEBINFO builds.")
set("CMAKE_EXPORT_COMPILE_COMMANDS"
		"ON"
		CACHE
		"BOOL"
		"Enable/Disable output of compile commands during generation.")
set("CMAKE_MODULE_LINKER_FLAGS"
		""
		CACHE
		"STRING"
		"Flags used by the linker during the creation of modules during all build types.")
set("CMAKE_MODULE_LINKER_FLAGS_DEBUG"
		""
		CACHE
		"STRING"
		"Flags used by the linker during the creation of modules during DEBUG builds.")
set("CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL"
		""
		CACHE
		"STRING"
		"Flags used by the linker during the creation of modules during MINSIZEREL builds.")
set("CMAKE_MODULE_LINKER_FLAGS_RELEASE"
		""
		CACHE
		"STRING"
		"Flags used by the linker during the creation of modules during RELEASE builds.")
set("CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO"
		""
		CACHE
		"STRING"
		"Flags used by the linker during the creation of modules during RELWITHDEBINFO builds.")
set("CMAKE_SHARED_LINKER_FLAGS"
		""
		CACHE
		"STRING"
		"Flags used by the linker during the creation of shared libraries during all build types.")
set("CMAKE_SHARED_LINKER_FLAGS_DEBUG"
		""
		CACHE
		"STRING"
		"Flags used by the linker during the creation of shared libraries during DEBUG builds.")
set("CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL"
		""
		CACHE
		"STRING"
		"Flags used by the linker during the creation of shared libraries during MINSIZEREL builds.")
set("CMAKE_SHARED_LINKER_FLAGS_RELEASE"
		""
		CACHE
		"STRING"
		"Flags used by the linker during the creation of shared libraries during RELEASE builds.")
set("CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO"
		""
		CACHE
		"STRING"
		"Flags used by the linker during the creation of shared libraries during RELWITHDEBINFO builds.")
set("CMAKE_STATIC_LINKER_FLAGS"
		""
		CACHE
		"STRING"
		"Flags used by the linker during the creation of static libraries during all build types.")
set("CMAKE_STATIC_LINKER_FLAGS_DEBUG"
		""
		CACHE
		"STRING"
		"Flags used by the linker during the creation of static libraries during DEBUG builds.")
set("CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL"
		""
		CACHE
		"STRING"
		"Flags used by the linker during the creation of static libraries during MINSIZEREL builds.")
set("CMAKE_STATIC_LINKER_FLAGS_RELEASE"
		""
		CACHE
		"STRING"
		"Flags used by the linker during the creation of static libraries during RELEASE builds.")
set("CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO"
		""
		CACHE
		"STRING"
		"Flags used by the linker during the creation of static libraries during RELWITHDEBINFO builds.")
set("CMAKE_TOOLCHAIN_FILE"
		"${DISTORTOS_PATH}/source/board/ST_NUCLEO-G0B1RE/Toolchain-ST_NUCLEO-G0B1RE.cmake"
		CACHE
		"FILEPATH"
		"The CMake toolchain file")
set("CMAKE_VERBOSE_MAKEFILE"
		"OFF"
		CACHE
		"BOOL"
		"If this value is on, makefiles will be generated without the .SILENT directive, and all commands will be echoed to the console during the make.  This is useful for debugging only. With Visual Studio IDE projects all commands are done without /nologo.")
