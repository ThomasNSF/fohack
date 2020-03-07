# Fallout

set(FALLOUT_SOURCE 
    fallout.cpp
    gameboard.cpp
    gamedata.cpp
)

set(FALLOUT_HEADERS
    Fallout.h
    gameboard.h
    gamedata.h
)

add_executable(fallout ${FALLOUT_SOURCE} ${FALLOUT_HEADERS})
target_link_libraries(fallout ${CURSES_LIBRARIES} ${Boost_LIBRARIES})
