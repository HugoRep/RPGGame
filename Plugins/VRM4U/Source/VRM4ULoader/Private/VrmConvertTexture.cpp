// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#include "VrmConvertTexture.h"
#include "VrmConvert.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>
#include <assimp/vrm/vrmmeta.h>

#include "Materials/MaterialExpressionTextureSampleParameter2D.h"

#include "Modules/ModuleManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "PixelFormat.h"
#include "RenderUtils.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceConstant.h"
#include "VrmAssetListObject.h"
#include "Async/ParallelFor.h"
#include "UObject/UObjectHash.h"

namespace {
	void LocalMaterialSetParent(UMaterialInstanceConstant *material, UMaterialInterface *parent) {
#if WITH_EDITOR
		if (VRMConverter::IsImportMode()) {
			material->SetParentEditorOnly(parent);
		} else {
			material->Parent = parent;
		}
#else
		material->Parent = parent;
#endif
	}

	void LocalTextureSet(UMaterialInstanceConstant *dm, FName name, UTexture2D * tex) {
#if WITH_EDITOR
		if (VRMConverter::IsImportMode()) {
			dm->SetTextureParameterValueEditorOnly(name, tex);
		}else{
			FTextureParameterValue *v = new (dm->TextureParameterValues) FTextureParameterValue();
			v->ParameterInfo.Index = INDEX_NONE;
			v->ParameterInfo.Name = name;
			v->ParameterInfo.Association = EMaterialParameterAssociation::GlobalParameter;
			v->ParameterValue = tex;
}
#else
		FTextureParameterValue *v = new (dm->TextureParameterValues) FTextureParameterValue();
		v->ParameterInfo.Index = INDEX_NONE;
		v->ParameterInfo.Name = name;
		v->ParameterInfo.Association = EMaterialParameterAssociation::GlobalParameter;
		v->ParameterValue = tex;
#endif
	}

	void LocalScalarParameterSet(UMaterialInstanceConstant *dm, FName name, float f) {

#if WITH_EDITOR
		if (VRMConverter::IsImportMode()) {
			dm->SetScalarParameterValueEditorOnly(name, f);
		} else {
			FScalarParameterValue *v = new (dm->ScalarParameterValues) FScalarParameterValue();
			v->ParameterInfo.Index = INDEX_NONE;
			v->ParameterInfo.Name = name;
			v->ParameterInfo.Association = EMaterialParameterAssociation::GlobalParameter;
			v->ParameterValue = f;
		}
#else
		FScalarParameterValue *v = new (dm->ScalarParameterValues) FScalarParameterValue();
		v->ParameterInfo.Index = INDEX_NONE;
		v->ParameterInfo.Name = name;
		v->ParameterInfo.Association = EMaterialParameterAssociation::GlobalParameter;
		v->ParameterValue = f;
#endif
	}

	void LocalVectorParameterSet(UMaterialInstanceConstant *dm, FName name, FLinearColor c) {

#if WITH_EDITOR
		if (VRMConverter::IsImportMode()) {
			dm->SetVectorParameterValueEditorOnly(name, c);
		} else {
			FVectorParameterValue *v = new (dm->VectorParameterValues) FVectorParameterValue();
			v->ParameterInfo.Index = INDEX_NONE;
			v->ParameterInfo.Name = name;
			v->ParameterInfo.Association = EMaterialParameterAssociation::GlobalParameter;
			v->ParameterValue = c;
		}
#else
		FVectorParameterValue *v = new (dm->VectorParameterValues) FVectorParameterValue();
		v->ParameterInfo.Index = INDEX_NONE;
		v->ParameterInfo.Name = name;
		v->ParameterInfo.Association = EMaterialParameterAssociation::GlobalParameter;
		v->ParameterValue = c;
#endif
	}


	void LocalMaterialFinishParam(UMaterialInstanceConstant *material) {
#if WITH_EDITOR
		if (VRMConverter::IsImportMode()) {
			material->PreEditChange(NULL);
			material->PostEditChange();
		} else {
			material->PostLoad();
		}
#else
		material->PostLoad();
#endif
	}


