cmake_minimum_required(VERSION 3.15)
project(LostFoundGUI CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Gui Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Gui Core)

include_directories(
    src
    src/models
    src/auth
    src/network
    src/components
    src/views
)

set(SOURCES
    src/main.cpp
    src/auth/AuthManager.cpp
    src/network/ApiClient.cpp
    src/components/NavbarWidget.cpp
    src/components/SidebarWidget.cpp
    src/components/ItemCardWidget.cpp
    src/components/StatCardWidget.cpp
    src/components/ToastWidget.cpp
    src/components/EmptyStateWidget.cpp
    src/views/HomeScreen.cpp
    src/views/BrowseScreen.cpp
    src/views/ReportScreen.cpp
    src/views/ItemDetailsDialog.cpp
    src/views/LoginScreen.cpp
    src/views/RegisterScreen.cpp
    src/views/DashboardScreen.cpp
    src/views/AdminScreen.cpp
    src/views/AccessDeniedScreen.cpp
    src/views/ProfileDialog.cpp
    src/views/ClaimFormDialog.cpp
    src/views/MyClaimsScreen.cpp
    src/views/AdminClaimReviewDialog.cpp
    src/views/MainWindow.cpp
    resources.qrc
)

set(HEADERS
    src/models/User.h
    src/models/Item.h
    src/models/Claim.h
    src/auth/AuthManager.h
    src/network/ApiClient.h
    src/components/AvatarHelper.h
    src/components/NavbarWidget.h
    src/components/SidebarWidget.h
    src/components/ItemCardWidget.h
    src/components/StatCardWidget.h
    src/components/ToastWidget.h
    src/components/EmptyStateWidget.h
    src/views/HomeScreen.h
    src/views/BrowseScreen.h
    src/views/ReportScreen.h
    src/views/ItemDetailsDialog.h
    src/views/LoginScreen.h
    src/views/RegisterScreen.h
    src/views/DashboardScreen.h
    src/views/AdminScreen.h
    src/views/AccessDeniedScreen.h
    src/views/ProfileDialog.h
    src/views/ClaimFormDialog.h
    src/views/MyClaimsScreen.h
    src/views/AdminClaimReviewDialog.h
    src/views/MainWindow.h
)

add_executable(LostFoundGUI ${SOURCES} ${HEADERS})

target_link_libraries(LostFoundGUI
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Core
)
