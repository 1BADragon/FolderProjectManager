#include "folderprojectfileseditor.h"
#include "folderprojectconstants.h"

#include <coreplugin/editormanager/editormanager.h>
#include <texteditor/texteditoractionhandler.h>
#include <texteditor/textdocument.h>

#include <QCoreApplication>

using namespace TextEditor;

namespace FolderProjectManager {
namespace Internal {

//
// ProjectFilesFactory
//

FolderFilesFactory::FolderFilesFactory()
{
    setId(Constants::FILES_EDITOR_ID);
    setDisplayName(QCoreApplication::translate("OpenWith::Editors", ".project Editor"));
    addMimeType(Constants::FOLDERMIMETYPE);

    setDocumentCreator([]() { return new TextDocument(Constants::FILES_EDITOR_ID); });
    setEditorActionHandlers(TextEditorActionHandler::None);
}

} // namespace Internal
} // namespace GenericProjectManager