	bool createAndAddMaterial(UMaterialInstanceConstant *dm, int matIndex, UVrmAssetListObject *vrmAssetList, const aiScene *mScenePtr) {
		auto i = matIndex;
		const VRM::VRMMetadata *meta = static_cast<const VRM::VRMMetadata*>(mScenePtr->mVRMMeta);

		if (i >= meta->materialNum) {
			return false;
		}
		auto &vrmMat = meta->material[i];
		{
			struct TT {
				FString key;
				float* value;
			};
			TT table[] = {
				{TEXT("_Color"),			vrmMat.vectorProperties._Color},
				{TEXT("_ShadeColor"),	vrmMat.vectorProperties._ShadeColor},
				{TEXT("_MainTex"),		vrmMat.vectorProperties._MainTex},
				{TEXT("_ShadeTexture"),	vrmMat.vectorProperties._ShadeTexture},
				{TEXT("_BumpMap"),				vrmMat.vectorProperties._BumpMap},
				{TEXT("_ReceiveShadowTexture"),	vrmMat.vectorProperties._ReceiveShadowTexture},
				{TEXT("_ShadingGradeTexture"),		vrmMat.vectorProperties._ShadingGradeTexture},
				{TEXT("_RimColor"),					vrmMat.vectorProperties._RimColor},
				{TEXT("_RimTexture"),				vrmMat.vectorProperties._RimTexture},
				{TEXT("_SphereAdd"),				vrmMat.vectorProperties._SphereAdd},
				{TEXT("_EmissionColor"),			vrmMat.vectorProperties._EmissionColor},
				{TEXT("_EmissionMap"),			vrmMat.vectorProperties._EmissionMap},
				{TEXT("_OutlineWidthTexture"),	vrmMat.vectorProperties._OutlineWidthTexture},
				{TEXT("_OutlineColor"),			vrmMat.vectorProperties._OutlineColor},
				{TEXT("_UvAnimMaskTexture"),	vrmMat.vectorProperties._UvAnimMaskTexture},
			};

			for (auto &t : table) {
				LocalVectorParameterSet(dm, *(TEXT("mtoon") + t.key), FLinearColor(t.value[0], t.value[1], t.value[2], t.value[3]));

				//FVectorParameterValue *v = new (dm->VectorParameterValues) FVectorParameterValue();
				//v->ParameterInfo.Index = INDEX_NONE;
				//v->ParameterInfo.Name = *(TEXT("mtoon") + t.key);
				//v->ParameterInfo.Association = EMaterialParameterAssociation::GlobalParameter;
				//v->ParameterValue = FLinearColor(t.value[0], t.value[1], t.value[2], t.value[3]);
			}
		}
		{
			struct TT {
				FString key;
				float& value;
			};
			TT table[] = {
				{TEXT("_Cutoff"),		vrmMat.floatProperties._Cutoff},
				{TEXT("_BumpScale"),	vrmMat.floatProperties._BumpScale},
				{TEXT("_ReceiveShadowRate"),	vrmMat.floatProperties._ReceiveShadowRate},
				{TEXT("_ShadeShift"),			vrmMat.floatProperties._ShadeShift},
				{TEXT("_ShadeToony"),			vrmMat.floatProperties._ShadeToony},
				{TEXT("_LightColorAttenuation"),	vrmMat.floatProperties._LightColorAttenuation},
				{TEXT("_IndirectLightIntensity"),	vrmMat.floatProperties._IndirectLightIntensity},
				{TEXT("_RimLightingMix"),			vrmMat.floatProperties._RimLightingMix},
				{TEXT("_RimFresnelPower"),			vrmMat.floatProperties._RimFresnelPower},
				{TEXT("_RimLift"),					vrmMat.floatProperties._RimLift},
				{TEXT("_OutlineWidth"),			vrmMat.floatProperties._OutlineWidth},
				{TEXT("_OutlineScaledMaxDistance"),	vrmMat.floatProperties._OutlineScaledMaxDistance},
				{TEXT("_OutlineLightingMix"),			vrmMat.floatProperties._OutlineLightingMix},
				{TEXT("_UvAnimScrollX"),			vrmMat.floatProperties._UvAnimScrollX},
				{TEXT("_UvAnimScrollY"),			vrmMat.floatProperties._UvAnimScrollY},
				{TEXT("_UvAnimRotation"),			vrmMat.floatProperties._UvAnimRotation},
				{TEXT("_MToonVersion"),				vrmMat.floatProperties._MToonVersion},
				{TEXT("_DebugMode"),				vrmMat.floatProperties._DebugMode},
				{TEXT("_BlendMode"),				vrmMat.floatProperties._BlendMode},
				{TEXT("_OutlineWidthMode"),		vrmMat.floatProperties._OutlineWidthMode},
				{TEXT("_OutlineColorMode"),	vrmMat.floatProperties._OutlineColorMode},
				{TEXT("_CullMode"),			vrmMat.floatProperties._CullMode},
				{TEXT("_OutlineCullMode"),		vrmMat.floatProperties._OutlineCullMode},
				{TEXT("_SrcBlend"),			vrmMat.floatProperties._SrcBlend},
				{TEXT("_DstBlend"),			vrmMat.floatProperties._DstBlend},
				{TEXT("_ZWrite"),			vrmMat.floatProperties._ZWrite},
			};

			for (auto &t : table) {
				LocalScalarParameterSet(dm, *(TEXT("mtoon") + t.key), t.value);

				//FScalarParameterValue *v = new (dm->ScalarParameterValues) FScalarParameterValue();
				//v->ParameterInfo.Index = INDEX_NONE;
				//v->ParameterInfo.Name = *(TEXT("mtoon") + t.key);
				//v->ParameterInfo.Association = EMaterialParameterAssociation::GlobalParameter;
				//v->ParameterValue = t.value;
			}

			//if (vrmMat.floatProperties._CullMode == 0.f) {
			//	dm->BasePropertyOverrides.bOverride_TwoSided = true;
			//	dm->BasePropertyOverrides.TwoSided = 1;
			//}
			if (vrmMat.floatProperties._Cutoff != 0.f) {
				dm->BasePropertyOverrides.bOverride_OpacityMaskClipValue = true;
				dm->BasePropertyOverrides.OpacityMaskClipValue = vrmMat.floatProperties._Cutoff;
			}

			{
				switch ((int)(vrmMat.floatProperties._BlendMode)) {
				case 0:
				case 1:
					break;
				case 2:
				case 3:
					//dm->BasePropertyOverrides.bOverride_BlendMode = true;
					//dm->BasePropertyOverrides.BlendMode = EBlendMode::BLEND_Translucent;
					break;
				}
			}
		}
		{
			struct TT {
				FString key;
				int value;
			};
			TT table[] = {
				{TEXT("mtoon_tex_MainTex"),		vrmMat.textureProperties._MainTex},
				{TEXT("mtoon_tex_ShadeTexture"),	vrmMat.textureProperties._ShadeTexture},
				{TEXT("mtoon_tex_BumpMap"),		vrmMat.textureProperties._BumpMap},
				{TEXT("_ReceiveShadowTexture"),	vrmMat.textureProperties._ReceiveShadowTexture},
				{TEXT("_ShadingGradeTexture"),	vrmMat.textureProperties._ShadingGradeTexture},
				{TEXT("_RimTexture"),			vrmMat.textureProperties._RimTexture},
				{TEXT("mtoon_tex_SphereAdd"),	vrmMat.textureProperties._SphereAdd},
				{TEXT("mtoon_tex_EmissionMap"),	vrmMat.textureProperties._EmissionMap},
				{TEXT("mtoon_tex_OutlineWidthTexture"),	vrmMat.textureProperties._OutlineWidthTexture},
				{TEXT("_UvAnimMaskTexture"),	vrmMat.textureProperties._UvAnimMaskTexture},
			};
			for (auto &t : table) {
				if (t.value < 0) {
					continue;
				}
				LocalTextureSet(dm, *t.key, vrmAssetList->Textures[t.value]);

				//FTextureParameterValue *v = new (dm->TextureParameterValues) FTextureParameterValue();
				//v->ParameterInfo.Index = INDEX_NONE;
				//v->ParameterInfo.Name = *t.key;
				//v->ParameterInfo.Association = EMaterialParameterAssociation::GlobalParameter;
				//v->ParameterValue = vrmAssetList->Textures[t.value];
			}

		}

		return true;
	}

