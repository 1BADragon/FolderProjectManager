cmake_minimum_required(VERSION 3.10)

project(FolderProjectManager)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

find_package(QtCreator COMPONENTS Core REQUIRED)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
set(QtX Qt${QT_VERSION_MAJOR})

add_qtc_plugin(FolderProjectManager
  PLUGIN_DEPENDS
    QtCreator::Core
    QtCreator::ProjectExplorer
    QtCreator::QtSupport
    QtCreator::TextEditor
  DEPENDS
    ${QtX}::Widgets
    QtCreator::ExtensionSystem
    QtCreator::Utils
  SOURCES
    .github/workflows/build_cmake.yml
    .github/workflows/README.md
    README.md
    folderprojectmanager.qbs
    folderprojectmanager.qrc

    # Actual Source Files...    
    #filesselectionwizardpage.h filesselectionwizardpage.cpp
    folderbuildconfiguration.h folderbuildconfiguration.cpp
    foldermakestep.h foldermakestep.cpp
    folderproject.h folderproject.cpp
    folderprojectconstants.h    
    folderprojectfileseditor.h folderprojectfileseditor.cpp
    folderprojectplugin.h folderprojectplugin.cpp
    folderprojectwizard.h folderprojectwizard.cpp
    folderprojectsettings.h folderprojectsettings.cpp
    recursivefoldermonitor.h recursivefoldermonitor.cpp
)

set_property(TARGET FolderProjectManager
    APPEND
    PROPERTY ADDITIONAL_CLEAN_FILES ${PROJECT_BINARY_DIR}/FolderProjectManager.json)
