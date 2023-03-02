cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

project(cytos CXX)

if (WIN32)
    if(NOT DEFINED VCPKG_TARGET_TRIPLET)
        set(VCPKG_TARGET_TRIPLET "x64-windows")
    endif()
    
    include_directories(${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}-static/include/)
    # include_directories(${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/)

    message("Arch: ${VCPKG_TARGET_TRIPLET}, Build type: ${CMAKE_BUILD_TYPE}")
    
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        link_directories(${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib/)
        link_directories(${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}-static/lib/)
        link_directories(${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/)
    else()
        link_directories(${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}-static/lib/)
        link_directories(${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/)
    endif()

    find_package(Python3 ${PYTHON_VERSION} EXACT COMPONENTS NumPy)

    if(Python3_FOUND)
        message("Found Python3: ${Python3_EXECUTABLE}")
        message("Python3 version: ${Python3_VERSION}")
        message("NumPy version: ${Python3_NumPy_VERSION}")
    else()
        message(FATAL_ERROR "Python3 not found")
    endif()

    set(CMAKE_SHARED_LIBRARY_SUFFIX ".pyd")
    add_library(test SHARED "src/py/module.cpp")
    target_include_directories(test PRIVATE ${Python3_INCLUDE_DIRS} ${Python3_NumPy_INCLUDE_DIRS})
    target_link_libraries(test PRIVATE ${Python3_LIBRARIES})
else()
    include_directories($ENV{VCPKG_ROOT}/installed/x64-linux/include/)
    link_directories($ENV{VCPKG_ROOT}/installed/x64-linux/lib/)
    
    message("Build type: ${CMAKE_BUILD_TYPE}")
endif()

set(CMAKE_CXX_STANDARD 20)
message(VERBOSE CXX)

if (WIN32)
else()
    set(CMAKE_CXX_FLAGS "-Wno-format-security -march=native -Ofast")
endif()

set(SRC_FILES
    "src/misc/pool.cpp"
    "src/entity/control.cpp"
    "src/entity/handle.cpp"
    "src/entity/bot.cpp"
    "src/agent/manager.cpp"
    "src/agent/observe.cpp"
    "src/network/server.cpp"
    "src/network/server-open.cpp"
    "src/network/action-parser.cpp"
)

if (WIN32)
    add_executable(cytos ${SRC_FILES} "src/main.cpp")
    set_property(TARGET cytos PROPERTY CXX_STANDARD 20)

    target_link_libraries(cytos uSockets zlib libssl libcrypto libuv iphlpapi userenv psapi cairo pixman-1 libpng16)
else()
    add_executable(cytos ${SRC_FILES} "src/main.cpp")
    target_link_libraries(cytos uSockets ssl crypto dl z libuv pthread cairo png pixman-1)
endif()

