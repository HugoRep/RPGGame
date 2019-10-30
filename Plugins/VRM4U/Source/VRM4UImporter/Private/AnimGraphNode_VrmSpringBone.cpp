// VRM4U Copyright (c) 2019 Haruyoshi Yamamoto. This software is released under the MIT License.

#include "AnimGraphNode_VrmSpringBone.h"
#include "UnrealWidget.h"
#include "AnimNodeEditModes.h"
#include "Kismet2/CompilerResultsLog.h"
#include "VrmMetaObject.h"

/////////////////////////////////////////////////////
// UAnimGraphNode_ModifyBone

#define LOCTEXT_NAMESPACE "A3Nodesaaaa"

UAnimGraphNode_VrmSpringBone::UAnimGraphNode_VrmSpringBone(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CurWidgetMode = (int32)FWidget::WM_Rotate;
}

void UAnimGraphNode_VrmSpringBone::ValidateAnimNodePostCompile(FCompilerResultsLog& MessageLog, UAnimBlueprintGeneratedClass* CompiledClass, int32 CompiledNodeIndex) {

	if (Node.VrmMetaObject == nullptr) {
		MessageLog.Warning(*LOCTEXT("VrmNoMetaObject", "@@ - You must set VrmMetaObject").ToString(), this);
	} else {
		if (Node.VrmMetaObject->SkeletalMesh) {
			if (Node.VrmMetaObject->SkeletalMesh->Skeleton != CompiledClass->GetTargetSkeleton()) {
				MessageLog.Warning(*LOCTEXT("VrmDifferentSkeleton", "@@ - You must set VrmMetaObject has same skeleton").ToString(), this);
			}
		}
		if (CompiledClass->GetTargetSkeleton()->GetBoneTree().Num() <= 0) {
			MessageLog.Warning(*LOCTEXT("VrmNoBone", "@@ - Skeleton bad data").ToString(), this);
		}
	}

	Super::ValidateAnimNodePostCompile(MessageLog, CompiledClass, CompiledNodeIndex);
}

void UAnimGraphNode_VrmSpringBone::ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog)
{
	// Temporary fix where skeleton is not fully loaded during AnimBP compilation and thus virtual bone name check is invalid UE-39499 (NEED FIX) 
	if (ForSkeleton && !ForSkeleton->HasAnyFlags(RF_NeedPostLoad))
	{
		if (Node.VrmMetaObject == nullptr) {
			//MessageLog.Warning(*LOCTEXT("VrmNoMetaObject", "@@ - You must set VrmMetaObject").ToString(), this);
		} else {
			if (Node.VrmMetaObject->SkeletalMesh){
				if (Node.VrmMetaObject->SkeletalMesh->Skeleton != ForSkeleton) {
			//		MessageLog.Warning(*LOCTEXT("VrmDifferentSkeleton", "@@ - You must set VrmMetaObject has same skeleton").ToString(), this);
				}
			}
			if (ForSkeleton->GetBoneTree().Num() <= 0) {
			//	MessageLog.Warning(*LOCTEXT("VrmNoBone", "@@ - Skeleton bad data").ToString(), this);
			}
		}


		/*
		//if (ForSkeleton->GetReferenceSkeleton().FindBoneIndex(Node.BoneToModify.BoneName) == INDEX_NONE)
		if (ForSkeleton->GetReferenceSkeleton().FindBoneIndex(Node.BoneNameToModify) == INDEX_NONE)
		{
			//if (Node.BoneToModify.BoneName == NAME_None)
			if (Node.BoneNameToModify == NAME_None)
			{
				MessageLog.Warning(*LOCTEXT("NoBoneSelectedToModify", "@@ - You must pick a bone to modify").ToString(), this);
			}
			else
			{
				FFormatNamedArguments Args;
				//Args.Add(TEXT("BoneName"), FText::FromName(Node.BoneToModify.BoneName));
				Args.Add(TEXT("BoneName"), FText::FromName(Node.BoneNameToModify));

				FText Msg = FText::Format(LOCTEXT("NoBoneFoundToModify", "@@ - Bone {BoneName} not found in Skeleton"), Args);

				MessageLog.Warning(*Msg.ToString(), this);
			}
		}
		*/
	}

	//if ((Node.TranslationMode == BMM_Ignore) && (Node.RotationMode == BMM_Ignore) && (Node.ScaleMode == BMM_Ignore))
	{
	//	MessageLog.Warning(*LOCTEXT("NothingToModify", "@@ - No components to modify selected.  Either Rotation, Translation, or Scale should be set to something other than Ignore").ToString(), this);
	}

	Super::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);
}

FText UAnimGraphNode_VrmSpringBone::GetControllerDescription() const
{
	return LOCTEXT("VrmSpringBone", "VrmSpringBone");
}

FText UAnimGraphNode_VrmSpringBone::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_VrmSpringBone_Tooltip", "VrmCopyHandBone");
}

FText UAnimGraphNode_VrmSpringBone::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	//if ((TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle) && (Node.BoneToModify.BoneName == NAME_None))
	//if ((TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle) && (Node.BoneNameToModify == NAME_None))
	//{
		return GetControllerDescription();
	//}
	// @TODO: the bone can be altered in the property editor, so we have to 
	//        choose to mark this dirty when that happens for this to properly work
		/*
	else //if (!CachedNodeTitles.IsTitleCached(TitleType, this))
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("ControllerDescription"), GetControllerDescription());
		//Args.Add(TEXT("BoneName"), FText::FromName(Node.BoneToModify.BoneName));
		Args.Add(TEXT("BoneName"), FText::FromName(Node.BoneNameToModify));

		// FText::Format() is slow, so we cache this to save on performance
		if (TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle)
		{
			CachedNodeTitles.SetCachedTitle(TitleType, FText::Format(LOCTEXT("AnimGraphNode_ModifyBone_ListTitle", "{ControllerDescription} - Bone: {BoneName}"), Args), this);
		}
		else
		{
			CachedNodeTitles.SetCachedTitle(TitleType, FText::Format(LOCTEXT("AnimGraphNode_ModifyBone_Title", "{ControllerDescription}\nBone: {BoneName}"), Args), this);
		}
	}
	return CachedNodeTitles[TitleType];
	*/
}

void UAnimGraphNode_VrmSpringBone::CopyNodeDataToPreviewNode(FAnimNode_Base* InPreviewNode)
{
	FAnimNode_VrmSpringBone* node = static_cast<FAnimNode_VrmSpringBone*>(InPreviewNode);

	// no copy
}

//FEditorModeID UAnimGraphNode_VrmModifyBoneDynamic::GetEditorMode() const
//{
//	return AnimNodeEditModes::ModifyBone;
//}

void UAnimGraphNode_VrmSpringBone::CopyPinDefaultsToNodeData(UEdGraphPin* InPin)
{
	/*
	if (InPin->GetName() == GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_ModifyBone, Translation))
	{
		GetDefaultValue(GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_ModifyBone, Translation), Node.Translation);
	}
	else if (InPin->GetName() == GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_ModifyBone, Rotation))
	{
		GetDefaultValue(GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_ModifyBone, Rotation), Node.Rotation);
	}
	else if (InPin->GetName() == GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_ModifyBone, Scale))
	{
		GetDefaultValue(GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_ModifyBone, Scale), Node.Scale);
	}
	*/
}

#undef LOCTEXT_NAMESPACE
