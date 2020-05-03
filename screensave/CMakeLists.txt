# Screensave

set(SCREENSAVE_SOURCE 
	lockoutwindow.cpp
    screensave.cpp
    textscreen.cpp
)

set(SCREENSAVE_HEADERS
	lockoutwindow.h
    screensave.h
    textscreen.h
)

add_executable(screensave ${SCREENSAVE_SOURCE} ${SCREENSAVE_HEADERS})
target_link_libraries(screensave ${CURSES_LIBRARIES} ${Boost_LIBRARIES})
