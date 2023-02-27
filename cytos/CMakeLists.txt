cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

project (cytos CXX)

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
else()
    include_directories($ENV{VCPKG_ROOT}/installed/x64-linux/include/)
    link_directories($ENV{VCPKG_ROOT}/installed/x64-linux/lib/)
    
    message("Build type: ${CMAKE_BUILD_TYPE}")
endif()

set(CMAKE_CXX_STANDARD 17)
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
    set_property(TARGET cytos PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    target_link_libraries(cytos uSockets zlib libssl libcrypto libuv skia)
else()
    add_executable(cytos ${SRC_FILES} "src/main.cpp")
    target_link_libraries(cytos uSockets ssl crypto dl z libuv pthread)
endif()

