cmake_minimum_required(VERSION 4.1.1)
project(DaktCore VERSION 1.0.0 LANGUAGES CXX)

# ============================================================================
# Library Definition
# ============================================================================

add_library(DaktCore STATIC
    src/Platform.cpp
    src/String.cpp
    src/Memory.cpp
    src/Time.cpp
    src/Hash.cpp
    src/FileSystem.cpp
    src/Buffer.cpp
)

add_library(Dakt::Core ALIAS DaktCore)

# ============================================================================
# Include Directories
# ============================================================================

target_include_directories(DaktCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ============================================================================
# Compile Features
# ============================================================================

target_compile_features(DaktCore PUBLIC cxx_std_23)

# ============================================================================
# Platform-specific Configuration
# ============================================================================

if(WIN32)
    target_compile_definitions(DaktCore PUBLIC DAKT_PLATFORM_WINDOWS)
    target_link_libraries(DaktCore PRIVATE Shlwapi)
elseif(APPLE)
    target_compile_definitions(DaktCore PUBLIC DAKT_PLATFORM_MACOS)
elseif(UNIX)
    target_compile_definitions(DaktCore PUBLIC DAKT_PLATFORM_LINUX)
endif()

# Debug/Release definitions
target_compile_definitions(DaktCore PUBLIC
    $<$<CONFIG:Debug>:DAKT_DEBUG>
    $<$<CONFIG:Release>:DAKT_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:DAKT_RELEASE>
)

# ============================================================================
# Installation
# ============================================================================

include(GNUInstallDirs)

install(TARGETS DaktCore
    EXPORT DaktLibTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)