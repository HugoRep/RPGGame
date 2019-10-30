// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#include "LoaderBPFunctionLibrary.h"
//#include "VRM4U.h"
#include "VrmSkeleton.h"
#include "VrmSkeletalMesh.h"
#include "VrmModelActor.h"
#include "VrmAssetListObject.h"
#include "VrmMetaObject.h"
#include "VrmLicenseObject.h"

#include "VrmConvert.h"

#include "Components/SkeletalMeshComponent.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Animation/Skeleton.h"
#include "Animation/MorphTarget.h"
#include "Animation/NodeMappingContainer.h"

//#include ".h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>
#include <assimp/vrm/vrmmeta.h>

#include "RenderingThread.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintTemplate.h"
#include "Misc/FeedbackContext.h"
#include "Misc/FileHelper.h"
#include "UObject/Package.h"

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

#include "AssetRegistryModule.h"
#include "UObject/Package.h"
#include "Engine/Engine.h"

//#include "Windows/WindowsSystemIncludes.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

// tem
namespace {
	UPackage *package = nullptr;
	FString baseFileName;
	FReturnedData result;
}


static bool saveObject(UObject *u, bool bSave) {
#if WITH_EDITOR
	if (u == nullptr) return false;
	//package->MarkPackageDirty();
	//FAssetRegistryModule::AssetCreated(u);
	//bool bSaved = UPackage::SavePackage(package, u, EObjectFlags::RF_Standalone, *(package->GetName()), GError, nullptr, true, true, SAVE_NoError);

	if (VRMConverter::IsImportMode()) {
		u->PostEditChange();
	}
#endif
	return true;
}

static void UpdateProgress(int prog) {
#if WITH_EDITOR
	GWarn->UpdateProgress( prog, 100 );
#endif
}

////

static void ReTransformHumanoidBone(USkeleton *targetHumanoidSkeleton, const UVrmMetaObject *meta, const USkeleton *displaySkeleton) {

	FReferenceSkeleton &ReferenceSkeleton = const_cast<FReferenceSkeleton&>(targetHumanoidSkeleton->GetReferenceSkeleton());
	auto &allbone = const_cast<TArray<FMeshBoneInfo> &>(targetHumanoidSkeleton->GetReferenceSkeleton().GetRawRefBoneInfo());

	//auto &humanoidTrans = humanoidSkeleton->GetReferenceSkeleton().GetRawRefBonePose();

	FReferenceSkeletonModifier RefSkelModifier(ReferenceSkeleton, targetHumanoidSkeleton);

	for (int ind_target = 0; ind_target < targetHumanoidSkeleton->GetReferenceSkeleton().GetRawBoneNum(); ++ind_target) {
		FTransform t;
		t.SetIdentity();
		RefSkelModifier.UpdateRefPoseTransform(ind_target, t);

		auto boneName = ReferenceSkeleton.GetBoneName(ind_target);
		//auto &info = targetHumanoidSkeleton->GetReferenceSkeleton().GetRefBoneInfo();
		//auto &a = info[ind_target];

		int32 ind_disp = 0;

		if (meta) {
			auto p = meta->humanoidBoneTable.Find(boneName.ToString());
			if (p == nullptr) {
				continue;
			}
			ind_disp = displaySkeleton->GetReferenceSkeleton().FindBoneIndex(**p);
		}else {
			ind_disp = displaySkeleton->GetReferenceSkeleton().FindBoneIndex(boneName);
		}

		if (ind_disp == INDEX_NONE) {
			continue;
		}
		t = displaySkeleton->GetReferenceSkeleton().GetRefBonePose()[ind_disp];

		auto parent = displaySkeleton->GetReferenceSkeleton().GetParentIndex(ind_disp);
		while (parent != INDEX_NONE) {

			auto s = displaySkeleton->GetReferenceSkeleton().GetBoneName(parent);

			if (meta) {
				if (meta->humanoidBoneTable.FindKey(s.ToString()) != nullptr) {
					// parent == humanoidBone
					break;
				}
			}
			//t.SetLocation(t.GetLocation() + displaySkeleton->GetReferenceSkeleton().GetRefBonePose()[parent].GetLocation());;
			parent = displaySkeleton->GetReferenceSkeleton().GetParentIndex(parent);
		}
		RefSkelModifier.UpdateRefPoseTransform(ind_target, t);
	}

	ReferenceSkeleton.RebuildRefSkeleton(targetHumanoidSkeleton, true);

}

