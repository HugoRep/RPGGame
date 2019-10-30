// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"

#include "VrmAnimInstance.generated.h"

class UVrmMetaObject;
class USkeleton;

UENUM(BlueprintType)
enum class EVrmMorphGroupType : uint8
{
	MT_Neutoral		UMETA(DisplayName="00_Neutoral"),
	MT_A			UMETA(DisplayName="01_A"),
	MT_I			UMETA(DisplayName="02_I"),
	MT_U			UMETA(DisplayName="03_U"),
	MT_E			UMETA(DisplayName="04_E"),
	MT_O			UMETA(DisplayName="05_O"),
	MT_Blink		UMETA(DisplayName="06_Blink"),
	MT_Joy			UMETA(DisplayName="07_Joy"),
	MT_Angry		UMETA(DisplayName="08_Angry"),
	MT_Sorrow		UMETA(DisplayName="09_Sorrow"),
	MT_Fun			UMETA(DisplayName="10_Fun"),
	MT_LookUp		UMETA(DisplayName="11_LookUp"),
	MT_LookDown		UMETA(DisplayName="12_LookDown"),
	MT_LookLeft		UMETA(DisplayName="13_LookLeft"),
	MT_LookRight	UMETA(DisplayName="14_LookRight"),
	MT_Blink_L		UMETA(DisplayName="15_Blink_L"),
	MT_Blink_R		UMETA(DisplayName="16_Blink_R"),
	Num				UMETA(Hidden)
};


USTRUCT()
struct VRM4U_API FVrmAnimInstanceProxy : public FAnimInstanceProxy {

public:
	GENERATED_BODY()

		FVrmAnimInstanceProxy()
	{
	}

	FVrmAnimInstanceProxy(UAnimInstance* InAnimInstance)
		: FAnimInstanceProxy(InAnimInstance)
	{
	}

	virtual void Initialize(UAnimInstance* InAnimInstance) override;
	virtual bool Evaluate(FPoseContext& Output) override;
	virtual void UpdateAnimationNode(float DeltaSeconds) override;
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class VRM4U_API UVrmAnimInstance : public UAnimInstance
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	UVrmMetaObject *MetaObject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	USkeleton *BaseSkeleton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	USkeletalMeshComponent *BaseSkeletalMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tracking)
	FTransform TransHandLeft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tracking)
	FTransform TransHandRight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tracking)
	FTransform TransHead;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tracking)
	USceneComponent *ComponentHandLeft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tracking)
	USceneComponent *ComponentHandRight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tracking)
	USceneComponent *ComponentHead;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tracking)
		USceneComponent *ComponentHandJointTargetLeft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tracking)
		USceneComponent *ComponentHandJointTargetRight;

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
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

	//virtual USkeleton* GetTargetSkeleton() const override;

	UFUNCTION(BlueprintCallable, Category="Animation")
	void SetMorphTargetVRM(EVrmMorphGroupType type, float Value);

	
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetVrmData(USkeletalMeshComponent *baseSkeletalMesh, UVrmMetaObject *meta);

};
