project(fltk-tests)
set(PROJNAME fltk-tests)

add_definitions( -std=c++17 )

#set(FLTKDIR /mnt/ldata/program/fltk)

set(INCLUDES ${INCLUDES} 
    ${CMAKE_CURRENT_SOURCE_DIR}
    #-- build with installed fltk
#    ${FLTKDIR}/include
#    ${FLTKDIR}/include/FL/images
)
include_directories(${INCLUDES} )
message( "***  INCLUDES: ${INCLUDES}")

set(FLTK_COMPILE_FLAGS -D_THREAD_SAFE -D_REENTRANT -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE)
set(FLTK_LINKLIBS -lm -lX11 -lXext -lpthread -lXfixes -ldl)
set(FLTK_STATICLIBS  ${FLTKDIR}/lib-debug/libfltk.a ${FLTKDIR}/lib-debug/libfltk_images.a)


#add_definitions( ${FLTK_COMPILE_FLAGS} )
#link_libraries( ${FLTK_STATICLIBS} ${FLTK_LINKLIBS} )

#add_executable( animated animated.cxx )
#add_executable( menubar-add menubar-add.cxx )
#add_executable( nativefilechooser-simple-app nativefilechooser-simple-app.cxx )
#add_executable( buttons buttons.cxx )
#add_executable( pack pack.cxx )
#add_executable( keyboard keyboard.cxx keyboard_ui.cxx )
#add_executable( tile tile.cxx )
add_executable( showqr showqr.cxx )

target_link_libraries( showqr
    fltk
    fltk_images
    qrencode
)
