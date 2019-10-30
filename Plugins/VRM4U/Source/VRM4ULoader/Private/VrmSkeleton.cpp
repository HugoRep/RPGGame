// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#include "VrmSkeleton.h"
#include "VrmAssetListObject.h"
#include "VrmMetaObject.h"
#include "Engine/SkeletalMesh.h"
//#include "UnStringConv.h

#include <assimp/scene.h>       // Output data structure
#include <assimp/mesh.h>       // Output data structure

#if WITH_EDITOR
#include "VrmConvert.h"
#endif

#if	UE_VERSION_OLDER_THAN(4,20,0)
#else
USkeletalMesh* UVrmSkeleton::GetPreviewMesh(bool bFindIfNotSet)
{
	//return Super::GetPreviewMesh(bFindIfNotSet);
#if WITH_EDITORONLY_DATA
	USkeletalMesh* PreviewMesh = PreviewSkeletalMesh.LoadSynchronous();

	if(PreviewMesh && PreviewMesh->Skeleton != this) // fix mismatched skeleton
	{
		PreviewSkeletalMesh.Reset();
		PreviewMesh = nullptr;
	}

	// if not existing, and if bFindIfNotExisting is true, then try find one
	if(!PreviewMesh && bFindIfNotSet)
	{
		USkeletalMesh* CompatibleSkeletalMesh = FindCompatibleMesh();
		if(CompatibleSkeletalMesh)
		{
			SetPreviewMesh(CompatibleSkeletalMesh, false);
			// update PreviewMesh
			PreviewMesh = PreviewSkeletalMesh.Get();
		}
	}

	return PreviewMesh;
#else
	return nullptr;
#endif
}



USkeletalMesh* UVrmSkeleton::GetPreviewMesh() const
{
	//return Super::GetPreviewMesh();
#if WITH_EDITORONLY_DATA
	return PreviewSkeletalMesh.Get();
#else
	return nullptr;
#endif
}

void UVrmSkeleton::SetPreviewMesh(USkeletalMesh* PreviewMesh, bool bMarkAsDirty)
{
	//Super::SetPreviewMesh(PreviewMesh, bMarkAsDirty);
#if WITH_EDITORONLY_DATA
	if (bMarkAsDirty)
	{
		Modify();
	}

	PreviewSkeletalMesh = PreviewMesh;
#endif
}
#endif	// UE420

bool UVrmSkeleton::IsPostLoadThreadSafe() const
{
	return true;
}

static int countParent(const aiNode *node, TArray<const aiNode*> &t, int c) {
	for (auto &a : t) {
		for (uint32_t i = 0; i < a->mNumChildren; ++i) {
			if (node == a->mChildren[i]) {
				return countParent(a, t, c + 1);
			}
		}
	}
	return c;
}

static int countChild(aiNode *node, int c) {
	c += node->mNumChildren;
	for (uint32_t i = 0; i < node->mNumChildren; ++i) {
		c = countChild(node->mChildren[i], c);
	}
	return c;
}

bool findActiveBone(const aiNode *node, TArray<FString> &table) {

	if (table.Find(node->mName.C_Str()) != INDEX_NONE) {
		return true;
	}
	for (uint32_t i = 0; i < node->mNumChildren; ++i) {
		if (findActiveBone(node->mChildren[i], table)) {
			return true;
		}
	}
	return false;
}

TArray<FString> makeActiveBone(const aiScene *scene) {
	TArray <FString> boneNameTable;
	for (uint32 m = 0; m < scene->mNumMeshes; ++m) {
		const auto &aiM = *scene->mMeshes[m];

		for (uint32 b = 0; b < aiM.mNumBones; ++b) {
			const auto &aiB = *aiM.mBones[b];
			boneNameTable.AddUnique(aiB.mName.C_Str());
		}
	}
	return boneNameTable;
}


static bool hasMeshInChild(aiNode *node) {
	if (node == nullptr) {
		return false;
	}
	if (node->mNumMeshes > 0) {
		return true;
	}
	for (uint32_t i = 0; i < node->mNumChildren; ++i) {
		bool b = hasMeshInChild(node->mChildren[i]);
		if (b) {
			return true;
		}
	}
	return false;
}


