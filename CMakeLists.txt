cmake_minimum_required(VERSION 3.10)
project(QrGen)

#set(FLTK_SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/fltk)  # child project use it to include headers.
add_subdirectory( fltk )
add_subdirectory( qrencode )
add_subdirectory( showqr-fltk )
add_subdirectory( showqr-qt )

