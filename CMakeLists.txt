cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 11)

set(CURL_STATICLIB    ON)
add_subdirectory(deps/cpr)

project(2chDownloader)
set(SOURCE_EXE main.cpp)
include_directories(deps/cpr/include ${CPR_INCLUDE_DIRS})

set(Boost_USE_STATIC_LIBS    ON)
find_package(Boost COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_executable(2chDownloader ${SOURCE_EXE})

target_link_libraries(2chDownloader
	${CPR_LIBRARIES}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
)