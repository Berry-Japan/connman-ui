# po/CMakeLists.txt
find_package(Gettext REQUIRED)

# The domain name for gettext (e.g., used by msgfmt to name .mo files)
# Ensure this matches the GETTEXT_PACKAGE defined in the main CMakeLists.txt
set(GETTEXT_DOMAIN "connman-ui")

# Find all .po files in the current (po) directory.
# This assumes your .po files are directly in the po/ directory,
# e.g., po/ja.po, po/fr.po, etc.
file(GLOB_RECURSE PO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.po")

if(PO_FILES)
    # The gettext_create_translations command will handle the compilation
    # of .po files into .mo files and their installation.
    # It automatically creates build targets.
    gettext_create_translations(
        ${GETTEXT_DOMAIN} # Use the defined domain
        ALL # Process all languages found
        PO_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" # Look for .po files in this directory
        DESTINATION_DIR "${CMAKE_INSTALL_LOCALEDIR}" # Install .mo files here (e.g., share/locale)
    )
    message(STATUS "Gettext translations configured for ${GETTEXT_DOMAIN}")
else()
    message(STATUS "No .po files found in ${CMAKE_CURRENT_SOURCE_DIR}. Skipping gettext translation build.")
endif()
