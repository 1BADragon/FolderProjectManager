#ifndef __FOLDERPROJECTFILESEDITOR_H__
#define __FOLDERPROJECTFILESEDITOR_H__

#include <texteditor/texteditor.h>

namespace FolderProjectManager {
namespace Internal {

class FolderFilesFactory : public TextEditor::TextEditorFactory
{
public:
    FolderFilesFactory();
};

} // namespace Internal
} // namespace GenericProjectManager

#endif