	UMaterial* CreateDefaultMaterial(UVrmAssetListObject *vrmAssetList) {
		//auto MaterialFactory = NewObject<UMaterialFactoryNew>();

#if	UE_VERSION_OLDER_THAN(4,20,0)
		UMaterial* UnrealMaterial = NewObject<UMaterial>(vrmAssetList->Package, TEXT("M_BaseMaterial"), RF_Standalone | RF_Public);
#else
		UMaterial* UnrealMaterial = nullptr;
		if (vrmAssetList->Package == GetTransientPackage()) {
			UnrealMaterial = NewObject<UMaterial>(GetTransientPackage(), NAME_None, EObjectFlags::RF_Public | RF_Transient, nullptr);
		}
		else {
			UnrealMaterial = NewObject<UMaterial>(vrmAssetList->Package, TEXT("M_BaseMaterial"), RF_Standalone | RF_Public, nullptr);
		}
#endif

		if (UnrealMaterial != NULL)
		{
			//UnrealMaterialFinal = UnrealMaterial;
			// Notify the asset registry
			//FAssetRegistryModule::AssetCreated(UnrealMaterial);

			if(true)
			{
				bool bNeedsRecompile = true;
				UnrealMaterial->GetMaterial()->SetMaterialUsage(bNeedsRecompile, MATUSAGE_SkeletalMesh);
				UnrealMaterial->GetMaterial()->SetMaterialUsage(bNeedsRecompile, MATUSAGE_MorphTargets);
			}

			// Set the dirty flag so this package will get saved later
			//Package->SetDirtyFlag(true);

			// textures and properties

			{
				UMaterialExpressionTextureSampleParameter2D* UnrealTextureExpression = NewObject<UMaterialExpressionTextureSampleParameter2D>(UnrealMaterial);
				UnrealMaterial->Expressions.Add(UnrealTextureExpression);
				UnrealTextureExpression->SamplerType = SAMPLERTYPE_Color;
				UnrealTextureExpression->ParameterName = TEXT("gltf_tex_diffuse");

#if WITH_EDITORONLY_DATA
				UnrealMaterial->BaseColor.Expression = UnrealTextureExpression;
#endif
			}

			UnrealMaterial->BlendMode = BLEND_Masked;
			UnrealMaterial->SetShadingModel(MSM_DefaultLit);
			/*
				UnrealMaterial->BlendMode = BLEND_Translucent;
				CreateAndLinkExpressionForMaterialProperty(FbxMaterial, UnrealMaterial, FbxSurfaceMaterial::sTransparencyFactor, UnrealMaterial->OpacityMask, false, UVSets, FVector2D(150, 256));

			}
			FixupMaterial(FbxMaterial, UnrealMaterial); // add random diffuse if none exists
			*/
		}
		return UnrealMaterial;
	}


