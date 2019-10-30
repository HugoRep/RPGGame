// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"

#include "VrmAnimInstanceCopy.generated.h"

class UVrmAssetListObject;
struct FAnimNode_VrmSpringBone;

USTRUCT()
struct VRM4U_API FVrmAnimInstanceCopyProxy : public FAnimInstanceProxy {

public:
	GENERATED_BODY()

	float CurrentDeltaTime = 0.f;
	bool bIgnoreVRMSwingBone = false;
	TSharedPtr<FAnimNode_VrmSpringBone> SpringBoneNode;

	FVrmAnimInstanceCopyProxy();

	FVrmAnimInstanceCopyProxy(UAnimInstance* InAnimInstance);

	virtual void Initialize(UAnimInstance* InAnimInstance) override;
	virtual bool Evaluate(FPoseContext& Output) override;
	virtual void UpdateAnimationNode(float DeltaSeconds) override;
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class VRM4U_API UVrmAnimInstanceCopy : public UAnimInstance
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	USkeletalMeshComponent *SrcSkeletalMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	UVrmAssetListObject *SrcVrmAssetList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	UVrmAssetListObject *DstVrmAssetList;

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;

	FVrmAnimInstanceCopyProxy *myProxy = nullptr;
	bool bIgnoreVRMSwingBone = false;
public:
	virtual void NativeInitializeAnimation()override;
	// Native update override point. It is usually a good idea to simply gather data in this step and 
	// for the bulk of the work to be done in NativeUpdateAnimation.
	virtual void NativeUpdateAnimation(float DeltaSeconds)override;
	// Native Post Evaluate override point
	virtual void NativePostEvaluateAnimation()override;
	// Native Uninitialize override point
	virtual void NativeUninitializeAnimation()override;

	// Executed when begin play is called on the owning component
	virtual void NativeBeginPlay()override;

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetSkeletalMeshCopyData(UVrmAssetListObject *dstAssetList, USkeletalMeshComponent *srcSkeletalMesh, UVrmAssetListObject *srcAssetList);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetVrmSpringBoneParam(float gravityScale = 1.f, FVector gravityAdd = FVector::ZeroVector, float stiffinessScale = 1.f, float stiffinessAdd = 0.f);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetVrmSpringBoneBool(bool bIgnoreVrmSpringBone = false, bool bIgnorePhysicsCollision = false, bool bIgnoreVRMCollision = false);
};
