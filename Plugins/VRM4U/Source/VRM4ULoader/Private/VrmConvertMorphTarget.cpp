// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#include "VrmConvertMorphTarget.h"
#include "VrmConvert.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>
#include <assimp/vrm/vrmmeta.h>

#include "VrmAssetListObject.h"
#include "VrmSkeleton.h"
#include "LoaderBPFunctionLibrary.h"

#include "Engine/SkeletalMesh.h"
#include "RenderingThread.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Animation/MorphTarget.h"

#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintTemplate.h"

#include "Async/ParallelFor.h"


static bool readMorph2(TArray<FMorphTargetDelta> &MorphDeltas, aiString targetName,const aiScene *mScenePtr) {

	//return readMorph33(MorphDeltas, targetName, mScenePtr);

	MorphDeltas.Reset(0);
	uint32_t currentVertex = 0;

	FMorphTargetDelta morphinit;
	morphinit.PositionDelta = FVector::ZeroVector;
	morphinit.SourceIdx = 0;
	morphinit.TangentZDelta = FVector::ZeroVector;

	for (uint32_t m = 0; m < mScenePtr->mNumMeshes; ++m) {
		const aiMesh &aiM = *(mScenePtr->mMeshes[m]);

		for (uint32_t a = 0; a < aiM.mNumAnimMeshes; ++a) {
			const aiAnimMesh &aiA = *(aiM.mAnimMeshes[a]);
			if (targetName != aiA.mName) {
				continue;
			}

			if (aiM.mNumVertices != aiA.mNumVertices) {
				UE_LOG(LogTemp, Warning, TEXT("test18.\n"));
			}

			TArray<FMorphTargetDelta> tmpData;
			tmpData.SetNumZeroed(aiA.mNumVertices);

			bool bIncludeNormal = VRMConverter::Options::Get().IsEnableMorphTargetNormal();

			ParallelFor(aiA.mNumVertices, [&](int32 i) {
				FMorphTargetDelta &v = tmpData[i];
				v.SourceIdx = i + currentVertex;
				v.PositionDelta.Set(
					-aiA.mVertices[i][0] * 100.f,
					aiA.mVertices[i][2] * 100.f,
					aiA.mVertices[i][1] * 100.f
				);

				if (bIncludeNormal) {
					const FVector n(
						-aiA.mNormals[i][0],
						aiA.mNormals[i][2],
						aiA.mNormals[i][1]);
					if (n.Size() > 1.f) {
						v.TangentZDelta = n.GetUnsafeNormal();
					}
				}
			});
			MorphDeltas.Append(tmpData);
		}
		currentVertex += aiM.mNumVertices;
	}
	return MorphDeltas.Num() != 0;
}


bool VRMConverter::ConvertMorphTarget(UVrmAssetListObject *vrmAssetList, const aiScene *mScenePtr) {
#if WITH_EDITOR
	if (Options::Get().IsSkipMorphTarget()) {
		return true;
	}

	USkeletalMesh *sk = vrmAssetList->SkeletalMesh;

	{
		///sk->MarkPackageDirty();
		// need to refresh the map
		//sk->InitMorphTargets();
		// invalidate render data
		//sk->InvalidateRenderData();
		//return true;
	}

	TArray<FSoftSkinVertex> sVertex;
	sk->GetImportedModel()->LODModels[0].GetVertices(sVertex);
	//mScenePtr->mMeshes[0]->mAnimMeshes[0]->mWeight

	TArray<FString> MorphNameList;

	TArray<UMorphTarget*> MorphTargetList;

	for (uint32_t m = 0; m < mScenePtr->mNumMeshes; ++m) {
		const aiMesh &aiM = *(mScenePtr->mMeshes[m]);
		for (uint32_t a = 0; a < aiM.mNumAnimMeshes; ++a) {
			const aiAnimMesh &aiA = *(aiM.mAnimMeshes[a]);
			//aiA.
			TArray<FMorphTargetDelta> MorphDeltas;

			FString morphName = UTF8_TO_TCHAR(aiA.mName.C_Str());
			if (morphName == TEXT("")) {
				//morphName = FString::Printf("%d_%d", m, a);
			}


			if (MorphNameList.Find(morphName) != INDEX_NONE) {
				continue;
			}
			MorphNameList.Add(morphName);
			if (readMorph2(MorphDeltas, aiA.mName, mScenePtr) == false) {
				continue;
			}

			//FString sss = FString::Printf(TEXT("%02d_%02d_"), m, a) + FString(aiA.mName.C_Str());
			FString sss = morphName;// FString::Printf(TEXT("%02d_%02d_"), m, a) + FString();
			UMorphTarget *mt = NewObject<UMorphTarget>(sk, *sss);

			mt->PopulateDeltas(MorphDeltas, 0, sk->GetImportedModel()->LODModels[0].Sections);

			if (mt->HasValidData()) {
				MorphTargetList.Add(mt);
			}
		}
	}
	for (int i=0; i<MorphTargetList.Num(); ++i){
		auto *mt = MorphTargetList[i];
		if (i == MorphTargetList.Num() - 1) {
			sk->RegisterMorphTarget(mt);
		} else {
			sk->MorphTargets.Add(mt);
		}
	}

#endif
	return true;
}


VrmConvertMorphTarget::VrmConvertMorphTarget()
{
}

VrmConvertMorphTarget::~VrmConvertMorphTarget()
{
}
