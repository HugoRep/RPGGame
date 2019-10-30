// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Animation/Skeleton.h"
#include "Misc/EngineVersionComparison.h"
#include "VrmSkeleton.generated.h"

/**
 * 
 */
UCLASS()
class VRM4ULOADER_API UVrmSkeleton : public USkeleton
{
	GENERATED_BODY()
	
public:
	/** IInterface_PreviewMeshProvider interface */
#if	UE_VERSION_OLDER_THAN(4,20,0)
#else
	virtual USkeletalMesh* GetPreviewMesh(bool bFindIfNotSet = false) override;
	virtual USkeletalMesh* GetPreviewMesh() const override;
	virtual void SetPreviewMesh(USkeletalMesh* PreviewMesh, bool bMarkAsDirty = true);
#endif

	virtual bool IsPostLoadThreadSafe() const override;

	TSoftObjectPtr<class USkeletalMesh> PreviewSkeletalMesh;

	///
public:
	void applyBoneFrom(const class USkeleton *src, const class UVrmMetaObject *meta);
	void readVrmBone(struct aiScene* s, int &offset);
	
	//FReferenceSkeleton& getRefSkeleton();
};
