// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Components/PoseableMeshComponent.h"
#include "VrmPoseableMeshComponent.generated.h"

/**
 * 
 */
UCLASS(meta=(BlueprintSpawnableComponent))
class VRM4U_API UVrmPoseableMeshComponent : public UPoseableMeshComponent
{
	GENERATED_UCLASS_BODY()
	
	
public:

	virtual void RefreshBoneTransforms(FActorComponentTickFunction* TickFunction = NULL) override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

};
