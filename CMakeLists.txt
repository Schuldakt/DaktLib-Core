cmake_minimum_required(VERSION 4.2.1)

project(DaktLib-Core VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(DAKTCORE_BUILD_IMPL "Build default runtime implementations" ON)
option(DAKTCORE_WARNINGS "Enable strict warnings" ON)

set(DaktCore_public_headers
	include/dakt/core/Core.hpp
	include/dakt/core/concepts/CoreConcepts.hpp
	include/dakt/core/interfaces/IAllocator.hpp
	include/dakt/core/interfaces/IEventBus.hpp
	include/dakt/core/interfaces/ILogger.hpp
	include/dakt/core/interfaces/IRegionProvider.hpp
	include/dakt/core/interfaces/ISerializable.hpp
	include/dakt/core/types/Result.hpp
	include/dakt/core/types/Span.hpp
	include/dakt/core/types/StringView.hpp
	include/dakt/core/logging/NullLogger.hpp
	include/dakt/core/memory/SystemAllocator.hpp
)

add_library(DaktCore INTERFACE)
add_library(Dakt::Core ALIAS DaktCore)

target_include_directories(DaktCore INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

target_sources(DaktCore INTERFACE
	FILE_SET HEADERS
	BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
	FILES ${DaktCore_public_headers}
)

target_compile_features(DaktCore INTERFACE cxx_std_23)

if(DAKTCORE_WARNINGS)
	if(MSVC)
		target_compile_options(DaktCore INTERFACE /W4 /permissive- /Zc:__cplusplus)
	else()
		target_compile_options(DaktCore INTERFACE -Wall -Wextra -Wpedantic)
	endif()
endif()

if(DAKTCORE_BUILD_IMPL)
	set(DaktCore_impl_sources
		src/logging/NullLogger.cpp
		src/memory/SystemAllocator.cpp
	)

	add_library(DaktCoreImpl STATIC ${DaktCore_impl_sources})
	target_link_libraries(DaktCoreImpl PRIVATE DaktCore)
	target_compile_features(DaktCoreImpl PRIVATE cxx_std_23)
	target_include_directories(DaktCoreImpl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

	if(DAKTCORE_WARNINGS)
		if(MSVC)
			target_compile_options(DaktCoreImpl PRIVATE /W4 /permissive- /Zc:__cplusplus)
		else()
			target_compile_options(DaktCoreImpl PRIVATE -Wall -Wextra -Wpedantic)
		endif()
	endif()
endif()

include(GNUInstallDirs)

install(TARGETS DaktCore
	EXPORT DaktCoreTargets
	FILE_SET HEADERS
)

export(EXPORT DaktCoreTargets
	FILE ${CMAKE_CURRENT_BINARY_DIR}/DaktCoreTargets.cmake
	NAMESPACE Dakt::
)
