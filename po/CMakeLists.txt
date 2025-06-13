# po/CMakeLists.txt
find_package(Gettext REQUIRED)

gettext_create_translations(
    ${PROJECT_NAME}
    ALL
    PO_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
    DESTINATION_DIR "${CMAKE_INSTALL_LOCALEDIR}"
)
