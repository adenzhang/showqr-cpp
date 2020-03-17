project(qrencode)

set( SRC
    QRDataBlocks.h
    QrCode.cpp
    QrCode.hpp
)

include_directories( ./ )

add_library(qrencode
    ${SRC}
)

target_include_directories( qrencode PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