	bool isSameMaterial(const UMaterialInterface *mi1, const UMaterialInterface *mi2) {
		const UMaterialInstanceConstant *m1 = Cast<UMaterialInstanceConstant>(mi1);
		const UMaterialInstanceConstant *m2 = Cast<UMaterialInstanceConstant>(mi2);

		if (m1 == nullptr || m2 == nullptr) {
			return false;
		}
		// tex
		{
			if (m1->TextureParameterValues.Num() != m2->TextureParameterValues.Num()) {
				return false;
			}
			for (int i = 0; i < m1->TextureParameterValues.Num(); ++i) {
				if (m1->TextureParameterValues[i].ParameterValue != m2->TextureParameterValues[i].ParameterValue) {
					return false;
				}
				if (m1->TextureParameterValues[i].ParameterInfo.Name != m2->TextureParameterValues[i].ParameterInfo.Name) {
					return false;
				}
			}
		}

		// scalar
		{
			if (m1->ScalarParameterValues.Num() != m2->ScalarParameterValues.Num()) {
				return false;
			}
			for (int i = 0; i < m1->ScalarParameterValues.Num(); ++i) {
				if (m1->ScalarParameterValues[i].ParameterValue != m2->ScalarParameterValues[i].ParameterValue) {
					return false;
				}
				if (m1->ScalarParameterValues[i].ParameterInfo.Name != m2->ScalarParameterValues[i].ParameterInfo.Name) {
					return false;
				}
			}
		}

		// vector
		{
			if (m1->VectorParameterValues.Num() != m2->VectorParameterValues.Num()) {
				return false;
			}
			for (int i = 0; i < m1->VectorParameterValues.Num(); ++i) {
				if (m1->VectorParameterValues[i].ParameterValue != m2->VectorParameterValues[i].ParameterValue) {
					return false;
				}
				if (m1->VectorParameterValues[i].ParameterInfo.Name != m2->VectorParameterValues[i].ParameterInfo.Name) {
					return false;
				}
			}
		}

		return true;
	}
}// namespace

