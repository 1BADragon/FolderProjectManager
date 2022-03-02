#ifndef __FOLDERMAKESTEP_H__
#define __FOLDERMAKESTEP_H__

#include <projectexplorer/makestep.h>

namespace FolderProjectManager {
namespace Internal {

class FolderMakeStepFactory final : public ProjectExplorer::BuildStepFactory
{
public:
    FolderMakeStepFactory();
};

} // namespace Internal
} // namespace GenericProjectManager

#endif
