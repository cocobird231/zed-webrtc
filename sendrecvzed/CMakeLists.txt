cmake_minimum_required(VERSION 3.0.0)
project(sendrecvzed VERSION 0.1.0)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED IMPORTED_TARGET gstreamer-1.0 gstreamer-app-1.0 gstreamer-webrtc-1.0 gstreamer-sdp-1.0)
pkg_check_modules(Other REQUIRED IMPORTED_TARGET libsoup-2.4 json-glib-1.0)
#pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)
file(GLOB global_src
     "*.h"
     "*.c"
     "my_webrtc/*.h"
     "my_webrtc/*.c"
     "media/*.h"
     "media/*.c"
)

add_executable(${PROJECT_NAME} ${global_src})
target_link_libraries(${PROJECT_NAME} 
        PkgConfig::GST
        PkgConfig::Other
#        PkgConfig::X11
        )
#target_include_directories(${PROJECT_NAME} PUBLIC PkgConfig::GST PkgConfig::Other PkgConfig::X11)
target_include_directories(${PROJECT_NAME} PUBLIC PkgConfig::GST PkgConfig::Other )
