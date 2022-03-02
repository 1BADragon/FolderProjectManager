#ifndef __FOLDERPROJECTWIZARD_H__
#define __FOLDERPROJECTWIZARD_H__

#include <coreplugin/basefilewizard.h>
#include <coreplugin/basefilewizardfactory.h>
#include <utils/fileutils.h>
#include <utils/wizard.h>

namespace Utils { class FileWizardPage; }

namespace FolderProjectManager {
namespace Internal {

class FilesSelectionWizardPage;

class FolderProjectWizardDialog : public Core::BaseFileWizard
{
    Q_OBJECT

public:
    explicit FolderProjectWizardDialog(const Core::BaseFileWizardFactory *factory, QWidget *parent = nullptr);

    Utils::FilePath filePath() const;
    void setFilePath(const Utils::FilePath &path);
    Utils::FilePaths selectedFiles() const;
    Utils::FilePaths selectedPaths() const;

    QString projectName() const;

    Utils::FileWizardPage *m_firstPage;
};

class FolderProjectWizard : public Core::BaseFileWizardFactory
{
    Q_OBJECT

public:
    FolderProjectWizard();

protected:
    Core::BaseFileWizard *create(QWidget *parent, const Core::WizardDialogParameters &parameters) const override;
    Core::GeneratedFiles generateFiles(const QWizard *w, QString *errorMessage) const override;
    bool postGenerateFiles(const QWizard *w, const Core::GeneratedFiles &l,
                           QString *errorMessage) const override;

private:
    QString projectDocument() const;
};

} // namespace Internal
} // namespace GenericProjectManager

#endif