bool ULoaderBPFunctionLibrary::VRMReTransformHumanoidBone(USkeletalMeshComponent *targetHumanoidSkeleton, const UVrmMetaObject *meta, const USkeletalMeshComponent *displaySkeleton) {

	if (targetHumanoidSkeleton == nullptr) return false;
	if (targetHumanoidSkeleton->SkeletalMesh == nullptr) return false;

	if (displaySkeleton == nullptr) return false;
	if (displaySkeleton->SkeletalMesh == nullptr) return false;

	// no meta. use default name.
	//if (meta == nullptr) return false;

	ReTransformHumanoidBone(targetHumanoidSkeleton->SkeletalMesh->Skeleton, meta, displaySkeleton->SkeletalMesh->Skeleton);
	auto &sk = targetHumanoidSkeleton->SkeletalMesh;
	auto &k = sk->Skeleton;

	sk->RefSkeleton = k->GetReferenceSkeleton();
	//sk->RefSkeleton.RebuildNameToIndexMap();

	//sk->RefSkeleton.RebuildRefSkeleton(sk->Skeleton, true);
	//sk->Proc();

	//out->RefSkeleton = sk->RefSkeleton;
	
	sk->Skeleton = k;
	sk->RefSkeleton = k->GetReferenceSkeleton();

	sk->CalculateInvRefMatrices();
	sk->CalculateExtendedBounds();

#if WITH_EDITORONLY_DATA
	sk->ConvertLegacyLODScreenSize();
#if	UE_VERSION_OLDER_THAN(4,20,0)
#else
	sk->UpdateGenerateUpToData();
#endif
#endif

#if WITH_EDITORONLY_DATA
	k->SetPreviewMesh(sk);
#endif
	k->RecreateBoneTree(sk);

	return true;
}



void ULoaderBPFunctionLibrary::SetImportMode(bool bIm, class UPackage *p) {
	VRMConverter::SetImportMode(bIm);
	package = p;
}

namespace {
#if PLATFORM_WINDOWS
	std::string utf_16_to_shift_jis(const std::wstring& str) {
		static_assert(sizeof(wchar_t) == 2, "this function is windows only");
		const int len = ::WideCharToMultiByte(932/*CP_ACP*/, 0, str.c_str(), -1, nullptr, 0, nullptr, nullptr);
		std::string re(len * 2, '\0');
		if (!::WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, &re[0], len, nullptr, nullptr)) {
			const auto ec = ::GetLastError();
			switch (ec)
			{
			case ERROR_INSUFFICIENT_BUFFER:
				//throw std::runtime_error("in function utf_16_to_shift_jis, WideCharToMultiByte fail. cause: ERROR_INSUFFICIENT_BUFFER"); break;
			case ERROR_INVALID_FLAGS:
				//throw std::runtime_error("in function utf_16_to_shift_jis, WideCharToMultiByte fail. cause: ERROR_INVALID_FLAGS"); break;
			case ERROR_INVALID_PARAMETER:
				//throw std::runtime_error("in function utf_16_to_shift_jis, WideCharToMultiByte fail. cause: ERROR_INVALID_PARAMETER"); break;
			default:
				//throw std::runtime_error("in function utf_16_to_shift_jis, WideCharToMultiByte fail. cause: unknown(" + std::to_string(ec) + ')'); break;
				break;
			}
		}
		const std::size_t real_len = std::strlen(re.c_str());
		re.resize(real_len);
		re.shrink_to_fit();
		return re;
	}
#endif
}

