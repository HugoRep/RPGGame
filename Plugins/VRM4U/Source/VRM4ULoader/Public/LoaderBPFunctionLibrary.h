// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#pragma once
#include "ProceduralMeshComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VrmConvert.h"
#include "LoaderBPFunctionLibrary.generated.h"

UENUM(BlueprintType)
enum class EPathType : uint8
{
	Absolute,
	Relative
};


USTRUCT(BlueprintType)
struct FMeshInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
		TArray<FVector> Vertices;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
		TArray<uint32> Triangles;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
		TArray<int32> Triangles2;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
		TArray<FVector> Normals;

//	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
	TArray< TArray<FVector2D> > UV0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
		TArray<FLinearColor> VertexColors;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
		TArray<FProcMeshTangent> MeshTangents;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
		TArray<FVector> Tangents;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
		FTransform RelativeTransform;
};

USTRUCT(BlueprintType)
struct FReturnedData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
		bool bSuccess;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
		int32 NumMeshes;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
		TArray<FMeshInfo> meshInfo;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ReturnedData")
	TMap<struct aiMesh*, uint32_t> meshToIndex;
};




/**
 * 
 */
UCLASS()
class VRM4ULOADER_API ULoaderBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "VRM4U")
	static bool VRMSetLoadMaterialType(EVRMImportMaterialType type);

	UFUNCTION(BlueprintCallable,Category="VRM4U", meta = (DynamicOutputParam = "OutVrmAsset"))
	static bool LoadVRMFile(const class UVrmAssetListObject *InVrmAsset, class UVrmAssetListObject *&OutVrmAsset, FString filepath);

	static void SetImportMode(bool bImportMode, class UPackage *package);

	//static void SetCopySkeletalMeshAnimation(bool bImportMode, class UPackage *package);


	UFUNCTION(BlueprintCallable,Category="VRM4U", meta = (DynamicOutputParam = "output"))
	static bool VRMReTransformHumanoidBone(class USkeletalMeshComponent *targetHumanoidSkeleton, const class UVrmMetaObject *meta, const class USkeletalMeshComponent *displaySkeleton);

	///static void ReTransformHumanoidBone(USkeleton *targetHumanoidSkeleton, const UVrmMetaObject *meta, const USkeleton *displaySkeleton) {

	UFUNCTION(BlueprintCallable, Category = "VRM4U")
	static bool CopyPhysicsAsset(USkeletalMesh *dstMesh, const USkeletalMesh *srcMesh);

	static class UVrmLicenseObject *GetVRMMeta(FString filepath);

};
