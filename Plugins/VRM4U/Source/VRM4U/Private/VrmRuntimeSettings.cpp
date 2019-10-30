// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#include "VrmRuntimeSettings.h"


UVrmRuntimeSettings::UVrmRuntimeSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bDropVRMFileEnable = false;
	AssetListObject.SetPath(TEXT("/VRM4U/VrmObjectListBP.VrmObjectListBP"));
}