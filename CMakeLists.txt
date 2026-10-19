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

FetchContent_Declare(
    sqlite3
    GIT_REPOSITORY https://github.com/sqlite/sqlite.git
    GIT_TAG version-3.45.1
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(sqlite3)


# ==========================================
# BCRYPT
# ==========================================

# Required because libbcrypt uses an old
# CMake minimum version declaration

set(
    CMAKE_POLICY_VERSION_MINIMUM
    3.5
    CACHE STRING ""
    FORCE
)

FetchContent_Declare(
    bcrypt
    GIT_REPOSITORY https://github.com/rg3/libbcrypt.git
    GIT_TAG master
)

FetchContent_MakeAvailable(bcrypt)


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
# INCLUDE DIRECTORIES
# ==========================================

# This is important for:
# #include <bcrypt/BCrypt.hpp>

target_include_directories(
    LostFoundBackend
    PRIVATE
    ${bcrypt_SOURCE_DIR}/include
    C:/Users/Lenovo/anaconda3/Library/include
)


# ==========================================
# LINK LIBRARIES
# ==========================================

target_link_libraries(
    LostFoundBackend
    PRIVATE

    Crow::Crow
    C:/Users/Lenovo/anaconda3/Library/lib/sqlite3.lib
    asio
    bcrypt

    ws2_32
    mswsock
)

# ==========================================
# LOST & FOUND QT GUI APPLICATION
# ==========================================

option(BUILD_GUI "Build the Qt GUI" OFF)

if(BUILD_GUI)
    add_subdirectory(gui)
endif()
