cmake_minimum_required(VERSION 3.10)

project(FolderProjectManager)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)

find_package(QtCreator COMPONENTS Core REQUIRED)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
set(QtX Qt${QT_VERSION_MAJOR})

add_qtc_plugin(FolderProjectManager
  PLUGIN_DEPENDS
    QtCreator::Core
  DEPENDS
    ${QtX}::Widgets
    QtCreator::ExtensionSystem
    QtCreator::Utils
  SOURCES
    .github/workflows/build_cmake.yml
    .github/workflows/README.md
    README.md
    filesselectionwizardpage.cpp
    filesselectionwizardpage.h
    folderbuildconfiguration.cpp
    folderbuildconfiguration.h
    foldermakestep.cpp
    foldermakestep.h
    folderproject.cpp
    folderproject.h
    folderprojectconstants.h
    folderprojectfileseditor.cpp
    folderprojectfileseditor.h
    folderprojectmanager.qbs
    folderprojectplugin.cpp
    folderprojectplugin.h
    folderprojectwizard.cpp
    folderprojectwizard.h
)
