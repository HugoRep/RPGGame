// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#include "VrmConvertHumanoid.h"
#include "VrmConvert.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>
#include <assimp/vrm/vrmmeta.h>

#include "VrmAssetListObject.h"
#include "VrmMetaObject.h"

#include "Engine/SkeletalMesh.h"
#include "RenderingThread.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Animation/MorphTarget.h"
#include "Animation/Skeleton.h"

static void renameToHumanoidBone(USkeleton *targetSkeleton, const UVrmMetaObject *meta) {
	if (meta == nullptr) {
		return;
	}
	//k->RemoveBonesFromSkeleton()
	auto &allbone = const_cast<TArray<FMeshBoneInfo> &>(targetSkeleton->GetReferenceSkeleton().GetRawRefBoneInfo());

	for (auto &a : allbone) {
		auto p = meta->humanoidBoneTable.FindKey(a.Name.ToString());
		if (p == nullptr) {
			continue;
		}
		for (auto &b : allbone) {
			if (a == b) continue;
			if (b.Name == **p) {
				b.Name = *(b.Name.ToString() + TEXT("_renamed_vrm4u"));
			}
		}
		a.Name = **p;
	}

	const_cast<FReferenceSkeleton&>(targetSkeleton->GetReferenceSkeleton()).RebuildRefSkeleton(targetSkeleton, true);
	//targetSkeleton->HandleSkeletonHierarchyChange();
}



bool VRMConverter::ConvertHumanoid(UVrmAssetListObject *vrmAssetList, const aiScene *mScenePtr) {
	if (Options::Get().IsCreateHumanoidRenamedMesh() == false) {
		return true;
	}

	const USkeletalMesh *sk = vrmAssetList->SkeletalMesh;
	const USkeleton *k = sk->Skeleton;

	USkeleton *base = DuplicateObject<USkeleton>(k, vrmAssetList->Package, *(TEXT("SKEL_") + vrmAssetList->BaseFileName + TEXT("_humanoid")));
	USkeletalMesh *ss = DuplicateObject<USkeletalMesh>(sk, vrmAssetList->Package, *(FString(TEXT("SK_")) + vrmAssetList->BaseFileName + TEXT("_humanoid")));

	renameToHumanoidBone(base, vrmAssetList->VrmMetaObject);

	ss->Skeleton = base;
	ss->RefSkeleton = base->GetReferenceSkeleton();

	ss->CalculateInvRefMatrices();
	ss->CalculateExtendedBounds();
#if WITH_EDITORONLY_DATA
	ss->ConvertLegacyLODScreenSize();
#if	UE_VERSION_OLDER_THAN(4,20,0)
#else
	ss->UpdateGenerateUpToData();
#endif
#endif

#if WITH_EDITORONLY_DATA
	base->SetPreviewMesh(ss);
#endif
	base->RecreateBoneTree(ss);

	vrmAssetList->HumanoidSkeletalMesh = ss;

	return true;
}

VrmConvertHumanoid::VrmConvertHumanoid()
{
}

VrmConvertHumanoid::~VrmConvertHumanoid()
{
}
