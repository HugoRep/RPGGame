// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#include "VrmConvertRig.h"
#include "VrmConvert.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>
#include <assimp/vrm/vrmmeta.h>

#include "VrmAssetListObject.h"
#include "VrmMetaObject.h"
#include "VrmSkeleton.h"
#include "LoaderBPFunctionLibrary.h"

#include "Engine/SkeletalMesh.h"
#include "RenderingThread.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Animation/MorphTarget.h"
#include "Animation/NodeMappingContainer.h"
#include "Animation/Rig.h"
#include "Animation/PoseAsset.h"
#include "Animation/Skeleton.h"
#include "Components/SkeletalMeshComponent.h"

#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintTemplate.h"

#if WITH_EDITOR
#include "IPersonaToolkit.h"
#include "PersonaModule.h"
#include "Modules/ModuleManager.h"
#include "Animation/DebugSkelMeshComponent.h"
#endif


//#include "Engine/.h"

namespace {
#if WITH_EDITOR
	struct Table {
		FString BoneUE4;
		FString BoneVRM;
	};

	static const Table table[] = {
	{"Root",""},
	{"Pelvis","hips"},
	{"spine_01","spine"},
	{"spine_02","chest"},
	{"spine_03","upperChest"},
	{"clavicle_l","leftShoulder"},
	{"UpperArm_L","leftUpperArm"},
	{"lowerarm_l","leftLowerArm"},
	{"Hand_L","leftHand"},
	{"index_01_l","leftIndexProximal"},
	{"index_02_l","leftIndexIntermediate"},
	{"index_03_l","leftIndexDistal"},
	{"middle_01_l","leftMiddleProximal"},
	{"middle_02_l","leftMiddleIntermediate"},
	{"middle_03_l","leftMiddleDistal"},
	{"pinky_01_l","leftLittleProximal"},
	{"pinky_02_l","leftLittleIntermediate"},
	{"pinky_03_l","leftLittleDistal"},
	{"ring_01_l","leftRingProximal"},
	{"ring_02_l","leftRingIntermediate"},
	{"ring_03_l","leftRingDistal"},
	{"thumb_01_l","leftThumbProximal"},
	{"thumb_02_l","leftThumbIntermediate"},
	{"thumb_03_l","leftThumbDistal"},
	{"lowerarm_twist_01_l",""},
	{"upperarm_twist_01_l",""},
	{"clavicle_r","rightShoulder"},
	{"UpperArm_R","rightUpperArm"},
	{"lowerarm_r","rightLowerArm"},
	{"Hand_R","rightHand"},
	{"index_01_r","rightIndexProximal"},
	{"index_02_r","rightIndexIntermediate"},
	{"index_03_r","rightIndexDistal"},
	{"middle_01_r","rightMiddleProximal"},
	{"middle_02_r","rightMiddleIntermediate"},
	{"middle_03_r","rightMiddleDistal"},
	{"pinky_01_r","rightLittleProximal"},
	{"pinky_02_r","rightLittleIntermediate"},
	{"pinky_03_r","rightLittleDistal"},
	{"ring_01_r","rightRingProximal"},
	{"ring_02_r","rightRingIntermediate"},
	{"ring_03_r","rightRingDistal"},
	{"thumb_01_r","rightThumbProximal"},
	{"thumb_02_r","rightThumbIntermediate"},
	{"thumb_03_r","rightThumbDistal"},
	{"lowerarm_twist_01_r",""},
	{"upperarm_twist_01_r",""},
	{"neck_01","neck"},
	{"head","head"},
	{"Thigh_L","leftUpperLeg"},
	{"calf_l","leftLowerLeg"},
	{"calf_twist_01_l",""},
	{"Foot_L","leftFoot"},
	{"ball_l","leftToes"},
	{"thigh_twist_01_l",""},
	{"Thigh_R","rightUpperLeg"},
	{"calf_r","rightLowerLeg"},
	{"calf_twist_01_r",""},
	{"Foot_R","rightFoot"},
	{"ball_r","rightToes"},
	{"thigh_twist_01_r",""},
	{"ik_foot_root",""},
	{"ik_foot_l",""},
	{"ik_foot_r",""},
	{"ik_hand_root",""},
	{"ik_hand_gun",""},
	{"ik_hand_l",""},
	{"ik_hand_r",""},
	{"Custom_1",""},
	{"Custom_2",""},
	{"Custom_3",""},
	{"Custom_4",""},
	{"Custom_5",""},
	};
#endif

