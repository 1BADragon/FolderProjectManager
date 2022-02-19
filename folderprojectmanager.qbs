import qbs 1.0
import qbs.FileInfo

QtcPlugin {
    name: "FolderProjectManager"

    Depends { name: "Qt.widgets" }
    Depends { name: "Utils" }

    Depends { name: "Core" }
    Depends { name: "TextEditor" }
    Depends { name: "ProjectExplorer" }
    Depends { name: "QtSupport" }
    Depends { name: "app_version_header" }

    files: [
        "filesselectionwizardpage.cpp",
        "filesselectionwizardpage.h",
        "folderbuildconfiguration.cpp",
        "folderbuildconfiguration.h",
        "foldermakestep.cpp",
        "foldermakestep.h",
        "folderproject.cpp",
        "folderproject.h",
        "folderprojectconstants.h",
        "folderprojectfileseditor.cpp",
        "folderprojectfileseditor.h",
        "folderprojectplugin.cpp",
        "folderprojectplugin.h",
        "folderprojectwizard.cpp",
        "folderprojectwizard.h",

        "FolderIcon.svg"
    ]
}
