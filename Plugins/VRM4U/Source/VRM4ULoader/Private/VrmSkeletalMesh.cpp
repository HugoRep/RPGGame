// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#include "VrmSkeletalMesh.h"
#include "VrmSkeleton.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/SkeletalMeshRenderData.h"





void UVrmSkeletalMesh::Proc() {

	//UMySkeleton *p = Cast<UMySkeleton> Skeleton;


	RefSkeleton = Skeleton->GetReferenceSkeleton();

	FSkeletalMeshRenderData *p = GetResourceForRendering();

	FSkeletalMeshLODRenderData d;
	p->LODRenderData.Add(&d);

	
	//d.StaticVertexBuffers.InitFromDynamicVertex;
	
	//d.SkinWeightVertexBuffer.InitResource



	/*
	FReferenceSkeletonModifier RefSkelModifier(RefSkeleton, this);

	FMeshBoneInfo NewMeshBoneInfo = ReferenceSkeleton.GetRefBoneInfo()[0];
	NewMeshBoneInfo.ParentIndex = 1;
	NewMeshBoneInfo.Name = TEXT("tttrr");

	FTransform t;
	t.SetLocation(FVector(10, 10, 10));

	RefSkelModifier.Add(NewMeshBoneInfo, t);
	BoneTree.AddZeroed(1);

	HandleSkeletonHierarchyChange();
	//	bShouldHandleHierarchyChange = true;
	//ReferenceSkeleton.
	*/

	//RefSkeleton.BoneIsChildOf

}