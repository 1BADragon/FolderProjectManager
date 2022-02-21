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

#include "folderbuildconfiguration.h"

#include "foldermakestep.h"
#include "folderproject.h"
#include "folderprojectconstants.h"

#include <projectexplorer/buildinfo.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/target.h>

#include <qtsupport/qtkitinformation.h>

#include <utils/aspects.h>
#include <utils/pathchooser.h>
#include <utils/qtcassert.h>


using namespace ProjectExplorer;
using namespace Utils;

namespace FolderProjectManager {
namespace Internal {

FolderBuildConfiguration::FolderBuildConfiguration(Target *parent, Utils::Id id)
    : BuildConfiguration(parent, id)
{
    setConfigWidgetDisplayName(tr("Generic Manager"));
    setBuildDirectoryHistoryCompleter("Generic.BuildDir.History");

    setInitializer([this](const BuildInfo &) {
        buildSteps()->appendStep(Constants::GENERIC_MS_ID);
        cleanSteps()->appendStep(Constants::GENERIC_MS_ID);
        updateCacheAndEmitEnvironmentChanged();
    });

    updateCacheAndEmitEnvironmentChanged();
}


// GenericBuildConfigurationFactory

FolderBuildConfigurationFactory::FolderBuildConfigurationFactory()
{
    registerBuildConfiguration<FolderBuildConfiguration>
        ("FolderProjectManager.FolderBuildConfiguration");

    setSupportedProjectType(Constants::FOLDERPROJECT_ID);
    setSupportedProjectMimeTypeName(Constants::FOLDERMIMETYPE);

    setBuildGenerator([](const Kit *, const FilePath &projectPath, bool forSetup) {
        BuildInfo info;
        info.typeName = BuildConfiguration::tr("Build");
        info.buildDirectory = forSetup ? Project::projectDirectory(projectPath) : projectPath;

        if (forSetup)  {
            //: The name of the build configuration created by default for a generic project.
            info.displayName = BuildConfiguration::tr("Default");
        }

        return QList<BuildInfo>{info};
    });
}

void FolderBuildConfiguration::addToEnvironment(Utils::Environment &env) const
{
    QtSupport::QtKitAspect::addHostBinariesToPath(kit(), env);
}

} // namespace Internal
} // namespace GenericProjectManager