UTexture2D* VRMConverter::CreateTexture(int32 InSizeX, int32 InSizeY, FString name, UPackage *package) {
	auto format = PF_B8G8R8A8;
	UTexture2D* NewTexture = NULL;
	if (InSizeX > 0 && InSizeY > 0 &&
		(InSizeX % GPixelFormats[format].BlockSizeX) == 0 &&
		(InSizeY % GPixelFormats[format].BlockSizeY) == 0)
	{
		if (package == GetTransientPackage()) {
			NewTexture = NewObject<UTexture2D>(GetTransientPackage(), NAME_None, EObjectFlags::RF_Public | RF_Transient);
		} else {
			NewTexture = NewObject<UTexture2D>(
				// GetTransientPackage(),
				package,
				*name,
				//RF_Transient
				RF_Public | RF_Standalone
				);
		}
		NewTexture->Modify();
#if WITH_EDITOR
		NewTexture->PreEditChange(NULL);
#endif

		NewTexture->PlatformData = new FTexturePlatformData();
		NewTexture->PlatformData->SizeX = InSizeX;
		NewTexture->PlatformData->SizeY = InSizeY;
		NewTexture->PlatformData->PixelFormat = format;

		int32 NumBlocksX = InSizeX / GPixelFormats[format].BlockSizeX;
		int32 NumBlocksY = InSizeY / GPixelFormats[format].BlockSizeY;
		FTexture2DMipMap* Mip = new(NewTexture->PlatformData->Mips) FTexture2DMipMap();
		Mip->SizeX = InSizeX;
		Mip->SizeY = InSizeY;
		Mip->BulkData.Lock(LOCK_READ_WRITE);
		Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[format].BlockBytes);
		Mip->BulkData.Unlock();
	} else
	{
		UE_LOG(LogTexture, Warning, TEXT("Invalid parameters specified for UTexture2D::Create()"));
	}
	return NewTexture;
}


