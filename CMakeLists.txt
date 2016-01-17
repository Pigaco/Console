project(pigaco)
cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
include(FindPkgConfig)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

SET(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/)
SET(HDR ${CMAKE_CURRENT_SOURCE_DIR}/include/pigaco/)
set(RSC ${CMAKE_CURRENT_SOURCE_DIR}/Data/ressources)
set(FOR ${CMAKE_CURRENT_SOURCE_DIR}/Data/forms)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(VERSION_MAJOR "1" CACHE STRING "Major-Version")
set(VERSION_MINOR "0" CACHE STRING "Minor-Version")
set(VERSION_PATCH "0" CACHE STRING "Patch-Version")
set(VERSION_TWEAK "0" CACHE STRING "Tweak-Version")
set(PIGACO_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK})

SET(HDRS
    ${HDR}/App.hpp
    ${HDR}/DirectoryScanner.hpp
    ${HDR}/Players.hpp
    ${HDR}/GameHostStarter.hpp
    ${HDR}/WebAdmin.hpp
    ${HDR}/WebNavigationBar.hpp
    ${HDR}/WebGames.hpp
    ${HDR}/WebPlayers.hpp
    ${HDR}/WebInputs.hpp

    ${HDR}/Game.hpp

    ${HDR}/packaging/Package.hpp
    ${HDR}/packaging/PackageManager.hpp
    ${HDR}/packaging/Version.hpp
)

SET(SRCS
    ${SRC}/App.cpp
    ${SRC}/DirectoryScanner.cpp
    ${SRC}/Players.cpp
    ${SRC}/GameHostStarter.cpp
    ${SRC}/WebAdmin.cpp
    ${SRC}/WebNavigationBar.cpp
    ${SRC}/WebGames.cpp
    ${SRC}/WebPlayers.cpp
    ${SRC}/WebInputs.cpp

    ${SRC}/Game.cpp

    ${SRC}/packaging/Package.cpp
    ${SRC}/packaging/PackageManager.cpp
    ${SRC}/packaging/Version.cpp
)

set(RSCS
    ${RSC}/main.qrc
)

set(FORS
    ${FOR}/MainMenu.qml
    ${FOR}/MainMenuHeader.qml
    ${FOR}/GameBox.qml

    ${FOR}/Pages/Store/Main.qml
    ${FOR}/Pages/Settings/Main.qml
    ${FOR}/Pages/Settings/Players.qml
    ${FOR}/Pages/Settings/PlayerDelegate.qml
)

# Find Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets)
find_package(Qt5Qml)
find_package(Qt5Quick)

add_executable(pigaco ${SRCS} ${HDRS} ${RSCS} ${FORS})

# Compatability with Wt and boost
ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

target_link_libraries(pigaco Qt5::Widgets Qt5::Qml Qt5::Quick)

find_package(Boost REQUIRED COMPONENTS system thread filesystem)

if(${Boost_FOUND})
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
    target_link_libraries(pigaco ${Boost_LIBRARIES})
endif()

target_link_libraries(pigaco pthread)

# Add sigc++2
find_package(SigC++ REQUIRED)
if(${SIGC++_FOUND})
    target_link_libraries(pigaco ${SIGC++_LIBRARY})
    include_directories(${SIGC++_INCLUDE_DIR})
elseif()
    message(STATUS "Did not find Sigc++2! Please install it before you try to compile again.")
endif()

find_package(GLM REQUIRED)
if(${GLM_FOUND})
    include_directories(${GLM_INCLUDE_DIR})
endif()

# Add yaml-cpp
find_package(YamlCpp REQUIRED)
if(${YAMLCPP_FOUND})
    target_link_libraries(pigaco ${YAMLCPP_LIBRARY})
    include_directories(${YAMLCPP_INCLUDE_DIR})
elseif()
    message(STATUS "Did not find Yaml-Cpp! Please install it before you try to compile again.")
endif()

find_package(QuaZip REQUIRED)
if(${QUAZIP_FOUND})
    target_link_libraries(pigaco ${QUAZIP_LIBRARIES})
    include_directories(${QUAZIP_INCLUDE_DIRS})
endif()

find_package(Crypto++)
if(${CRYPTO++_FOUND_TRUE})
    target_link_libraries(pigaco ${CRYPTO++_LIBRARY})
    include_directories(${CRYPTO++_INCLUDE_DIR})
endif()

# Link to Wt for built in webserver support.
find_package(Wt REQUIRED)
if(${Wt_FOUND})
    target_link_libraries(pigaco ${Wt_LIBRARY} ${Wt_HTTP_LIBRARY})
    include_directories(${Wt_INCLUDE_DIR})
endif()

find_package(Libpiga REQUIRED)
if(${Libpiga_FOUND})
    target_link_libraries(pigaco ${Libpiga_LIBRARIES})
    include_directories(${Libpiga_INCLUDE_DIRS})
endif()

add_custom_command(TARGET pigaco
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/distributor.py
                   COMMAND python
                   ARGS ${CMAKE_CURRENT_SOURCE_DIR}/distributor.py ${CMAKE_CURRENT_BINARY_DIR}/dist)

target_link_libraries(pigaco -lrt)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/piga_host_config.yml.in
    ${CMAKE_CURRENT_BINARY_DIR}/piga_host_config.yml)
configure_file(${CMAKE_MODULE_PATH}/Definitions.hpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/pigaco/Definitions.hpp)

include(CMakePackageConfigHelpers)
set_property(TARGET pigaco PROPERTY VERSION ${PIGACO_VERSION})
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/PigacoConfigVersion.cmake"
    VERSION ${PIGACO_VERSION}
    COMPATIBILITY AnyNewerVersion
)
install(TARGETS pigaco EXPORT PigacoTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
    RUNTIME DESTINATION bin)

# Data files
install(
    DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/Data
    DESTINATION ../var/piga)

# Configuration files
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/piga_host_config.yml
    DESTINATION ../etc)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Pigaco Console Suite")
set(CPACK_PACKAGE_VENDOR "Pigaco")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_PACKAGE_VERSION_TWEAK ${VERSION_TWEAK})
set(CPACK_DEBIAN_PACKAGE_NAME "piga-pigaco")
set(CPACK_PACKAGE_NAME "piga-pigaco")	
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Max Heisinger <mail@maximaximal.com>")
include(CPack)

# Activate C++11
list(APPEND CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	list(APPEND ${CMAKE_CXX_FLAGS} "-Wall -std=c++11 ${CMAKE_CXX_FLAGS}")
endif()
