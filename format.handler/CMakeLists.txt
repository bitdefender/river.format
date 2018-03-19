## format handler CmakeLists.txt

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -m32 -std=c++11 -D__cdecl=\"\" -D__stdcall=\"\"")

include_directories(${SMTLIB2_INSTALL}/include
    $ENV{RIVER_SDK_DIR}/include
    $ENV{YICES_INSTALL_DIR}/include)

add_library(format.handler STATIC
    smtlib2yices.c
    SmtParser.cpp
    BinFormat.cpp
    TextFormat.cpp
    )

link_directories(${SMTLIB2_INSTALL}/lib)
find_library(smtlib2parser libsmtlib2parser.a)

link_directories($ENV{YICES_INSTALL_DIR}/lib)
find_library(yices libyces.a)

target_link_libraries(format.handler
    smtlib2parser
    gmp
    gmpxx
    m
    yices)

install (TARGETS format.handler DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
