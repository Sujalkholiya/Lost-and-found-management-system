cmake_minimum_required(VERSION 3.15)

project(LOST_FOUND_BACKEND)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)


# ==========================================
# ASIO - Required for Crow
# ==========================================

FetchContent_Declare(
    Asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-28-0
)

FetchContent_MakeAvailable(Asio)

set(
    ASIO_INCLUDE_DIR
    ${asio_SOURCE_DIR}/asio/include
    CACHE PATH ""
    FORCE
)

add_library(asio INTERFACE)

target_include_directories(
    asio
    INTERFACE
    ${ASIO_INCLUDE_DIR}
)

target_compile_definitions(
    asio
    INTERFACE
    ASIO_STANDALONE
)


# ==========================================
# CROW
# ==========================================

set(CROW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CROW_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.2.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(Crow)


# ==========================================
# SQLITE
# ==========================================

# ==========================================
# SQLITE
# ==========================================

find_package(SQLite3 REQUIRED)


# ==========================================



# ==========================================
# LOST & FOUND BACKEND APPLICATION
# ==========================================

add_executable(
    LostFoundBackend

    src/main.cpp

    src/config/database.cpp

    src/controllers/userController.cpp
    src/controllers/itemController.cpp
    src/controllers/claimController.cpp

    src/middleware/authMiddleware.cpp

    src/routes/userRoutes.cpp
    src/routes/itemRoutes.cpp
    src/routes/claimRoutes.cpp
)




# ==========================================
# LINK LIBRARIES
# ==========================================

target_link_libraries(
    LostFoundBackend
    PRIVATE
    Crow::Crow
    SQLite::SQLite3
    asio
    
)

# ==========================================
# LOST & FOUND QT GUI APPLICATION
# ==========================================

option(BUILD_GUI "Build the Qt GUI" OFF)

if(BUILD_GUI)
    add_subdirectory(gui)
endif()
