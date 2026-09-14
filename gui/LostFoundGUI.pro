QT += core gui widgets network

CONFIG += c++17

TARGET = LostFoundGUI
TEMPLATE = app

INCLUDEPATH += src \
               src/models \
               src/auth \
               src/network \
               src/components \
               src/views

SOURCES += \
    src/main.cpp \
    src/auth/AuthManager.cpp \
    src/network/ApiClient.cpp \
    src/components/NavbarWidget.cpp \
    src/components/SidebarWidget.cpp \
    src/components/ProfileImageManager.cpp \
    src/components/ItemCardWidget.cpp \
    src/components/StatCardWidget.cpp \
    src/components/ToastWidget.cpp \
    src/components/EmptyStateWidget.cpp \
    src/views/HomeScreen.cpp \
    src/views/BrowseScreen.cpp \
    src/views/ReportScreen.cpp \
    src/views/ItemDetailsDialog.cpp \
    src/views/LoginScreen.cpp \
    src/views/RegisterScreen.cpp \
    src/views/DashboardScreen.cpp \
    src/views/AdminScreen.cpp \
    src/views/AccessDeniedScreen.cpp \
    src/views/ProfileDialog.cpp \
    src/views/ClaimFormDialog.cpp \
    src/views/MyClaimsScreen.cpp \
    src/views/AdminClaimReviewDialog.cpp \
    src/views/MainWindow.cpp

HEADERS += \
    src/models/User.h \
    src/models/Item.h \
    src/models/Claim.h \
    src/models/CampusLocations.h \
    src/auth/AuthManager.h \
    src/network/ApiClient.h \
    src/network/ImageLoader.h \
    src/components/AvatarHelper.h \
    src/components/ProfileImageManager.h \
    src/components/Theme.h \
    src/components/NavbarWidget.h \
    src/components/SidebarWidget.h \
    src/components/ItemCardWidget.h \
    src/components/StatCardWidget.h \
    src/components/ToastWidget.h \
    src/components/EmptyStateWidget.h \
    src/views/HomeScreen.h \
    src/views/BrowseScreen.h \
    src/views/ReportScreen.h \
    src/views/ItemDetailsDialog.h \
    src/views/LoginScreen.h \
    src/views/RegisterScreen.h \
    src/views/DashboardScreen.h \
    src/views/AdminScreen.h \
    src/views/AccessDeniedScreen.h \
    src/views/ProfileDialog.h \
    src/views/ClaimFormDialog.h \
    src/views/MyClaimsScreen.h \
    src/views/AdminClaimReviewDialog.h \
    src/views/MainWindow.h

RESOURCES += \
    resources.qrc