UVrmLicenseObject* ULoaderBPFunctionLibrary::GetVRMMeta(FString filepath) {
	std::string file;
#if PLATFORM_WINDOWS
	file = utf_16_to_shift_jis(*filepath);
#else
	file = TCHAR_TO_UTF8(*filepath);
#endif
	const aiScene *mScenePtr = nullptr;
	Assimp::Importer mImporter;

	{
		TArray<uint8> Res;
		if (FFileHelper::LoadFileToArray(Res, *filepath)) {
		}
		const FString ext = FPaths::GetExtension(filepath);
		std::string e = utf_16_to_shift_jis(*ext);

		mScenePtr = mImporter.ReadFileFromMemory(Res.GetData(), Res.Num(),
			aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals | aiProcess_OptimizeMeshes,
			e.c_str());

		//UE_LOG(LogTemp, Log, TEXT("VRM:(%3.3lf secs) ReadFileFromMemory"), FPlatformTime::Seconds() - StartTime);
	}
	if (mScenePtr == nullptr) {
		return nullptr;
	}
	auto *p = VRMConverter::GetVRMMeta(mScenePtr);

	UTexture2D* NewTexture2D = nullptr;

	VRM::VRMMetadata *meta = reinterpret_cast<VRM::VRMMetadata*>(mScenePtr->mVRMMeta);

	//mScenePtr->mTextures[0]->mFilename

	for (int i = 0; i < meta->license.licensePairNum; ++i) {

		auto &p = meta->license.licensePair[i];

		if (FString(TEXT("texture")) == p.Key.C_Str()) {
			unsigned int texIndex = FCString::Atoi(*FString(p.Value.C_Str()));
			if (texIndex >= 0 && texIndex < mScenePtr->mNumTextures) {

				IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
				TSharedPtr<IImageWrapper> ImageWrapper;
				// Note: PNG format.  Other formats are supported
				ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

				auto &t = *mScenePtr->mTextures[texIndex];
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
				FString baseName;

				NewTexture2D = VRMConverter::CreateTexture(Width, Height, FString(TEXT("T_")) + baseName, GetTransientPackage());
				//UTexture2D* NewTexture2D = _CreateTransient(Width, Height, PF_B8G8R8A8, t.mFilename.C_Str());

				// Fill in the base mip for the texture we created
				uint8* MipData = (uint8*)NewTexture2D->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
				if (RawData) {
					FMemory::Memcpy(MipData, RawData->GetData(), RawData->Num());
				} else {
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
			}
		}

	}

	p->thumbnail = NewTexture2D;
	return p;
}

bool ULoaderBPFunctionLibrary::VRMSetLoadMaterialType(EVRMImportMaterialType type) {
	VRMConverter::Options::Get().SetMaterialType(type);
	return true;
}

bool ULoaderBPFunctionLibrary::LoadVRMFile(const UVrmAssetListObject *InVrmAsset, UVrmAssetListObject *&OutVrmAsset, FString filepath) {

	if (InVrmAsset == nullptr) {
		return false;
	}

	Assimp::Importer mImporter;
	const aiScene* mScenePtr = nullptr;

	if (filepath.IsEmpty())
	{
	}
	//filepath.esc
	//setlocale(LC_ALL, "");
	//_tsetlocale(LC_ALL, _T(""));
	//_wsetlocale(LC_ALL, _T(""));
	std::string file;
	//TChar a;
	//switch (type)
	//{
	//case EPathType::Absolute:
	//file = TCHAR_TO_UTF8(filepath.GetCharArray().GetData());
	//file = TCHAR_TO_ANSI( *filepath );
	//file = TCHAR_TO_UTF8(*filepath.ReplaceCharWithEscapedChar());
	//file = TCHAR_TO_UTF8(*filepath);
#if PLATFORM_WINDOWS
	file = utf_16_to_shift_jis(*filepath);
#else
	file = TCHAR_TO_UTF8(*filepath);
#endif

	//	break;
	//case EPathType::Relative:
	//	file = TCHAR_TO_UTF8(*FPaths::Combine(FPaths::ProjectContentDir(), filepath));
	//	break;
	//}

	double StartTime = FPlatformTime::Seconds();
	auto LogAndUpdate = [&](FString logname) {
		UE_LOG(LogTemp, Log, TEXT("VRM:(%3.3lf secs) %s"), FPlatformTime::Seconds() - StartTime, *logname);
		StartTime = FPlatformTime::Seconds();
	};

	VRMConverter::Options::Get().SetVRMModel(true);
	{
		TArray<uint8> Res;
		if (FFileHelper::LoadFileToArray(Res, *filepath)) {
		}
		const FString ext = FPaths::GetExtension(filepath);
		std::string e = utf_16_to_shift_jis(*ext);

		if (e.compare("vrm") == 0) {
			VRMConverter::Options::Get().SetVRMModel(true);
		} else {
			VRMConverter::Options::Get().SetVRMModel(false);
		}

		mScenePtr = mImporter.ReadFileFromMemory(Res.GetData(), Res.Num(),
			aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals | aiProcess_OptimizeMeshes,
			e.c_str());
		if (mScenePtr == nullptr) {
			mScenePtr = mImporter.ReadFile(file, aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals | aiProcess_OptimizeMeshes);
		}

		UE_LOG(LogTemp, Log, TEXT("VRM:(%3.3lf secs) ReadFileFromMemory"), FPlatformTime::Seconds() - StartTime);
		StartTime = FPlatformTime::Seconds();
	}
//	mScenePtr = mImporter.ReadFile(file, aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals | aiProcess_OptimizeMeshes);

	UpdateProgress(20);
	if (mScenePtr == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("VRM4U: read failure.\n"));
		return false;
	}

	{
		FString fullpath = FPaths::GameUserDeveloperDir() + TEXT("VRM/");
		FString basepath = FPackageName::FilenameToLongPackageName(fullpath);
		//FPackageName::RegisterMountPoint("/VRMImportData/", fullpath);

		baseFileName = FPaths::GetBaseFilename(filepath);

		//if (VRMConverter::IsImportMode() == false) {
		//	FString name = basepath + baseFileName + TEXT("/") + VRMConverter::NormalizeFileName(FPaths::GetBaseFilename(filepath));
		//	package = CreatePackage(nullptr, *name);
		//}

		if (package == nullptr) {
			package = GetTransientPackage();
		}
	}
	UVrmAssetListObject *out = nullptr;
	if (package == GetTransientPackage()) {
		out = Cast<UVrmAssetListObject>(StaticDuplicateObject(InVrmAsset, package, NAME_None));
	}else {
		out = NewObject<UVrmAssetListObject>(package, *(VRMConverter::NormalizeFileName(baseFileName) + FString(TEXT("_VrmAssetList"))), EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
		//out = Cast<UVrmAssetListObject>(StaticDuplicateObject(InVrmAsset, package, *(VRMConverter::NormalizeFileName(baseFileName) + FString(TEXT("_VrmAssetList"))), EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, UVrmAssetListObject::StaticClass()));
		//out->Modify();
		InVrmAsset->CopyMember(out);
	}
	OutVrmAsset = out;

	if (out == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("VRM4U: no UVrmAssetListObject.\n"));
		return false;
	}

	out->OrigFileName = baseFileName;
	out->BaseFileName = VRMConverter::NormalizeFileName(baseFileName);
	out->Package = package;

	{
		bool ret = true;

		LogAndUpdate(TEXT("Begin convert"));
		ret &= VRMConverter::NormalizeBoneName(mScenePtr);
		LogAndUpdate(TEXT("NormalizeBoneName"));
		ret &= VRMConverter::ConvertTextureAndMaterial(out, mScenePtr);
		LogAndUpdate(TEXT("ConvertTextureAndMaterial"));
		UpdateProgress(40);
		{
			bool r = VRMConverter::ConvertVrmMeta(out, mScenePtr);	// use texture.
			if (VRMConverter::Options::Get().IsVRMModel() == true) {
				ret &= r;
			}
		}
		LogAndUpdate(TEXT("ConvertVrmMeta"));
		UpdateProgress(60);
		ret &= VRMConverter::ConvertModel(out, mScenePtr);
		LogAndUpdate(TEXT("ConvertModel"));
		ret &= VRMConverter::ConvertRig(out, mScenePtr);
		LogAndUpdate(TEXT("ConvertRig"));
#if WITH_EDITOR
		if (out->bSkipMorphTarget == false) {
			ret &= VRMConverter::ConvertMorphTarget(out, mScenePtr);
			LogAndUpdate(TEXT("ConvertMorphTarget"));
		}
		ret &= VRMConverter::ConvertHumanoid(out, mScenePtr);
		LogAndUpdate(TEXT("ConvertHumanoid"));
#endif
		UpdateProgress(80);
		if (ret == false) {
			return false;
		}
	}
	out->VrmMetaObject->SkeletalMesh = out->SkeletalMesh;

	{
		LogAndUpdate(TEXT("BeginSave"));
		bool b = out->bAssetSave;
		saveObject(out, b);
		for (auto &t : out->Textures) {
			saveObject(t, b);
		}
		for (auto &t : out->Materials) {
			saveObject(t, b);
		}
		for (auto &t : out->OutlineMaterials) {
			saveObject(t, b);
		}
		saveObject(out->SkeletalMesh, b);
		saveObject(out->SkeletalMesh->PhysicsAsset, b);
		saveObject(out->VrmMetaObject, b);
		saveObject(out->VrmLicenseObject, b);
		saveObject(out->HumanoidSkeletalMesh, b);
		saveObject(out->HumanoidRig, b);

		LogAndUpdate(TEXT("Save"));
	}

	if (VRMConverter::IsImportMode() == false){
		//FString fullpath = FPaths::GameUserDeveloperDir() + TEXT("VRM/");
		//FString basepath = FPackageName::FilenameToLongPackageName(fullpath);
		//FPackageName::RegisterMountPoint("/VRMImportData/", fullpath);

		//ULevel::LevelDirtiedEvent.Broadcast();
	}

	// force delete vrmdata
	if (mScenePtr->mVRMMeta) {
		auto *a = const_cast<aiScene*>(mScenePtr);
		if (a) {
			VRM::VRMMetadata *meta = reinterpret_cast<VRM::VRMMetadata*>(a->mVRMMeta);
			//VRM::ReleaseVRMMeta(meta);
			//delete meta;
			//a->mVRMMeta = nullptr;
		}

	}
	UpdateProgress(100);
	return true;
}


