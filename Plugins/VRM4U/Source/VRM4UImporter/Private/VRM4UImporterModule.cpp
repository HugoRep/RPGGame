
#include "CoreMinimal.h"
#include "VRM4UImporterLog.h"
#include "Modules/ModuleManager.h"
#include "Internationalization/Internationalization.h"

#define LOCTEXT_NAMESPACE "VRM4UImporter"

DEFINE_LOG_CATEGORY(LogVRM4UImporter);

//////////////////////////////////////////////////////////////////////////
// FSpriterImporterModule

class FVRM4UImporterModule : public FDefaultModuleImpl
{
public:
	virtual void StartupModule() override
	{
	}

	virtual void ShutdownModule() override
	{
	}
};

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_MODULE(FVRM4UImporterModule, VRM4UImporter);

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
