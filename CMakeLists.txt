cmake_minimum_required(VERSION 3.20)

# Set project
project(Honeydew VERSION 0.0.0)

set(EXEC_NAME hdw)

# Source files
set(EXEC_FILES 
	src/main.c)

# Force compiler to high warning levels
if(MSVC)
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

# Create executable and set up linking
add_executable(${EXEC_NAME} ${EXEC_FILES})

target_include_directories(${EXEC_NAME} PRIVATE ${LUA_DIR})
target_include_directories(${EXEC_NAME} PRIVATE "src")

target_link_libraries(${EXEC_NAME} m)
