// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#include "VrmBPFunctionLibrary.h"
#include "Materials/MaterialInterface.h"

#include "Engine/Engine.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/SkeletalMesh.h"
#include "Logging/MessageLog.h"
#include "Engine/Canvas.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/MorphTarget.h"
#include "Misc/EngineVersionComparison.h"
#include "AssetRegistryModule.h"
#include "ARFilter.h"

#include "Rendering/SkeletalMeshLODModel.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/SkeletalMeshRenderData.h"


//#include "VRM4U.h"

void UVrmBPFunctionLibrary::VRMTransMatrix(const FTransform &transform, TArray<FLinearColor> &matrix, TArray<FLinearColor> &matrix_inv){

	FMatrix m = transform.ToMatrixWithScale();
	FMatrix mi = transform.ToMatrixWithScale().Inverse();

	matrix.SetNum(4);
	matrix_inv.SetNum(4);

	for (int i = 0; i < 4; ++i) {
		matrix[i] = FLinearColor(m.M[i][0], m.M[i][1], m.M[i][2], m.M[i][3]);
		matrix_inv[i] = FLinearColor(mi.M[i][0], mi.M[i][1], mi.M[i][2], mi.M[i][3]);
	}

	return;
}

void UVrmBPFunctionLibrary::VRMGetMorphTargetList(const USkeletalMesh *target, TArray<FString> &morphTargetList) {
	morphTargetList.Empty();

	if (target == nullptr) {
		return;
	}
	for (const auto &a : target->MorphTargets) {
		morphTargetList.Add(a->GetName());
	}
}


void UVrmBPFunctionLibrary::VRMGetMaterialPropertyOverrides(const UMaterialInterface *Material, TEnumAsByte<EBlendMode> &BlendMode, TEnumAsByte<EMaterialShadingModel> &ShadingModel, bool &IsTwoSided, bool &IsMasked){
	if (Material == nullptr) {
		return;
	}
	BlendMode		= Material->GetBlendMode();
#if	UE_VERSION_OLDER_THAN(4,23,0)
	ShadingModel = Material->GetShadingModel();
#else
	ShadingModel = Material->GetShadingModels().GetFirstShadingModel();
#endif
	IsTwoSided		= Material->IsTwoSided();
	IsMasked		= Material->IsMasked();
}

void UVrmBPFunctionLibrary::VRMGetMobileMode(bool &IsMobile, bool &IsAndroid, bool &IsIOS) {
	IsMobile = false;
	IsAndroid = false;
	IsIOS = false;

#if PLATFORM_ANDROID
	IsMobile = true;
	IsAndroid = true;
#endif

#if PLATFORM_IOS
	IsMobile = true;
	IsIOS = true;
#endif

}



void UVrmBPFunctionLibrary::VRMDrawMaterialToRenderTarget(UObject* WorldContextObject, UTextureRenderTarget2D* TextureRenderTarget, UMaterialInterface* Material)
{
#if	UE_VERSION_OLDER_THAN(4,20,0)
#else
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	if (!World)
	{
		//FMessageLog("Blueprint").Warning(LOCTEXT("DrawMaterialToRenderTarget_InvalidWorldContextObject", "DrawMaterialToRenderTarget: WorldContextObject is not valid."));
	} else if (!Material)
	{
		//FMessageLog("Blueprint").Warning(LOCTEXT("DrawMaterialToRenderTarget_InvalidMaterial", "DrawMaterialToRenderTarget: Material must be non-null."));
	} else if (!TextureRenderTarget)
	{
		//FMessageLog("Blueprint").Warning(LOCTEXT("DrawMaterialToRenderTarget_InvalidTextureRenderTarget", "DrawMaterialToRenderTarget: TextureRenderTarget must be non-null."));
	} else if (!TextureRenderTarget->Resource)
	{
		//FMessageLog("Blueprint").Warning(LOCTEXT("DrawMaterialToRenderTarget_ReleasedTextureRenderTarget", "DrawMaterialToRenderTarget: render target has been released."));
	} else
	{
		UCanvas* Canvas = World->GetCanvasForDrawMaterialToRenderTarget();

		FCanvas RenderCanvas(
			TextureRenderTarget->GameThread_GetRenderTargetResource(),
			nullptr,
			World,
			World->FeatureLevel);

		Canvas->Init(TextureRenderTarget->SizeX, TextureRenderTarget->SizeY, nullptr, &RenderCanvas);
		Canvas->Update();



		TDrawEvent<FRHICommandList>* DrawMaterialToTargetEvent = new TDrawEvent<FRHICommandList>();

		FName RTName = TextureRenderTarget->GetFName();
		ENQUEUE_RENDER_COMMAND(BeginDrawEventCommand)(
			[RTName, DrawMaterialToTargetEvent](FRHICommandListImmediate& RHICmdList)
		{
			// Update resources that were marked for deferred update. This is important
			// in cases where the blueprint is invoked in the same frame that the render
			// target is created. Among other things, this will perform deferred render
			// target clears.
			FDeferredUpdateResource::UpdateResources(RHICmdList);

			BEGIN_DRAW_EVENTF(
				RHICmdList,
				DrawCanvasToTarget,
				(*DrawMaterialToTargetEvent),
				*RTName.ToString());
		});

		Canvas->K2_DrawMaterial(Material, FVector2D(0, 0), FVector2D(TextureRenderTarget->SizeX, TextureRenderTarget->SizeY), FVector2D(0, 0));

		RenderCanvas.Flush_GameThread();
		Canvas->Canvas = NULL;

		FTextureRenderTargetResource* RenderTargetResource = TextureRenderTarget->GameThread_GetRenderTargetResource();
		ENQUEUE_RENDER_COMMAND(CanvasRenderTargetResolveCommand)(
			[RenderTargetResource, DrawMaterialToTargetEvent](FRHICommandList& RHICmdList)
		{
			RHICmdList.CopyToResolveTarget(RenderTargetResource->GetRenderTargetTexture(), RenderTargetResource->TextureRHI, FResolveParams());
			STOP_DRAW_EVENT((*DrawMaterialToTargetEvent));
			delete DrawMaterialToTargetEvent;
		}
		);
	}
#endif
}

