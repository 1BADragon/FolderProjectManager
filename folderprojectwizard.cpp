/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "folderprojectwizard.h"
#include "folderprojectconstants.h"
#include "filesselectionwizardpage.h"

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

const char ConfigFileTemplate[] =
        "// Add predefined macros for your project here. For example:\n"
        "// #define THE_ANSWER 42\n";

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

    // second page
//    m_secondPage = new FilesSelectionWizardPage(this);
//    m_secondPage->setTitle(tr("File Selection"));
//    addPage(m_secondPage);
}

FilePath FolderProjectWizardDialog::filePath() const
{
    return m_firstPage->filePath();
}

FilePaths FolderProjectWizardDialog::selectedPaths() const
{
    return m_secondPage->selectedPaths();
}

FilePaths FolderProjectWizardDialog::selectedFiles() const
{
    return m_secondPage->selectedFiles();
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
    setId("F.Makefile");
    setDescription(tr("Imports existing folder as a project workspace.")
                   .arg(Core::Constants::IDE_DISPLAY_NAME));
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
    const QStringList paths = Utils::transform(wizard->selectedPaths(), &Utils::FilePath::toString);

    Utils::MimeType headerTy = Utils::mimeTypeForName(QLatin1String("text/x-chdr"));

    QStringList nameFilters = headerTy.globPatterns();

    QStringList includePaths;
    const QDir dir(projectPath.toString());
    foreach (const QString &path, paths) {
        QFileInfo fileInfo(path);
        QDir thisDir(fileInfo.absoluteFilePath());

        if (! thisDir.entryList(nameFilters, QDir::Files).isEmpty()) {
            QString relative = dir.relativeFilePath(path);
            if (relative.isEmpty())
                relative = QLatin1Char('.');
            includePaths.append(relative);
        }
    }
    includePaths.append(QString()); // ensure newline at EOF

    Core::GeneratedFile generatedCreatorFile(creatorFileName);
    generatedCreatorFile.setContents(QLatin1String("[General]\n"));
    generatedCreatorFile.setAttributes(Core::GeneratedFile::OpenProjectAttribute);

    QStringList sources = Utils::transform(wizard->selectedFiles(), &Utils::FilePath::toString);
    for (int i = 0; i < sources.length(); ++i)
        sources[i] = dir.relativeFilePath(sources[i]);
    Utils::sort(sources);
    sources.append(QString()); // ensure newline at EOF

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

} // namespace Internal
} // namespace GenericProjectManager
