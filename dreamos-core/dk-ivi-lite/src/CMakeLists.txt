cmake_minimum_required(VERSION 3.16)

project(untitled2 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-fpermissive")

find_package(Qt6 6.2 REQUIRED COMPONENTS Quick Network)


qt_add_executable(dk_ivi
    main/main.cpp
    main/config.cpp
    main/dkmanager_subprocess.cpp
    uss/uss.cpp
)

add_library(vapiclient STATIC
    library/vapiclient/vapiclient.cpp
)

qt_add_qml_module(dk_ivi
    URI untitled2
    VERSION 1.0
    QML_FILES
        main/main.qml
        main/settings.qml
        uss/uss.qml
    RESOURCES
        resource/icons/car.png
        resource/icons/seat.png
)

set_target_properties(dk_ivi PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Declare the option with OFF as the default.
message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    message(STATUS "Detected architecture: amd64 (x86_64)")
    set(TARGET_ARCH "amd64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    message(STATUS "Detected architecture: arm64 (aarch64)")
    set(TARGET_ARCH "arm64")
else()
    message(STATUS "Detected architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Check NATIVE_TESTING value.
if(NATIVE_TESTING)
    set(TARGET_ARCH "sk_3.8")
endif()

message(STATUS "TARGET_ARCH: ${TARGET_ARCH}")


# message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
# Create an imported target for libKuksaClient.so
add_library(KuksaClient SHARED IMPORTED)
# Replace the path below with the actual location of your libKuksaClient.so file.
set_target_properties(KuksaClient PROPERTIES 
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/library/target/${TARGET_ARCH}/libKuksaClient.so"
)

target_link_libraries(dk_ivi
    PRIVATE Qt6::Quick Qt6::Network KuksaClient vapiclient
)

install(TARGETS dk_ivi
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)