static void rr3(const aiNode *node, TArray<const aiNode*> &t, bool &bHasMesh, const bool bOnlyRootBone) {
	bHasMesh = false;
	if (node == nullptr) {
		return;
	}
	if (node->mNumMeshes > 0) {
		bHasMesh = true;
	}

	t.Push(node);
	for (uint32_t i = 0; i < node->mNumChildren; ++i) {
		bool b = false;
		rr3(node->mChildren[i], t, b, bOnlyRootBone);

		if (b) {
			bHasMesh = true;
		}
	}
	if (bHasMesh==false && bOnlyRootBone) {
		//t.Remove(node);
	}
}

static void rr(const aiNode *node, TArray<const aiNode*> &t, bool &bHasMesh, const bool bOnlyRootBone, aiScene *scene) {

	auto target = node;

	if (bOnlyRootBone) {

		int maxIndex = -1;
		int maxChild = 0;

		// find deep tree
		for (uint32 i = 0; i < node->mNumChildren; ++i) {
			int cc = countChild(node->mChildren[i], 0);

			if (cc > maxChild) {
				maxChild = cc;
				maxIndex = i;
			}

		}

		auto table = makeActiveBone(scene);

		for (uint32 i = 0; i < node->mNumChildren; ++i) {
			if (i == maxIndex) {
				continue;
			}
			if (findActiveBone(node->mChildren[i], table)){
				maxIndex = -1;
				break;
			}
		}

		if (maxIndex >= 0) {

			const auto t = node->mChildren[maxIndex]->mTransformation;
			float f = 0.f;
			f += FMath::Abs(t.a4) + FMath::Abs(t.b4) + FMath::Abs(t.c4);
			if (f < 1.e-8f) {
				target = node->mChildren[maxIndex];
			} else {
				// root transform must zero.
			}
		}
	}

	rr3(target, t, bHasMesh, bOnlyRootBone);
}

void UVrmSkeleton::applyBoneFrom(const USkeleton *src, const UVrmMetaObject *meta) {
	//UVrmAssetListObject *alo;

	FReferenceSkeletonModifier RefSkelModifier(ReferenceSkeleton, this);

	{
		FTransform t;
		t.SetIdentity();

		for (int i = 0; i < ReferenceSkeleton.GetRawBoneNum(); ++i) {
			RefSkelModifier.UpdateRefPoseTransform(i, t);
		}
	}

	auto allbone = ReferenceSkeleton.GetRawRefBoneInfo();
	for (auto &a : allbone) {
		auto targetBoneName = meta->humanoidBoneTable.Find(a.Name.ToString());
		if (targetBoneName == nullptr) {
			continue;
		}

		auto index_src = src->GetReferenceSkeleton().FindRawBoneIndex(**targetBoneName);
		auto &t = src->GetReferenceSkeleton().GetRawRefBonePose()[index_src];

		auto index_my = GetReferenceSkeleton().FindRawBoneIndex(a.Name);

		RefSkelModifier.UpdateRefPoseTransform(index_my, t);
	}

	ReferenceSkeleton.RebuildRefSkeleton(this, true);
#if WITH_EDITOR
	HandleSkeletonHierarchyChange();
#endif
}