bool VRMConverter::ConvertTextureAndMaterial(UVrmAssetListObject *vrmAssetList, const aiScene *mScenePtr) {
	if (vrmAssetList == nullptr || mScenePtr == nullptr) {
		return false;
	}

	vrmAssetList->Textures.Reset(0);
	vrmAssetList->Materials.Reset(0);
	vrmAssetList->OutlineMaterials.Reset(0);

	TArray<bool> NormalBoolTable;
	NormalBoolTable.SetNum(mScenePtr->mNumTextures);
	{
		const VRM::VRMMetadata *meta = static_cast<const VRM::VRMMetadata*>(mScenePtr->mVRMMeta);
		if (meta) {
			for (int i = 0; i < meta->materialNum; ++i) {
				int t = meta->material[i].textureProperties._BumpMap;
				if (t < 0) continue;

				if (t < NormalBoolTable.Num()) {
					NormalBoolTable[t] = true;
				}
			}
		}
	}


	TArray<UTexture2D*> texArray;
	texArray.Reserve(mScenePtr->mNumTextures);
	if (mScenePtr->HasTextures()) {
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		TSharedPtr<IImageWrapper> ImageWrapper;
		// Note: PNG format.  Other formats are supported
		ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

		for (uint32_t i = 0; i < mScenePtr->mNumTextures; ++i) {
			auto &t = *mScenePtr->mTextures[i];
			int Width = t.mWidth;
			int Height = t.mHeight;
			const TArray<uint8>* RawData = nullptr;

			if (Height == 0) {
				if (ImageWrapper->SetCompressed(t.pcData, t.mWidth)) {

				}
				Width = ImageWrapper->GetWidth();
				Height = ImageWrapper->GetHeight();

				if (Width == 0 || Height == 0) {
					continue;
				}

				ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData);
			}
			FString baseName = NormalizeFileName(t.mFilename.C_Str());
			if (baseName.Len() == 0) {
				baseName = FString::FromInt(i);
			}
			if (NormalBoolTable[i]) {
				baseName += TEXT("_N");

			}

			UTexture2D* NewTexture2D = CreateTexture(Width, Height, FString(TEXT("T_")) + baseName, vrmAssetList->Package);
			//UTexture2D* NewTexture2D = _CreateTransient(Width, Height, PF_B8G8R8A8, t.mFilename.C_Str());

			// Fill in the base mip for the texture we created
			uint8* MipData = (uint8*)NewTexture2D->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			if (RawData) {
				FMemory::Memcpy(MipData, RawData->GetData(), RawData->Num());
			}
			else {
				for (int32 y = 0; y < Height; y++)
				{
					const aiTexel *c = &(t.pcData[y*Width]);
					uint8* DestPtr = &MipData[y * Width * sizeof(FColor)];
					for (int32 x = 0; x < Width; x++)
					{
						*DestPtr++ = c->b;
						*DestPtr++ = c->g;
						*DestPtr++ = c->r;
						*DestPtr++ = c->a;
						c++;
					}
				}
			}
			NewTexture2D->PlatformData->Mips[0].BulkData.Unlock();

			// Set options
			NewTexture2D->SRGB = true;// bUseSRGB;
			NewTexture2D->CompressionSettings = TC_Default;
			if (NormalBoolTable[i]) {
				NewTexture2D->CompressionSettings = TC_Normalmap;
				NewTexture2D->SRGB = 0;
			}
			NewTexture2D->AddressX = TA_Wrap;
			NewTexture2D->AddressY = TA_Wrap;

#if WITH_EDITORONLY_DATA
			NewTexture2D->CompressionNone = false;
			NewTexture2D->DeferCompression = true;
			NewTexture2D->MipGenSettings = TMGS_NoMipmaps;
			NewTexture2D->Source.Init(Width, Height, 1, 1, ETextureSourceFormat::TSF_BGRA8, RawData->GetData());
			//NewTexture2D->Source.Compress();
#endif

			// Update the remote texture data
			NewTexture2D->UpdateResource();
#if WITH_EDITOR
			NewTexture2D->PostEditChange();
#endif

			texArray.Push(NewTexture2D);
		}
		vrmAssetList->Textures = texArray;
	}

	const bool bOptimizeMaterial = Options::Get().IsOptimizeMaterial();

	TArray<UMaterialInterface*> matArray;
	if (mScenePtr->HasMaterials()) {

		//TArray<FString> MatNameList;
		TMap<FString, int> MatNameList;


		vrmAssetList->Materials.SetNum(mScenePtr->mNumMaterials);
		for (uint32_t iMat = 0; iMat < mScenePtr->mNumMaterials; ++iMat) {
			auto &aiMat = *mScenePtr->mMaterials[iMat];

			UMaterialInterface *baseM = nullptr;;
			bool bMToon = false;
			bool bLit = false;
			{
				aiString alphaMode;
				aiReturn result00 = aiMat.Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
				FString ShaderName = aiMat.mShaderName.C_Str();

				if (ShaderName.Find(TEXT("MToon")) >= 0) {
					bMToon = true;
				}

				// select
				switch (Options::Get().GetMaterialType()) {
				case EVRMImportMaterialType::VRMIMT_MToon:
					baseM = vrmAssetList->BaseMToonLitOpaqueMaterial;
					bMToon = true;
					bLit = true;
					break;
				case EVRMImportMaterialType::VRMIMT_MToonUnlit:
					baseM = vrmAssetList->BaseMToonUnlitOpaqueMaterial;
					bMToon = true;
					bLit = false;
					break;
				case EVRMImportMaterialType::VRMIMT_Unlit:
					baseM = vrmAssetList->BaseUnlitOpaqueMaterial;
					break;
				case EVRMImportMaterialType::VRMIMT_glTF:
					baseM = vrmAssetList->BasePBROpaqueMaterial;
					break;
				}

				// auto
				if (Options::Get().GetMaterialType() == EVRMImportMaterialType::VRMIMT_Auto) {
					if (bMToon) {
						baseM = vrmAssetList->BaseMToonUnlitOpaqueMaterial;
					} else {
						baseM = vrmAssetList->BaseUnlitOpaqueMaterial;
					}
				}

				// mtoon optimize
				if (bMToon && bOptimizeMaterial) {
					bool bTwoSided = false;
					bool bTranslucent = false;
					{
						aiString alphaMode;
						aiReturn result = aiMat.Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
						FString alpha = alphaMode.C_Str();
						if (alpha == TEXT("BLEND")) {
							// check also _ZWrite
							bTranslucent = true;
						}

						const VRM::VRMMetadata *meta = static_cast<const VRM::VRMMetadata*>(mScenePtr->mVRMMeta);
						if ((int)iMat < meta->materialNum) {
							auto &vrmMat = meta->material[iMat];
							if (vrmMat.floatProperties._CullMode == 0.f) {
								bTwoSided = true;
							}
							if (vrmMat.floatProperties._ZWrite == 1.f) {
								bTranslucent = false;
							}
						}
					}

					// lit, opaque, twoside
					UMaterialInterface *table[2][2][2] = {
						{
							vrmAssetList->OptMToonLitOpaqueMaterial,
							vrmAssetList->OptMToonLitOpaqueTwoSidedMaterial,
							vrmAssetList->OptMToonLitTranslucentMaterial,
							vrmAssetList->OptMToonLitTranslucentTwoSidedMaterial,
						},
						{
							vrmAssetList->OptMToonUnlitOpaqueMaterial,
							vrmAssetList->OptMToonUnlitOpaqueTwoSidedMaterial,
							vrmAssetList->OptMToonUnlitTranslucentMaterial,
							vrmAssetList->OptMToonUnlitTranslucentTwoSidedMaterial,
						},
					};
					int c[3] = {
						bLit ? 0 : 1,
						bTranslucent ? 1 : 0,
						bTwoSided ? 1 : 0,
					};

					baseM = table[c[0]][c[1]][c[2]];

					if (bLit) {
						//baseM = vrmAssetList->OptMToonLitOpaqueMaterial;
					}else{
						//baseM = vrmAssetList->OptMToonUnlitOpaqueMaterial;
					}
				}

				//unlit or gltf
				if (bMToon == false) {
					if (ShaderName.Find(TEXT("UnlitTexture")) >= 0) {
						baseM = vrmAssetList->BaseUnlitOpaqueMaterial;
					}
					if (ShaderName.Find(TEXT("UnlitTransparent")) >= 0) {
						baseM = vrmAssetList->BaseUnlitTranslucentMaterial;
					}
				}

				if (baseM == nullptr) {
					baseM = vrmAssetList->BaseMToonUnlitOpaqueMaterial;
				}
			}
			//if (FString(m.mShaderName.C_Str()).Find(TEXT("UnlitTexture"))) {

			if (baseM == nullptr) {
				baseM = CreateDefaultMaterial(vrmAssetList);
				vrmAssetList->BaseUnlitOpaqueMaterial = baseM;
			}
			if (baseM == nullptr) {
				continue;
			}

			aiString texName;
			int index = -1;
			{
				for (uint32_t t = 0; t < AI_TEXTURE_TYPE_MAX; ++t) {
					uint32_t n = aiMat.GetTextureCount((aiTextureType)t);
					for (uint32_t y = 0; y < n; ++y) {
						aiMat.GetTexture((aiTextureType)t, y, &texName);
						//UE_LOG(LogTemp, Warning, TEXT("R--%s\n"), texName.C_Str());
					}
				}

				for (uint32_t i = 0; i < mScenePtr->mNumTextures; ++i) {
					if (mScenePtr->mTextures[i]->mFilename == texName) {
						index = i;
						break;
					}
				}
			}
			{
				aiString path;
				aiReturn r = aiMat.GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &path);
				if (r == AI_SUCCESS) {
					std::string s = path.C_Str();
					s = s.substr(s.find_last_of('*') + 1);
					index = atoi(s.c_str());
				}
			}

			//UMaterialInstanceDynamic* dm = UMaterialInstanceDynamic::Create(baseM, vrmAssetList, m.GetName().C_Str());
			//UMaterialInstanceDynamic* dm = UMaterialInstance::Create(baseM, vrmAssetList, m.GetName().C_Str());
			//MaterialInstance->TextureParameterValues

			//set paramater with Set***ParamaterValue
			//DynMaterial->SetScalarParameterValue("MyParameter", myFloatValue);
			//MyComponent1->SetMaterial(0, DynMaterial);
			//MyComponent2->SetMaterial(0, DynMaterial);

			if (index >= 0) {
			//if (index >= 0 && index < vrmAssetList->Textures.Num()) {

				UMaterialInstanceConstant* dm = nullptr;
				{
					const FString origname = (FString(TEXT("M_")) + NormalizeFileName(aiMat.GetName().C_Str()));
					FString name = origname;

					if (MatNameList.Find(origname)) {
						name += FString::Printf(TEXT("_%03d"), MatNameList[name]);// TEXT("_2");
					}
					MatNameList.FindOrAdd(origname)++;

					if (vrmAssetList->Package == GetTransientPackage()) {
						dm = NewObject<UMaterialInstanceConstant>(GetTransientPackage(), NAME_None, EObjectFlags::RF_Public | RF_Transient);
					} else {
						TArray<UObject*> ret;
						GetObjectsWithOuter(vrmAssetList->Package, ret);
						for (auto *a : ret) {
							auto s = a->GetName().ToLower();
							if(s.IsEmpty()) continue;

							if (s == name.ToLower()) {
								a->ClearFlags(EObjectFlags::RF_Standalone);
								a->SetFlags(EObjectFlags::RF_Public | RF_Transient);
								a->ConditionalBeginDestroy();
								break;
							}
						}
						dm = NewObject<UMaterialInstanceConstant>(vrmAssetList->Package, *name, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
					}
				}
				LocalMaterialSetParent(dm, baseM);

				if (dm) {
					{
						aiColor4D col;
						aiReturn result = aiMat.Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, col);
						if (result == 0) {
							FVectorParameterValue *v = new (dm->VectorParameterValues) FVectorParameterValue();
							v->ParameterInfo.Index = INDEX_NONE;
							v->ParameterInfo.Name = TEXT("gltf_basecolor");
							v->ParameterInfo.Association = EMaterialParameterAssociation::GlobalParameter;
							v->ParameterValue = FLinearColor(col.r, col.g, col.b, col.a);
						}
					}

					{
						float f[2] = { 1,1 };
						aiReturn result0 = aiMat.Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, f[0]);
						aiReturn result1 = aiMat.Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, f[1]);
						if (result0 == AI_SUCCESS || result1 == AI_SUCCESS) {
							f[0] = (result0 == AI_SUCCESS) ? f[0] : 1;
							f[1] = (result1 == AI_SUCCESS) ? f[1] : 1;
							if (f[0] == 0 && f[1] == 0) {
								f[0] = f[1] = 1.f;
							}
							FVectorParameterValue *v = new (dm->VectorParameterValues) FVectorParameterValue();
							v->ParameterInfo.Index = INDEX_NONE;
							v->ParameterInfo.Name = TEXT("gltf_RM");
							v->ParameterInfo.Association = EMaterialParameterAssociation::GlobalParameter;;
							v->ParameterValue = FLinearColor(f[0], f[1], 0, 0);
						}
					}
					if (index < vrmAssetList->Textures.Num()) {
						LocalTextureSet(dm, TEXT("gltf_tex_diffuse"), vrmAssetList->Textures[index]);
					}
					if (bMToon == false){
						aiString alphaMode;
						aiReturn result = aiMat.Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
						FString alpha = alphaMode.C_Str();
						if (alpha == TEXT("BLEND")) {
							dm->BasePropertyOverrides.bOverride_BlendMode = true;;
							dm->BasePropertyOverrides.BlendMode = EBlendMode::BLEND_Translucent;
						}
					}


					// mtoon
					if (bMToon) {
						createAndAddMaterial(dm, iMat, vrmAssetList, mScenePtr);
					}

					LocalMaterialFinishParam(dm);

					//dm->InitStaticPermutation();
					matArray.Add(dm);
				}
			}
		}


		if (Options::Get().IsMergeMaterial() == false) {
			vrmAssetList->Materials = matArray;
		} else {
			TArray<UMaterialInterface*> tmp;

			vrmAssetList->MaterialMergeTable.Reset();

			for (int i = 0; i < matArray.Num(); ++i) {

				vrmAssetList->MaterialMergeTable.Add(i, 0);

				bool bFind = false;
				for (int j = 0; j < tmp.Num(); ++j) {
					if (isSameMaterial(matArray[i], tmp[j]) == false) {
						continue;
					}
					bFind = true;
					vrmAssetList->MaterialMergeTable[i] = j;

					matArray[i]->Rename(nullptr, GetTransientPackage(), EObjectFlags::RF_Public | RF_Transient);

					break;
				}
				if (bFind == false) {
					int t = tmp.Add(matArray[i]);
					vrmAssetList->MaterialMergeTable[i] = t;
				}
			}

			vrmAssetList->Materials = tmp;
		}

		// ouline Material
		{
			if (bOptimizeMaterial && vrmAssetList->OptMToonOutlineMaterial){
				for (const auto aa : vrmAssetList->Materials) {
					const UMaterialInstanceConstant *a = Cast<UMaterialInstanceConstant>(aa);

					FString s = a->GetName() + TEXT("_outline");
					//UMaterialInstanceConstant *m = Cast<UMaterialInstanceConstant>(StaticDuplicateObject(a->GetOuter(), a, *s, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, UMaterialInstanceConstant::StaticClass()));
					UMaterialInstanceConstant *m = NewObject<UMaterialInstanceConstant>(vrmAssetList->Package, *s, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);

					if (m) {
						LocalMaterialSetParent(m, vrmAssetList->OptMToonOutlineMaterial);

						m->VectorParameterValues = a->VectorParameterValues;
						m->ScalarParameterValues = a->ScalarParameterValues;
						m->TextureParameterValues = a->TextureParameterValues;

						LocalMaterialFinishParam(m);
						//m->InitStaticPermutation();
						vrmAssetList->OutlineMaterials.Add(m);
					}
				}
			}
		}
	}

	return true;
}

VrmConvertTexture::VrmConvertTexture()
{
}

VrmConvertTexture::~VrmConvertTexture()
{
}