bool ULoaderBPFunctionLibrary::CopyPhysicsAsset(USkeletalMesh *dstMesh, const USkeletalMesh *srcMesh){
	//GetTransientPackage
#if WITH_EDITOR

	UPhysicsAsset *srcPA = srcMesh->PhysicsAsset;
	UPackage *pk = srcPA->GetOutermost();

	FString name = srcPA->GetFName().ToString() + TEXT("_copy");
	UPhysicsAsset *dstPA = NewObject<UPhysicsAsset>(pk, *name, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
	dstPA->Modify();
	dstPA->SetPreviewMesh(dstMesh);

	for (auto *a : srcPA->SkeletalBodySetups) {
		const auto rigName = srcMesh->Skeleton->GetRigNodeNameFromBoneName(a->BoneName);
		const auto dstBoneName = dstMesh->Skeleton->GetRigBoneMapping(rigName);

		USkeletalBodySetup *bs = Cast<USkeletalBodySetup>(StaticDuplicateObject(a, dstPA, NAME_None));
		bs->BoneName = dstBoneName;

		{
			auto srcIndex = srcMesh->RefSkeleton.FindBoneIndex(a->BoneName);
			auto dstIndex = dstMesh->RefSkeleton.FindBoneIndex(dstBoneName);
			FTransform srcTrans, dstTrans;
			while (1)
			{
				srcIndex = srcMesh->RefSkeleton.GetParentIndex(srcIndex);
				if (srcIndex < 0) {
					break;
				}
				srcTrans = srcMesh->RefSkeleton.GetRefBonePose()[srcIndex].GetRelativeTransform(srcTrans);
			}
			while (1)
			{
				dstIndex = dstMesh->RefSkeleton.GetParentIndex(dstIndex);
				if (dstIndex < 0) {
					break;
				}
				dstTrans = dstMesh->RefSkeleton.GetRefBonePose()[dstIndex].GetRelativeTransform(dstTrans);
			}

			{
				for (int i = 0; i < bs->AggGeom.SphylElems.Num(); ++i) {
					bs->AggGeom.SphylElems[i].Center.Set(0, 0, 0);
					bs->AggGeom.SphylElems[i].Rotation = FRotator::ZeroRotator;
					//bs->AggGeom.SphylElems[i].Center.X = 0;// -v.Z;
					//bs->AggGeom.SphylElems[i].Center.Y = 0;// v.Y;
					//bs->AggGeom.SphylElems[i].Center.Z = 0;// v.X;
				}
				for (auto &b : bs->AggGeom.BoxElems) {
					b.Center.Set(0, 0, 0);
					b.Rotation = FRotator::ZeroRotator;
				}
			}
		}

		dstPA->SkeletalBodySetups.Add(bs);
	}
	for (auto *a : srcPA->ConstraintSetup) {
		const auto rigName1 = srcMesh->Skeleton->GetRigNodeNameFromBoneName(a->DefaultInstance.ConstraintBone1);
		const auto rigName2 = srcMesh->Skeleton->GetRigNodeNameFromBoneName(a->DefaultInstance.ConstraintBone2);
		const auto rigNameJ = srcMesh->Skeleton->GetRigNodeNameFromBoneName(a->DefaultInstance.JointName);

		UPhysicsConstraintTemplate *ct = Cast<UPhysicsConstraintTemplate>(StaticDuplicateObject(a, dstPA, NAME_None));
		ct->DefaultInstance.ConstraintBone1 = dstMesh->Skeleton->GetRigBoneMapping(rigName1);
		ct->DefaultInstance.ConstraintBone2 = dstMesh->Skeleton->GetRigBoneMapping(rigName2);
		ct->DefaultInstance.JointName = dstMesh->Skeleton->GetRigBoneMapping(rigNameJ);

		{
			auto ind1 = dstMesh->RefSkeleton.FindBoneIndex(ct->DefaultInstance.ConstraintBone1);
			auto ind2 = dstMesh->RefSkeleton.FindBoneIndex(ct->DefaultInstance.ConstraintBone2);

			auto t = dstMesh->RefSkeleton.GetRefBonePose()[ind1];
			while (1) {

				ind1 = dstMesh->RefSkeleton.GetParentIndex(ind1);
				if (ind1 == ind2) {
					break;
				}
				if (ind1 < 0) {
					break;
				}
				auto v = dstMesh->RefSkeleton.GetRefBonePose()[ind1].GetLocation();
				t.SetLocation(t.GetLocation() + v);
			}
			//auto indParent = dstMesh->RefSkeleton.GetParentIndex(ind1);


			ct->DefaultInstance.SetRefFrame(EConstraintFrame::Frame1, FTransform::Identity);
			ct->DefaultInstance.SetRefFrame(EConstraintFrame::Frame2, t);
		}
		{
			/*
			auto s1 = a->DefaultInstance->ProfileInstance.ConeLimit.Swing1Motion;
			auto s2 = a->DefaultInstance->ProfileInstance.ConeLimit.Swing1LimitDegrees;
			ct->DefaultInstance.SetAngularTwistLimit(s1, s2);

			s1 = a->DefaultInstance->ProfileInstance.ConeLimit.Swing2Motion;
			s2 = a->DefaultInstance->ProfileInstance.ConeLimit.Swing2LimitDegrees;

			//ct->DefaultInstance.SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, 10);
			//ct->DefaultInstance.SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, 10);
			*/
		}

		dstPA->ConstraintSetup.Add(ct);
	}

	dstPA->UpdateBodySetupIndexMap();

	dstPA->RefreshPhysicsAssetChange();
	dstPA->UpdateBoundsBodiesArray();

#endif

	return true;
}