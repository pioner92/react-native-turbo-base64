cmake_minimum_required(VERSION 3.4.1)
project(react-native-turbo-base64)

set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_STANDARD 17)


add_library(react-native-turbo-base64 SHARED
        ../cpp/react-native-turbo-base64.cpp
        ../cpp/base64.h
        cpp-adapter.cpp
)

include_directories(
            ../cpp
)

find_package(ReactAndroid REQUIRED CONFIG)


if (ReactAndroid_VERSION_MINOR GREATER_EQUAL 76)
    target_link_libraries(
            react-native-turbo-base64
            ReactAndroid::jsi
            ReactAndroid::reactnative
    )
else()
    target_link_libraries(
            react-native-turbo-base64
            ReactAndroid::jsi
            ReactAndroid::reactnativejni
)
endif()
