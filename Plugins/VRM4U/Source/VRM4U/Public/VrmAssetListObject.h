// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "VrmAssetListObject.generated.h"

class UMaterialInterface;
class UTexture2D;
class USkeletalMesh;
class USkeleton;
class UVrmMetaObject;
class UVrmLicenseObject;
struct FReturnedData;
class UNodeMappingContainer;

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class VRM4U_API UVrmAssetListObject : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	void CopyMember(UVrmAssetListObject *dst) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InOption")
	bool bAssetSave;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InOption")
	bool bSkipMorphTarget=false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InMaterial_MToon_Lit")
	UMaterialInterface* BaseMToonLitOpaqueMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InMaterial_MToon_Lit")
	UMaterialInterface* BaseMToonLitTranslucentMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InMaterial_MToon_Lit")
	UMaterialInterface* OptMToonLitOpaqueMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InMaterial_MToon_Lit")
	UMaterialInterface* OptMToonLitOpaqueTwoSidedMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InMaterial_MToon_Lit")
	UMaterialInterface* OptMToonLitTranslucentMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InMaterial_MToon_Lit")
	UMaterialInterface* OptMToonLitTranslucentTwoSidedMaterial;

	// unlit

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InMaterial_MToon_unlit")
	UMaterialInterface* BaseMToonUnlitOpaqueMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InMaterial_MToon_unlit")
	UMaterialInterface* BaseMToonUnlitTranslucentMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InMaterial_MToon_Unlit")
	UMaterialInterface* OptMToonUnlitOpaqueMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InMaterial_MToon_Unlit")
	UMaterialInterface* OptMToonUnlitOpaqueTwoSidedMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InMaterial_MToon_Unlit")
	UMaterialInterface* OptMToonUnlitTranslucentMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InMaterial_MToon_Unlit")
	UMaterialInterface* OptMToonUnlitTranslucentTwoSidedMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InMaterial_MToon")
	UMaterialInterface* OptMToonOutlineMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InMaterial")
	UMaterialInterface* BaseUnlitOpaqueMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InMaterial")
	UMaterialInterface* BaseUnlitTranslucentMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InMaterial")
	UMaterialInterface* BasePBROpaqueMaterial;

	//

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out")
	//USkeleton* BaseSkeleton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out")
	USkeletalMesh* BaseSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out")
	UVrmMetaObject* VrmMetaObject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out")
	UVrmLicenseObject* VrmLicenseObject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out")
	USkeletalMesh* SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out")
	TArray<UTexture2D*> Textures;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out")
	TArray<UMaterialInterface*> Materials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out")
	TArray<UMaterialInterface*> OutlineMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out")
	UNodeMappingContainer *HumanoidRig;

	UPROPERTY()
	UPackage *Package;

	UPROPERTY()
	FString OrigFileName;

	UPROPERTY()
	FString BaseFileName;

	//FReturnedData *Result;

	UPROPERTY()
	USkeletalMesh* HumanoidSkeletalMesh;

	TMap<int, int> MaterialMergeTable;

};
