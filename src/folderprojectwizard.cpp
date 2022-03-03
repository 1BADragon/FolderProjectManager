#include "folderprojectwizard.h"
#include "folderprojectconstants.h"
#include "folderprojectsettings.h"

#include <coreplugin/icore.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectexplorericons.h>
#include <projectexplorer/customwizard/customwizard.h>

#include <app/app_version.h>
#include <utils/algorithm.h>
#include <utils/fileutils.h>
#include <utils/filewizardpage.h>
#include <utils/mimetypes/mimedatabase.h>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QPixmap>
#include <QStyle>

using namespace Utils;

namespace FolderProjectManager {
namespace Internal {

//////////////////////////////////////////////////////////////////////////////
//
// GenericProjectWizardDialog
//
//////////////////////////////////////////////////////////////////////////////

FolderProjectWizardDialog::FolderProjectWizardDialog(const Core::BaseFileWizardFactory *factory,
                                                       QWidget *parent) :
    Core::BaseFileWizard(factory, QVariantMap(), parent)
{
    setWindowTitle(tr("Import Folder As Project"));

    // first page
    m_firstPage = new Utils::FileWizardPage;
    m_firstPage->setTitle(tr("Project Name and Location"));
    m_firstPage->setFileNameLabel(tr("Project name:"));
    m_firstPage->setPathLabel(tr("Location:"));
    addPage(m_firstPage);
}

FilePath FolderProjectWizardDialog::filePath() const
{
    return m_firstPage->filePath();
}

void FolderProjectWizardDialog::setFilePath(const FilePath &path)
{
    m_firstPage->setFilePath(path);
}

QString FolderProjectWizardDialog::projectName() const
{
    return m_firstPage->fileName();
}

//////////////////////////////////////////////////////////////////////////////
//
// GenericProjectWizard
//
//////////////////////////////////////////////////////////////////////////////

FolderProjectWizard::FolderProjectWizard()
{
    setSupportedProjectTypes({Constants::FOLDERPROJECT_ID});
    setIcon(QIcon(":/media/foldericon.svg"));
    setDisplayName(tr("Import Folder Project"));
    setId("FolderProject");
    setDescription(tr("Imports existing folder as a project workspace."));
    setCategory(QLatin1String(ProjectExplorer::Constants::IMPORT_WIZARD_CATEGORY));
    setDisplayCategory(QLatin1String(ProjectExplorer::Constants::IMPORT_WIZARD_CATEGORY_DISPLAY));
    setFlags(Core::IWizardFactory::PlatformIndependent);
}

Core::BaseFileWizard *FolderProjectWizard::create(QWidget *parent,
                                                   const Core::WizardDialogParameters &parameters) const
{
    auto wizard = new FolderProjectWizardDialog(this, parent);

    wizard->setFilePath(parameters.defaultPath());

    foreach (QWizardPage *p, wizard->extensionPages())
        wizard->addPage(p);

    return wizard;
}

Core::GeneratedFiles FolderProjectWizard::generateFiles(const QWizard *w,
                                                         QString *errorMessage) const
{
    Q_UNUSED(errorMessage)

    auto wizard = qobject_cast<const FolderProjectWizardDialog *>(w);
    const FilePath projectPath = wizard->filePath();
    const QString projectName = wizard->projectName();
    const FilePath creatorFileName = projectPath.pathAppended(projectName + ".project");

    Core::GeneratedFile generatedCreatorFile(creatorFileName);
    generatedCreatorFile.setContents(projectDocument());
    generatedCreatorFile.setAttributes(Core::GeneratedFile::OpenProjectAttribute);

    Core::GeneratedFiles files;
    files.append(generatedCreatorFile);

    return files;
}

bool FolderProjectWizard::postGenerateFiles(const QWizard *w, const Core::GeneratedFiles &l,
                                             QString *errorMessage) const
{
    Q_UNUSED(w)
    return ProjectExplorer::CustomProjectWizard::postGenerateOpen(l, errorMessage);
}

QString FolderProjectWizard::projectDocument() const
{    
    return defaultSettingDocument();
}

} // namespace Internal
} // namespace GenericProjectManager
