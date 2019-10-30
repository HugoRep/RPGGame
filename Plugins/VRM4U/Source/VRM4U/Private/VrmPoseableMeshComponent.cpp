// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#include "VrmPoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "VrmAnimInstance.h"




UVrmPoseableMeshComponent::UVrmPoseableMeshComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UVrmPoseableMeshComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	USkinnedMeshComponent* MPCPtr = MasterPoseComponent.Get();
	if (MPCPtr) {
		MorphTargetWeights = MPCPtr->MorphTargetWeights;
		ActiveMorphTargets = MPCPtr->ActiveMorphTargets;

		//const TMap<FName, float>& GetMorphTargetCurves() const { return MorphTargetCurves; }

		//auto &a = MPCPtr->GetMorphTargetCurves();
		//this->MorphTargetWeights
	}
}

void UVrmPoseableMeshComponent::RefreshBoneTransforms(FActorComponentTickFunction* TickFunction)
{
	Super::RefreshBoneTransforms(TickFunction);

	USkinnedMeshComponent* MPCPtr = MasterPoseComponent.Get();
	if (MPCPtr) {
		MorphTargetWeights = MPCPtr->MorphTargetWeights;
		ActiveMorphTargets = MPCPtr->ActiveMorphTargets;

		//const TMap<FName, float>& GetMorphTargetCurves() const { return MorphTargetCurves; }

		//auto &a = MPCPtr->GetMorphTargetCurves();
		//this->MorphTargetWeights





	}

}