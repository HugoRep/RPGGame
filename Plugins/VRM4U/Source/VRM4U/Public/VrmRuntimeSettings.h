// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
//#include "VrmAssetListObject.h"
#include "VrmRuntimeSettings.generated.h"

/**
 * 
 */
UCLASS(config=Engine, defaultconfig)
class VRM4U_API UVrmRuntimeSettings : public UObject
{
	
	GENERATED_UCLASS_BODY()

		// Enables experimental *incomplete and unsupported* texture atlas groups that sprites can be assigned to
		//UPROPERTY(EditAnywhere, config, Category=Settings)
		//bool bEnableSpriteAtlasGroups;

	//
	UPROPERTY(config, EditAnywhere, Category = Settings, meta = (
		ConfigRestartRequired=true
		))
	uint32 bDropVRMFileEnable:1;

	UPROPERTY(EditAnywhere, Category = Settings)
	bool bAllowAllAssimpFormat = true;

	//UPROPERTY(config, EditAnywhere, Category = Settings, meta = (AllowedClasses = "VrmAssetListObject", ExactClass = false))

	// Asset List
	UPROPERTY(config, EditAnywhere, Category = Settings, meta = (
		AllowedClasses = "Object",
		ExactClass = false
		))
	FSoftObjectPath AssetListObject;


};