	static int GetChildBoneLocal(const FReferenceSkeleton &skeleton, const int32 ParentBoneIndex, TArray<int32> & Children) {
		Children.Reset();
		//auto &r = skeleton->GetReferenceSkeleton();
		auto &r = skeleton;

		const int32 NumBones = r.GetRawBoneNum();
		for (int32 ChildIndex = ParentBoneIndex + 1; ChildIndex < NumBones; ChildIndex++)
		{
			if (ParentBoneIndex == r.GetParentIndex(ChildIndex))
			{
				Children.Add(ChildIndex);
			}
		}
		return Children.Num();
	}

	static bool isSameOrChild(const FReferenceSkeleton &skeleton, const int32 TargetBoneIndex, const int32 SameOrChildBoneIndex) {
		auto &r = skeleton;

		int32 c = SameOrChildBoneIndex;
		for (int i = 0; i < skeleton.GetRawBoneNum(); ++i) {

			if (TargetBoneIndex < 0 || SameOrChildBoneIndex < 0) {
				return false;
			}

			if (c == TargetBoneIndex) {
				return true;
			}


			c = skeleton.GetParentIndex(c);
		}
		return false;
	}
}


bool VRMConverter::ConvertRig(UVrmAssetListObject *vrmAssetList, const aiScene *mScenePtr) {
#if	UE_VERSION_OLDER_THAN(4,20,0)
#else
#if WITH_EDITOR
	FString name = FString(TEXT("RIG_")) + vrmAssetList->BaseFileName;
	UNodeMappingContainer* mc = NewObject<UNodeMappingContainer>(vrmAssetList->Package, *name, RF_Public | RF_Standalone);

	auto *k = vrmAssetList->SkeletalMesh->Skeleton;
	vrmAssetList->SkeletalMesh->NodeMappingData.Add(mc);

	//USkeletalMeshComponent* PreviewMeshComp = PreviewScenePtr.Pin()->GetPreviewMeshComponent();
	//USkeletalMesh* PreviewMesh = PreviewMeshComp->SkeletalMesh;

	URig *EngineHumanoidRig = LoadObject<URig>(nullptr, TEXT("/Engine/EngineMeshes/Humanoid.Humanoid"), nullptr, LOAD_None, nullptr);
	//FSoftObjectPath r(TEXT("/Engine/EngineMeshes/Humanoid.Humanoid"));
	//UObject *u = r.TryLoad();
	mc->SetSourceAsset(EngineHumanoidRig);

	vrmAssetList->SkeletalMesh->Skeleton->SetRigConfig(EngineHumanoidRig);


	mc->SetTargetAsset(vrmAssetList->SkeletalMesh);
	mc->AddDefaultMapping();

	FString PelvisBoneName;
	{
		const VRM::VRMMetadata *meta = reinterpret_cast<VRM::VRMMetadata*>(mScenePtr->mVRMMeta);

		auto func = [&](const FString &a, const FString b) {
			mc->AddMapping(*a, *b);
			vrmAssetList->SkeletalMesh->Skeleton->SetRigBoneMapping(*a, *b);
		};
		auto func2 = [&](const FString &a, FName b) {
			func(a, b.ToString());
		};

		if (meta) {
			for (auto &t : table) {
				FString target = t.BoneVRM;
				const FString &ue4 = t.BoneUE4;

				if (ue4.Compare(TEXT("Root"), ESearchCase::IgnoreCase) == 0) {
					auto &a = vrmAssetList->SkeletalMesh->Skeleton->GetReferenceSkeleton().GetRefBoneInfo();
					target = a[0].Name.ToString();
				}

				if (target.Len() == 0) {
					continue;
				}


				for (auto b : meta->humanoidBone) {

					if (target.Compare(b.humanBoneName.C_Str()) != 0) {
						continue;
					}
					target = b.nodeName.C_Str();
					break;
				}

				if (PelvisBoneName.Len() == 0) {
					if (ue4.Compare(TEXT("Pelvis"), ESearchCase::IgnoreCase) == 0) {
						PelvisBoneName = target;
					}
				}
				func(ue4, target);
				//mc->AddMapping(*ue4, *target);
				//vrmAssetList->SkeletalMesh->Skeleton->SetRigBoneMapping(*ue4, *target);
			}
			{
				const TArray<FString> cc = {
					TEXT("Root"),
					TEXT("Pelvis"),
					TEXT("spine_01"),
					TEXT("spine_02"),
					TEXT("spine_03"),
					TEXT("neck_01"),
				};

				// find bone from child bone
				for (int i = cc.Num() - 2; i > 0; --i) {
					const auto &m = mc->GetNodeMappingTable();

					{
						const auto p0 = m.Find(*cc[i]);
						if (p0) {
							// map exist
							continue;
						}
					}
					const auto p = m.Find(*cc[i + 1]);
					if (p == nullptr) {
						// child none
						continue;
					}

					const FName *parentRoot = nullptr;
					for (int toParent = i - 1; toParent > 0; --toParent) {
						parentRoot = m.Find(*cc[toParent]);
						if (parentRoot) {
							break;
						}
					}
					if (parentRoot == nullptr) {
						continue;
					}

					// find (child) p -> (parent)parentRoot
					FString newTarget = parentRoot->ToString();
					{

						const int32 index = k->GetReferenceSkeleton().FindBoneIndex(*p);
						const int32 indexParent = k->GetReferenceSkeleton().GetParentIndex(index);
						const int32 indexRoot = k->GetReferenceSkeleton().FindBoneIndex(*parentRoot);

						if (isSameOrChild(k->GetReferenceSkeleton(), indexRoot, indexParent)) {
							newTarget = k->GetReferenceSkeleton().GetBoneName(indexParent).ToString();
						}
					}
					func(cc[i], newTarget);
				}

				// set null -> parent bone
				for (int i = 1; i < cc.Num(); ++i) {
					const auto &m = mc->GetNodeMappingTable();

					{
						const auto p0 = m.Find(*cc[i]);
						if (p0) {
							// map exist
							continue;
						}
					}
					const auto pp = m.Find(*cc[i-1]);
					if (pp == nullptr) {
						// parent none
						continue;
					}

					// map=nullptr, parent=exist
					FString newTarget = pp->ToString();

					{
						int32 index = k->GetReferenceSkeleton().FindBoneIndex(*pp);
						TArray<int32> child;
						GetChildBoneLocal(k->GetReferenceSkeleton(), index, child);
						if (child.Num() == 1) {
							// use one child
							// need neck check...
							//newTarget = k->GetReferenceSkeleton().GetBoneName(child[0]).ToString();
						}
					}

					func(cc[i], newTarget);
				}
			}
		} else {
			// auto mapping
			
			const auto &rSk = k->GetReferenceSkeleton(); //EngineHumanoidRig->GetSourceReferenceSkeleton();
			
			TArray<FString> rBoneList;
			{
				for (auto &a : rSk.GetRawRefBoneInfo()) {
					rBoneList.Add(a.Name.ToString());
				}
			}
			TArray<FString> existTable;
			int boneIndex = -1;
			for (auto &b : rBoneList) {
				++boneIndex;

				if (b.Find(TEXT("neck")) >= 0) {
					const FString t = TEXT("neck_01");
					if (existTable.Find(t) >= 0) {
						continue;
					}
					existTable.Add(t);
					func(t, b);

					int p = boneIndex;

					const TArray<FString> cc = {
						TEXT("spine_03"),
						TEXT("spine_02"),
						TEXT("spine_01"),
						TEXT("Pelvis"),
						TEXT("Root"),
					};

					for (auto &a : cc) {
						int p2 = rSk.GetParentIndex(p);
						if (p2 >= 0) {
							p = p2;
						}
						func2(a, rSk.GetBoneName(p));
					}
				}
				if (b.Find(TEXT("head")) >= 0) {
					const FString t = TEXT("head");
					if (existTable.Find(t) >= 0) {
						continue;
					}
					existTable.Add(t);
					func(t, b);
				}

				if (b.Find(TEXT("hand")) >= 0) {
					if (b.Find("r") >= 0) {
						const FString t = TEXT("Hand_R");
						if (existTable.Find(t) >= 0){
							continue;
						}
						existTable.Add(t);

						func(t, b);

						int p = rSk.GetParentIndex(boneIndex);
						func2(TEXT("lowerarm_r"), rSk.GetBoneName(p));

						p = rSk.GetParentIndex(p);
						func2(TEXT("UpperArm_R"), rSk.GetBoneName(p));

						p = rSk.GetParentIndex(p);
						func2(TEXT("clavicle_r"), rSk.GetBoneName(p));
					}
					if (b.Find("l") >= 0) {
						const FString t = TEXT("Hand_L");
						if (existTable.Find(t) >= 0) {
							continue;
						}
						existTable.Add(t);

						func(t, b);

						int p = rSk.GetParentIndex(boneIndex);
						func2(TEXT("lowerarm_l"), rSk.GetBoneName(p));

						p = rSk.GetParentIndex(p);
						func2(TEXT("UpperArm_L"), rSk.GetBoneName(p));

						p = rSk.GetParentIndex(p);
						func2(TEXT("clavicle_l"), rSk.GetBoneName(p));
					}
				}
				if (b.Find(TEXT("foot")) >= 0) {
					if (b.Find("r") >= 0) {
						const FString t = TEXT("Foot_R");
						if (existTable.Find(t) >= 0) {
							continue;
						}
						existTable.Add(t);

						func(t, b);

						{
							TArray<int32> c;
							GetChildBoneLocal(rSk, boneIndex, c);
							if (c.Num()) {
								func2(TEXT("ball_r"), rSk.GetBoneName(c[0]));
							}
						}

						int p = rSk.GetParentIndex(boneIndex);
						func2(TEXT("calf_r"), rSk.GetBoneName(p));

						p = rSk.GetParentIndex(p);
						func2(TEXT("Thigh_R"), rSk.GetBoneName(p));
					}
					if (b.Find("l") >= 0) {
						const FString t = TEXT("Foot_L");
						if (existTable.Find(t) >= 0) {
							continue;
						}
						existTable.Add(t);

						func(t, b);

						{
							TArray<int32> c;
							GetChildBoneLocal(rSk, boneIndex, c);
							if (c.Num()) {
								func2(TEXT("ball_l"), rSk.GetBoneName(c[0]));
							}
						}

						int p = rSk.GetParentIndex(boneIndex);
						func2(TEXT("calf_l"), rSk.GetBoneName(p));

						p = rSk.GetParentIndex(p);
						func2(TEXT("Thigh_L"), rSk.GetBoneName(p));
					}
				}
			}
		}
	}

	{
		int bone = -1;
		for (int i = 0; i < k->GetReferenceSkeleton().GetRawBoneNum(); ++i) {
			//const int32 BoneIndex = k->GetReferenceSkeleton().FindBoneIndex(InBoneName);
			k->SetBoneTranslationRetargetingMode(i, EBoneTranslationRetargetingMode::Skeleton);
			//FAssetNotifications::SkeletonNeedsToBeSaved(k);
			if (k->GetReferenceSkeleton().GetBoneName(i).Compare(*PelvisBoneName) == 0) {
				bone = i;
			}
		}

		bool first = true;
		while(bone >= 0){
			if (first) {
				k->SetBoneTranslationRetargetingMode(bone, EBoneTranslationRetargetingMode::AnimationScaled);
			} else {
				k->SetBoneTranslationRetargetingMode(bone, EBoneTranslationRetargetingMode::Animation);
			}
			first = false;

			bone = k->GetReferenceSkeleton().GetParentIndex(bone);
		}

		if (VRMConverter::Options::Get().IsVRMModel() == false) {
			k->SetBoneTranslationRetargetingMode(0, EBoneTranslationRetargetingMode::Animation, false);
		}

	}
	//mc->AddMapping
	mc->PostEditChange();
	vrmAssetList->HumanoidRig = mc;

#if 1
	{
		USkeletalMesh *sk = vrmAssetList->SkeletalMesh;

		FString name = FString(TEXT("POSE_")) + vrmAssetList->BaseFileName;
		UPoseAsset *pose = NewObject<UPoseAsset>(vrmAssetList->Package, *name, RF_Public | RF_Standalone);
		pose->SetSkeleton(k);
		pose->SetPreviewMesh(sk);

		FSmartName PoseName;

		//USkeletalMeshComponent *smc = nullptr;
		{
			FPersonaModule& PersonaModule = FModuleManager::LoadModuleChecked<FPersonaModule>("Persona");
			auto PersonaToolkit = PersonaModule.CreatePersonaToolkit(sk);

			UDebugSkelMeshComponent* PreviewComponent = PersonaToolkit->GetPreviewMeshComponent();

			auto *kk = Cast<USkeletalMeshComponent>(PreviewComponent);
			kk->SetComponentSpaceTransformsDoubleBuffering(false);

			pose->AddOrUpdatePoseWithUniqueName(Cast<USkeletalMeshComponent>(PreviewComponent), &PoseName);
			pose->RenameSmartName(PoseName.DisplayName, TEXT("POSE_T"));

			//pose->AddOrUpdatePose(PoseName, Cast<USkeletalMeshComponent>(PreviewComponent));

			auto &dstTrans = kk->GetEditableComponentSpaceTransforms();
			{
				struct RetargetParts {
					FString BoneUE4;
					FString BoneVRM;
					FString BoneModel;

					FRotator rot;
				};
				TArray<RetargetParts> retargetTable;
				TMap<FString, RetargetParts> mapTable;

				{
					RetargetParts t;
					t.BoneUE4 = TEXT("UpperArm_R");
					t.rot = FRotator(40, 0, 0);
					retargetTable.Push(t);
				}
				{
					RetargetParts t;
					t.BoneUE4 = TEXT("lowerarm_r");
					t.rot = FRotator(0, -30, 0);
					retargetTable.Push(t);
				}
				{
					RetargetParts t;
					t.BoneUE4 = TEXT("Hand_R");
					t.rot = FRotator(10, 0, 0);
					retargetTable.Push(t);
				}
				{
					RetargetParts t;
					t.BoneUE4 = TEXT("UpperArm_L");
					t.rot = FRotator(-40, 0, 0);
					retargetTable.Push(t);
				}
				{
					RetargetParts t;
					t.BoneUE4 = TEXT("lowerarm_l");
					t.rot = FRotator(-0, 30, 0);
					retargetTable.Push(t);
				}
				{
					RetargetParts t;
					t.BoneUE4 = TEXT("Hand_L");
					t.rot = FRotator(-10, 0, 0);
					retargetTable.Push(t);
				}
				{
					RetargetParts t;
					t.BoneUE4 = TEXT("Thigh_R");
					t.rot = FRotator(-7, 0, 0);
					retargetTable.Push(t);
				}
				{
					RetargetParts t;
					t.BoneUE4 = TEXT("Thigh_L");
					t.rot = FRotator(7, 0, 0);
					retargetTable.Push(t);
				}

				for (auto &a : retargetTable) {
					for (auto &t : table) {
						if (t.BoneUE4.Compare(a.BoneUE4) != 0) {
							continue;
						}
						auto *m = vrmAssetList->VrmMetaObject->humanoidBoneTable.Find(t.BoneVRM);
						a.BoneVRM = t.BoneVRM;
						a.BoneModel = *m;
						mapTable.Add(a.BoneModel, a);
						break;
					}
				}

				auto &rk = k->GetReferenceSkeleton();

				// init retarget pose
				for (int i = 0; i < dstTrans.Num(); ++i) {
					auto &t = dstTrans[i];
					t = rk.GetRefBonePose()[i];
				}
				sk->RetargetBasePose = dstTrans;

				// override
				for (int i = 0; i < dstTrans.Num(); ++i) {
					auto &t = dstTrans[i];

					auto *m = mapTable.Find(rk.GetBoneName(i).ToString());
					if (m) {
						t.SetRotation(FQuat(m->rot));
					}
				}

				if (VRMConverter::Options::Get().IsAPoseRetarget() == true) {
					sk->RetargetBasePose = dstTrans;
				}

				for (int i = 0; i < dstTrans.Num(); ++i) {
					int parent = rk.GetParentIndex(i);
					if (parent == INDEX_NONE) continue;
					
					dstTrans[i] = dstTrans[i] * dstTrans[parent];
				}
			}
			pose->AddOrUpdatePoseWithUniqueName(Cast<USkeletalMeshComponent>(PreviewComponent), &PoseName);
			pose->RenameSmartName(PoseName.DisplayName, TEXT("POSE_A"));

		}
	}
#endif

#endif
#endif //420

	if (vrmAssetList){
		// dummy Collision

		const VRM::VRMMetadata *meta = reinterpret_cast<VRM::VRMMetadata*>(mScenePtr->mVRMMeta);
		USkeletalMesh *sk = vrmAssetList->SkeletalMesh;
		UPhysicsAsset *pa = sk->PhysicsAsset;
		FString h = TEXT("hips");
		if (meta && pa){
			for (const auto &a : meta->humanoidBone) {

				if (h.Compare(a.humanBoneName.C_Str())) {
					continue;
				}

				const int targetBone = sk->RefSkeleton.FindRawBoneIndex(a.nodeName.C_Str());
				if (targetBone == INDEX_NONE) {
					break;
				}

				FVector center(0, 0, 0);
				{
					int i = targetBone;
					while (sk->RefSkeleton.GetParentIndex(i) != INDEX_NONE) {

						center += sk->RefSkeleton.GetRefBonePose()[i].GetLocation();
						i = sk->RefSkeleton.GetParentIndex(i);
					}
				}

				USkeletalBodySetup *bs = nullptr;
				int BodyIndex1 = -1;

				bs = NewObject<USkeletalBodySetup>(pa, TEXT("dummy_for_clip"));

				FKAggregateGeom agg;
				FKSphereElem SphereElem;
				SphereElem.Center = FVector(0);
				SphereElem.Radius = center.Size();
				agg.SphereElems.Add(SphereElem);

				bs->Modify();
				bs->BoneName = a.nodeName.C_Str();
				bs->AddCollisionFrom(agg);
				bs->CollisionTraceFlag = CTF_UseSimpleAsComplex;
				// newly created bodies default to simulating
				bs->PhysicsType = PhysType_Kinematic;	// fix
														//bs->get
				bs->CollisionReponse = EBodyCollisionResponse::BodyCollision_Disabled;
				bs->DefaultInstance.InertiaTensorScale.Set(2, 2, 2);
				bs->DefaultInstance.LinearDamping = 0.f;
				bs->DefaultInstance.AngularDamping = 0.f;

				bs->InvalidatePhysicsData();
				bs->CreatePhysicsMeshes();
				BodyIndex1 = pa->SkeletalBodySetups.Add(bs);
				break;
			}
		}
	}

	return true;

}