void UVrmBPFunctionLibrary::VRMChangeMaterialParent(UMaterialInstanceConstant *dst, UMaterialInterface* NewParent, USkeletalMesh *UseSkeletalMesh) {
	if (dst == nullptr) {
		return;
	}

	if (dst->Parent == NewParent) {
		return;
	}
	dst->MarkPackageDirty();

	if (UseSkeletalMesh) {
		UseSkeletalMesh->MarkPackageDirty();
	}

#if WITH_EDITOR
	dst->PreEditChange(NULL);
	dst->SetParentEditorOnly(NewParent);
	dst->PostEditChange();

	// remove dynamic materials
	for (TObjectIterator<UMaterialInstanceDynamic> Itr; Itr; ++Itr) {
		if (Itr->Parent == dst) {
			Itr->ConditionalBeginDestroy();
		}
	}

	if (UseSkeletalMesh) {
		UseSkeletalMesh->PreEditChange(NULL);
		UseSkeletalMesh->PostEditChange();
	}

#else
	dst->Parent = NewParent;
	dst->PostLoad();
#endif
}

UObject* UVrmBPFunctionLibrary::VRMDuplicateAsset(UObject *src, FString name, UObject *thisOwner) {
	if (src == nullptr) {
		return nullptr;
	}
	if (thisOwner == nullptr) {
		return nullptr;
	}

	auto *a = DuplicateObject<UObject>(src, thisOwner->GetOuter(), *name);
	return a;
}


void UVrmBPFunctionLibrary::VRMSetMaterial(USkeletalMesh *target, int no, UMaterialInterface *material) {
	if (target == nullptr) {
		return;
	}
	if (no < target->Materials.Num()) {
		target->Materials[no].MaterialInterface = material;
	}
}

void UVrmBPFunctionLibrary::VRMSetImportedBounds(USkeletalMesh *target, FVector min, FVector max) {
	if (target == nullptr) {
		return;
	}
	FBox BoundingBox(min, max);
	target->SetImportedBounds(FBoxSphereBounds(BoundingBox));
}

bool UVrmBPFunctionLibrary::VRMGetAssetsByPackageName(FName PackageName, TArray<FAssetData>& OutAssetData, bool bIncludeOnlyOnDiskAssets){

	OutAssetData.Empty();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	auto &AssetRegistry = AssetRegistryModule.Get();

	return AssetRegistry.GetAssetsByPackageName(PackageName, OutAssetData, bIncludeOnlyOnDiskAssets);
}

UTextureRenderTarget2D* UVrmBPFunctionLibrary::VRMCreateRenderTarget2D(UObject* WorldContextObject, int32 Width, int32 Height, ETextureRenderTargetFormat Format, FLinearColor ClearColor)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	if (Width > 0 && Height > 0 && World)
	{
		UTextureRenderTarget2D* NewRenderTarget2D = NewObject<UTextureRenderTarget2D>(WorldContextObject);
		check(NewRenderTarget2D);
		NewRenderTarget2D->RenderTargetFormat = Format;
		NewRenderTarget2D->ClearColor = ClearColor;
		NewRenderTarget2D->InitAutoFormat(Width, Height);
		NewRenderTarget2D->UpdateResourceImmediate(true);

		return NewRenderTarget2D;
	}

	return nullptr;
}

bool UVrmBPFunctionLibrary::VRMRenderingThreadEnable(bool bEnable) {
	if (GIsThreadedRendering)
	{
		if (bEnable == false) {
			StopRenderingThread();
			GUseThreadedRendering = false;
		}
	} else
	{
		if (bEnable == true) {
			GUseThreadedRendering = true;
			StartRenderingThread();
		}
	}
	return true;
}

bool UVrmBPFunctionLibrary::VRMGetShadowEnable(const USkeletalMesh *mesh, int MaterialIndex) {

	if (mesh == nullptr) {
		return false;
	}
	if (mesh->GetResourceForRendering() == nullptr) {
		return false;
	}

	const FSkeletalMeshLODRenderData &rd = mesh->GetResourceForRendering()->LODRenderData[0];

	bool bShadow = false;

	for (const auto &a : rd.RenderSections) {
		if (a.MaterialIndex != MaterialIndex) continue;
		if (a.bDisabled) continue;

		if (a.NumVertices == 0) return false;
		if (a.NumTriangles == 0) return false;

		bShadow |= a.bCastShadow;
	}

	return bShadow;
}
