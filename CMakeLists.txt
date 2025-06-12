cmake_minimum_required(VERSION 3.5)

project(connman-ui VERSION 0.3 LANGUAGES C)

# Automake のサブディレクトリとオブジェクトの処理を再現
# CMake はデフォルトでこれらをより適切に処理します

# config.h の設定
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)
# config.h.cmake ファイルを作成する必要があります
# 例:
# #cmakedefine GETTEXT_PACKAGE "@GETTEXT_PACKAGE@"

# m4 ディレクトリは Automake 固有なので、CMake では不要です

# AM_SILENT_RULES は CMake の CMAKE_VERBOSE_MAKEFILE で制御できます
# set(CMAKE_VERBOSE_MAKEFILE OFF) # デフォルトは OFF

# MAINTAINER_MODE は CMake のビルドタイプで制御できます
# if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#     message(STATUS "Maintainer mode enabled")
# endif()

# デフォルトのプレフィックス
if (NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()

# PKG_PROG_PKG_CONFIG
find_package(PkgConfig REQUIRED)

# AC_SUBST 変数 (CMake では set で代用)
set(abs_top_srcdir "${CMAKE_CURRENT_SOURCE_DIR}")
set(abs_top_builddir "${CMAKE_CURRENT_BINARY_DIR}")

# C 言語の有効化 (project コマンドで設定済み)
# AC_PROG_CC, AM_PROG_CC_C_O, AC_PROG_INSTALL, AC_PROG_MKDIR_P は CMake が自動で処理

# Libtool の設定 (CMake は独自の共有ライブラリとモジュールの管理機能を持つため、通常は不要)
# add_library(SHARED ...) や add_library(MODULE ...) を使用します
# AC_DISABLE_STATIC はデフォルトで共有ライブラリが優先されるため、通常は不要

# IT_PROG_INTLTOOL
find_package(Intltool QUIET)
if (Intltool_FOUND)
    message(STATUS "Intltool found.")
else()
    message(WARNING "Intltool not found. Gettext files might need manual processing.")
endif()

# GETTEXT_PACKAGE
set(GETTEXT_PACKAGE "connman-ui")
add_definitions(-DGETTEXT_PACKAGE="${GETTEXT_PACKAGE}")
# AM_GLIB_GNU_GETTEXT は通常、find_package(Glib REQUIRED) と add_subdirectory(po) で処理します

# 最適化オプション
option(DISABLE_OPTIMIZATION "Disable code optimization through compiler" OFF)
if (DISABLE_OPTIMIZATION)
    string(APPEND CMAKE_C_FLAGS " -O0")
endif()

# デバッグオプション
option(ENABLE_DEBUG "Enable compiling with debugging information" OFF)
if (ENABLE_DEBUG AND CMAKE_C_COMPILER_WORKS) # ac_cv_prog_cc_g の代わり
    string(APPEND CMAKE_C_FLAGS " -g -Wall")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# PKG_CHECK_MODULES
pkg_check_modules(GTK REQUIRED gtk+-3.0>=3.0)
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.28)
pkg_check_modules(DBUS REQUIRED dbus-1>=1.4)

# GLib と D-Bus の CFLAGS をアプリケーション全体の CFLAGS に追加
add_compile_options(${GLIB_CFLAGS})
add_compile_options(${DBUS_CFLAGS})

# 定義
set(CUI_UI_PATH_REL "data/ui")
set(CUI_ICON_PATH_REL "data/icons")

# MAINTAINER_MODE のパス設定
if (CMAKE_BUILD_TYPE STREQUAL "Debug") # Debug ビルドを MAINTAINER_MODE と仮定
    set(CUI_UI_PATH "${abs_top_srcdir}/${CUI_UI_PATH_REL}")
    set(CUI_ICON_PATH "${abs_top_srcdir}/${CUI_ICON_PATH_REL}")
    set(LOCALEDIR "${abs_top_srcdir}/po")
else()
    set(CUI_UI_PATH "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME_LOWER}/${CUI_UI_PATH_REL}")
    set(CUI_ICON_PATH "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME_LOWER}/${CUI_ICON_PATH_REL}")
    set(LOCALEDIR "${CMAKE_INSTALL_LOCALEDIR}") # 通常は CMAKE_INSTALL_PREFIX/share/locale
endif()

add_definitions(
    -DCUI_UI_PATH="${CUI_UI_PATH}"
    -DCUI_ICON_PATH="${CUI_ICON_PATH}"
    -DLOCALEDIR="${LOCALEDIR}"
)

# ソースファイル
set(GDBUS_SOURCES
    gdbus/gdbus.h
    gdbus/mainloop.c
    gdbus/watch.c
    gdbus/object.c
    gdbus/polkit.c
)

set(COMMON_SOURCES
    lib/connman-interface.h
    lib/interface.c
    lib/dbus.c
    lib/manager.c
    lib/technology.c
    lib/service.c
    lib/agent.c
)

set(CONNMAN_UI_GTK_SOURCES
    ${GDBUS_SOURCES}
    ${COMMON_SOURCES}
    src/main.c
    src/tray.c
    src/left-menu.c
    src/right-menu.c
    src/gtktechnology.h
    src/gtktechnology.c
    src/gtkservice.h
    src/gtkservice.c
    src/agent_dialogs.c
    src/settings.c
    src/theme.c
    src/utils.c
)

add_executable(connman-ui-gtk ${CONNMAN_UI_GTK_SOURCES})

# インクルードディレクトリ
target_include_directories(connman-ui-gtk PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}/include" # builddir/include
    "${CMAKE_CURRENT_BINARY_DIR}/src"     # builddir/src
    "${CMAKE_CURRENT_SOURCE_DIR}/gdbus"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_options(connman-ui-gtk PUBLIC
    ${GTK_CFLAGS}
    ${GLIB_CFLAGS}
    ${DBUS_CFLAGS}
)

# ライブラリのリンク
target_link_libraries(connman-ui-gtk PUBLIC
    ${GTK_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${DBUS_LIBRARIES}
)

# データのインストール
set(UI_DATA
    data/ui/agent.ui
    data/ui/left_menu.ui
    data/ui/right_menu.ui
    data/ui/settings.ui
    data/ui/tray.ui
)

set(ICONS_DATA
    data/icons/gsm-3g-full.png
    data/icons/network-wired-symbolic.png
    data/icons/network-wireless-signal-excellent-symbolic.png
    data/icons/network-wireless-signal-good-symbolic.png
    data/icons/network-wireless-signal-weak-symbolic.png
    data/icons/network-wireless-signal-ok-symbolic.png
    data/icons/network-offline-symbolic.png
    data/icons/network-transmit-receive-symbolic.png
    data/icons/network-idle-symbolic.png
    data/icons/nm-adhoc.png
)

install(TARGETS connman-ui-gtk DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
install(FILES ${UI_DATA} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/connman_ui_gtk/ui")
install(FILES ${ICONS_DATA} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/connman_ui_gtk/icons")

# po ディレクトリの処理 (通常は gettext を使う)
#add_subdirectory(po) # po/CMakeLists.txt が必要

# ヘッダファイルのインストール (Makefile.am の include_HEADERS, noinst_HEADERS を考慮)
# 現状では具体的なヘッダファイルがリストアップされていないため、必要に応じて追加
# install(FILES include/version.h DESTINATION include) # config.h と共に生成される

# EXTRA_DIST は配布物に含まれるファイルなので、CMake のビルドシステムには直接関係しません
