// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "VrmMetaObject.generated.h"


USTRUCT(Blueprintable, BlueprintType)
struct VRM4U_API FVRMSpringMeta{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	float stiffiness = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	float gravityPower = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	FVector gravityDir = { 0,0,0 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	float dragForce = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	float hitRadius = 0.f;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	//int boneNum = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	TArray<int> bones;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	TArray<FString> boneNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	TArray<int> ColliderIndexArray;
};

USTRUCT(Blueprintable, BlueprintType)
struct VRM4U_API FVRMSpringColliderData {
	GENERATED_BODY()
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	FVector offset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	float radius = 0.f;
};

USTRUCT(Blueprintable, BlueprintType)
struct VRM4U_API FVRMColliderMeta {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	int bone;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	FString boneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	TArray<FVRMSpringColliderData> collider;
};


// BlendShape
USTRUCT(Blueprintable, BlueprintType)
struct VRM4U_API FVrmBlendShape{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	FString morphTargetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	FString nodeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	FString meshName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	int meshID=0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	int shapeIndex=0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	int weight=100;
};

USTRUCT(Blueprintable, BlueprintType)
struct VRM4U_API FVrmBlendShapeGroup {
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	FString name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	TArray<FVrmBlendShape> BlendShape;
};

//struct VRM4U_API FBVrmlendShapeGroup {
//	TArray<FVrmBlendShape> 
//};

UCLASS(Blueprintable, BlueprintType)
class VRM4U_API UVrmMetaObject : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	TMap<FString, FString> humanoidBoneTable;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	TArray<FVrmBlendShapeGroup> BlendShapeGroup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	TArray<FVRMSpringMeta> VRMSprintMeta;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	TArray<FVRMColliderMeta> VRMColliderMeta;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rendering)
	class USkeletalMesh *SkeletalMesh;

	
};
