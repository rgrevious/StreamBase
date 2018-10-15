
set(SOURCES 

User.cpp
Event.cpp
EventManager.cpp
StreamBase.cpp
)


set(INCLUDE 
User.h
Event.h
EventManager.h
StreamBase.h
)


add_library(StreamBase STATIC ${SOURCES} ${INCLUDE})

add_definitions(-DEXPMODULE)