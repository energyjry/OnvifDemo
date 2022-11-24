project(OnvifDemo)
cmake_minimum_required(VERSION 2.8)

#############################################
if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -Wall")
endif()

include( CheckCXXCompilerFlag )
check_cxx_compiler_flag( "-std=c++11"   COMPILER_SUPPORTS_CXX11 )
if( COMPILER_SUPPORTS_CXX11 )
    if( CMAKE_COMPILER_IS_GNUCXX )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11" )
    else()
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
    endif()
endif()
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-use-cxa-atexit -fexceptions" )

############################################

aux_source_directory(. DIR_SRCS)
aux_source_directory(onvif DIR_SRCS)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/onvif)

add_executable(OnvifDemo ${DIR_SRCS})

set(INC_DIR /usr/local/include)
set(LINK_DIR /usr/local/lib)
include_directories(${INC_DIR})
link_directories(${LINK_DIR})
target_link_libraries(${PROJECT_NAME}
        crypto
        ssl
    )

add_definitions(-DWITH_NONAMESPACES -DWITH_DOM -DWITH_OPENSSL)

