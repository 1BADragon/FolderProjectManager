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
    README.md
    BUILDING.md
    folderprojectmanager.qrc

    # Actual Source Files...    
    src/folderbuildconfiguration.h src/folderbuildconfiguration.cpp
    src/foldermakestep.h src/foldermakestep.cpp
    src/folderproject.h src/folderproject.cpp
    src/folderprojectconstants.h
    src/folderprojectfileseditor.h src/folderprojectfileseditor.cpp
    src/folderprojectplugin.h src/folderprojectplugin.cpp
    src/folderprojectwizard.h src/folderprojectwizard.cpp
    src/folderprojectsettings.h src/folderprojectsettings.cpp
    src/asyncfoldermonitor.h src/asyncfoldermonitor.cpp
    src/asyncfoldermonitorworker.h src/asyncfoldermonitorworker.cpp
)

set_property(TARGET FolderProjectManager
    APPEND
    PROPERTY ADDITIONAL_CLEAN_FILES ${PROJECT_BINARY_DIR}/FolderProjectManager.json)