void UVrmSkeleton::readVrmBone(aiScene* scene, int &boneOffset) {

	boneOffset = 0;
	//FBoneNode n;

	//n.Name_DEPRECATED = TEXT("aaaaa");
	//n.ParentIndex_DEPRECATED = 1;
	//n.TranslationRetargetingMode

	//this->BoneTree.Push(n);


	FReferenceSkeletonModifier RefSkelModifier(ReferenceSkeleton, this);

	{
		FTransform t;
		t.SetIdentity();

		for (int i = 0; i < ReferenceSkeleton.GetRawBoneNum(); ++i) {
			RefSkelModifier.UpdateRefPoseTransform(i, t);
		}
	}


	{
		//const int offset = GetBoneTree().Num();
		//if (offset > 30) {
		//	return;
		//}
		//boneOffset = offset-1;

		TArray<const aiNode*> bone;
		{
			bool dummy = false;
			bool bSimpleRootBone = false;
#if WITH_EDITOR
			bSimpleRootBone = VRMConverter::Options::Get().IsSimpleRootBone();
#endif
			rr(scene->mRootNode, bone, dummy, bSimpleRootBone, scene);
		}

		{

			TArray<FString> rec_low;
			TArray<FString> rec_orig;
			for (int targetBondeID = 0; targetBondeID < bone.Num(); ++targetBondeID) {
				auto &a = bone[targetBondeID];

				FString str = UTF8_TO_TCHAR(a->mName.C_Str());
				auto f = rec_low.Find(str.ToLower());
				if (f >= 0) {
					// same name check
					//aiString origName = a->mName;
					//str += TEXT("_renamed_vrm4u_") + FString::Printf(TEXT("%02d"), targetBondeID);
					//a->mName.Set(TCHAR_TO_ANSI(*str));

					if (rec_orig[f] == UTF8_TO_TCHAR(a->mName.C_Str())) {
						// same name node!
						aiNode *n[] = {
							scene->mRootNode->FindNode(a->mName),
							scene->mRootNode->FindNode(TCHAR_TO_ANSI(*rec_orig[f])),
						};
						int t[] = {
							countParent(n[0], bone, 0),
							countParent(n[1], bone, 0),
						};

						char tmp[512];
						snprintf(tmp, 512, "%s_DUP", (n[0]->mName.C_Str()));
						if (t[0] < t[1]) {
							n[0]->mName = tmp;
						}else {
							n[1]->mName = tmp;
						}

						//countParent(

						//continue;
					}

					continue;

					TMap<FString, FString> renameTable;
					{
						FString s;
						s = rec_orig[f];
						s += TEXT("_renamed_vrm4u_") + FString::Printf(TEXT("%02d"), f);
						renameTable.FindOrAdd(rec_orig[f]) = s;

						s = UTF8_TO_TCHAR(a->mName.C_Str());
						s += TEXT("_renamed_vrm4u_") + FString::Printf(TEXT("%02d"), targetBondeID);
						renameTable.FindOrAdd(UTF8_TO_TCHAR(a->mName.C_Str())) = s;
						str = s;
					}


					//add
					for (uint32_t meshID = 0; meshID < scene->mNumMeshes; ++meshID) {
						auto &aiM = *(scene->mMeshes[meshID]);
						
						for (uint32_t allBoneID = 0; allBoneID < aiM.mNumBones; ++allBoneID) {
							auto &aiB = *(aiM.mBones[allBoneID]);
							auto res = renameTable.Find(UTF8_TO_TCHAR(aiB.mName.C_Str()));
							if (res) {
							//if (strcmp(aiB.mName.C_Str(), origName.C_Str()) == 0) {

								char tmp[512];
								//if (bone.Num() == aiM.mNumBones) {
								//	snprintf(tmp, 512, "%s_renamed_vrm4u_%02d", origName.C_Str(), allBoneID);
								//} else {
								//snprintf(tmp, 512, "%s", a->mName.C_Str());
								snprintf(tmp, 512, "%s", TCHAR_TO_ANSI(**res));
								//}
								//FString tmp = origName.C_Str();
								//tmp += TEXT("_renamed_vrm4u") + FString::Printf(TEXT("%02d"), allBoneID);

								aiB.mName = tmp;
							}
						}
					}
				}
				/*
				if (0) {
					// ascii code
					aiString origName = a->mName;
					str = TEXT("_renamed_vrm4u_") + FString::Printf(TEXT("%02d"), targetBondeID);
					a->mName.Set(TCHAR_TO_ANSI(*str));

					//add
					for (uint32_t meshID = 0; meshID < scene->mNumMeshes; ++meshID) {
						auto &aiM = *(scene->mMeshes[meshID]);

						for (uint32_t allBoneID = 0; allBoneID < aiM.mNumBones; ++allBoneID) {
							auto &aiB = *(aiM.mBones[allBoneID]);
							if (strcmp(aiB.mName.C_Str(), origName.C_Str()) == 0) {

								char tmp[512];
								if (bone.Num() == aiM.mNumBones) {
									snprintf(tmp, 512, "_renamed_vrm4u_%02d", allBoneID);
								}else {
									snprintf(tmp, 512, "_renamed_vrm4u_%02d", allBoneID + bone.Num()*targetBondeID);
								}
								//FString tmp = origName.C_Str();
								//tmp += TEXT("_renamed_vrm4u") + FString::Printf(TEXT("%02d"), allBoneID);

								aiB.mName = tmp;
							}
						}
					}

				}
				*/

				rec_low.Add(str.ToLower());
				rec_orig.Add(str);
			}
		}


		int totalBoneCount = 0;
		FVector RootTransOffset(0.f);

		for (auto &a: bone) {
			FMeshBoneInfo info;
			info.Name = UTF8_TO_TCHAR(a->mName.C_Str());
#if WITH_EDITORONLY_DATA
			info.ExportName = UTF8_TO_TCHAR(a->mName.C_Str());
#endif

			FTransform pose;
			int32 index = 0;
			FMatrix m;

			{
				auto &t = a->mTransformation;
				m.M[0][0] = t.a1; m.M[1][0] = t.a2; m.M[2][0] = t.a3; m.M[3][0] = -t.a4*100.f;
				m.M[0][1] = t.b1; m.M[1][1] = t.b2; m.M[2][1] = t.b3; m.M[3][1] = t.c4*100.f;//t.b4*100.f;
				m.M[0][2] = t.c1; m.M[1][2] = t.c2; m.M[2][2] = t.c3; m.M[3][2] = t.b4*100.f;//t.c4*100.f;
				m.M[0][3] = t.d1; m.M[1][3] = t.d2; m.M[2][3] = t.d3; m.M[3][3] = t.d4;

				if (VRMConverter::Options::Get().IsVRMModel() == false) {
					m.M[3][0] *= -1.f;
					m.M[3][1] *= -1.f;
				}
				{
					m.M[3][0] *= VRMConverter::Options::Get().GetModelScale();
					m.M[3][1] *= VRMConverter::Options::Get().GetModelScale();
					m.M[3][2] *= VRMConverter::Options::Get().GetModelScale();
					//m.M[3][3] *= VRMConverter::Options::Get().GetModelScale();
				}
			}


			if (bone.Find(a->mParent, index) == false) {
				//continue;
				// root
				info.ParentIndex = INDEX_NONE;

				RootTransOffset.Set(m.M[3][0], m.M[3][1], m.M[3][2]);

				m.SetIdentity();
			}
			else {
				if (index != 0) {
					//index += offset - 1;
				}
				info.ParentIndex = index;

				// parent == root
				if (index == 0) {
					m.M[3][0] += RootTransOffset.X;
					m.M[3][1] += RootTransOffset.Y;
					m.M[3][2] += RootTransOffset.Z;
				}


				if (a == scene->mRootNode) {
					//pose.SetScale3D(FVector(100));
				}
				
			}
			pose.SetFromMatrix(m);

			if (ReferenceSkeleton.FindRawBoneIndex(info.Name) != INDEX_NONE) {
				info.Name = *(info.Name.ToString() + FString(TEXT("_DUP")));
				//continue;
			}
			if (totalBoneCount > 0 && info.ParentIndex == INDEX_NONE) {
				// bad bone. root?
				continue;
			}
			RefSkelModifier.Add(info, pose);
			totalBoneCount++;

			if (totalBoneCount == 1) {
				if (VRMConverter::Options::Get().IsDebugOneBone()) {
					// root only
					break;
				}
			}
		}
	}



#if WITH_EDITOR
	HandleSkeletonHierarchyChange();
#endif

//	bShouldHandleHierarchyChange = true;
	//ReferenceSkeleton.
}