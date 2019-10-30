// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/EngineTypes.h"
#include "Engine/TextureRenderTarget2D.h"
#include "VrmBPFunctionLibrary.generated.h"

class UTextureRenderTarget2D;
class UMaterialInstanceConstant;
/**
 * 
 */
UCLASS()
class VRM4U_API UVrmBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable,Category="VRM4U")
	static void VRMTransMatrix(const FTransform &transform, TArray<FLinearColor> &matrix, TArray<FLinearColor> &matrix_inv);

	UFUNCTION(BlueprintPure, Category = "VRM4U")
	static void VRMGetMorphTargetList(const USkeletalMesh *target, TArray<FString> &morphTargetList);

	UFUNCTION(BlueprintPure, Category = "VRM4U")
	static void VRMGetMaterialPropertyOverrides(const UMaterialInterface *Material, TEnumAsByte<EBlendMode> &BlendMode, TEnumAsByte<EMaterialShadingModel> &ShadingModel, bool &IsTwoSided, bool &IsMasked);

	UFUNCTION(BlueprintPure, Category = "VRM4U")
	static void VRMGetMobileMode(bool &IsMobile, bool &IsAndroid, bool &IsIOS);

	UFUNCTION(BlueprintCallable, Category = "Rendering", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static void VRMDrawMaterialToRenderTarget(UObject* WorldContextObject, UTextureRenderTarget2D* TextureRenderTarget, UMaterialInterface* Material);

	UFUNCTION(BlueprintCallable, Category = "VRM4U")
	static void VRMChangeMaterialParent(UMaterialInstanceConstant *dst, UMaterialInterface* NewParent, USkeletalMesh *UseSkeletalMesh);

	UFUNCTION(BlueprintCallable, Category = "VRM4U")
	static UObject* VRMDuplicateAsset(UObject *src, FString name, UObject *thisOwner);

	UFUNCTION(BlueprintCallable, Category = "VRM4U")
	static void VRMSetMaterial(USkeletalMesh *target, int no, UMaterialInterface *material);

	UFUNCTION(BlueprintCallable, Category = "VRM4U")
	static void VRMSetImportedBounds(USkeletalMesh *target, FVector min, FVector max);

	UFUNCTION(BlueprintCallable, Category = "VRM4U")
	static bool VRMGetAssetsByPackageName(FName PackageName, TArray<FAssetData>& OutAssetData, bool bIncludeOnlyOnDiskAssets = false);

	UFUNCTION(BlueprintCallable, Category = "VRM4U", meta = (WorldContext = "WorldContextObject"))
	static UTextureRenderTarget2D* VRMCreateRenderTarget2D(UObject* WorldContextObject, int32 Width = 256, int32 Height = 256, ETextureRenderTargetFormat Format = RTF_RGBA16f, FLinearColor ClearColor = FLinearColor::Black);

	UFUNCTION(BlueprintCallable, Category = "VRM4U", meta = (WorldContext = "WorldContextObject"))
	static bool VRMRenderingThreadEnable(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "VRM4U", meta = (WorldContext = "WorldContextObject"))
	static bool VRMGetShadowEnable(const USkeletalMesh *mesh, int MaterialIndex);
};
