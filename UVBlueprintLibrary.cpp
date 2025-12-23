#include "UVBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"  // Gives access to USkeletalMeshComponent class
#include "Rendering/SkeletalMeshRenderData.h"  // For mesh vertex data
#include "Components/SkinnedMeshComponent.h"

bool UUVBlueprintLibrary::GetUVFromSkeletalMeshHit(
    UObject* WorldContextObject,
    const FHitResult& Hit,
    FVector2D& UV,
    int32 UVChannel)
{
    //Cast to USkinnedMeshComponent (base class)
    USkinnedMeshComponent* Mesh = Cast<USkinnedMeshComponent>(Hit.Component.Get());
    if (!Mesh || Hit.FaceIndex == INDEX_NONE) return false;

    //Get required data structures
    FSkeletalMeshRenderData* RenderData = Mesh->GetSkeletalMeshRenderData();
    if (!RenderData || RenderData->LODRenderData.Num() == 0) return false;

    FSkeletalMeshLODRenderData& LODData = RenderData->LODRenderData[0];

    // Validate UV channel exists
    const int32 NumUVChannels = LODData.StaticVertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords();
    if (UVChannel < 0 || UVChannel >= NumUVChannels) return false;
    
    FRawStaticIndexBuffer16or32Interface* Indices = LODData.MultiSizeIndexContainer.GetIndexBuffer();
    
    //Validate indices
    const int32 TriIndex = Hit.FaceIndex;
    if (TriIndex * 3 + 2 >= Indices->Num()) return false;

    //Get vertex indices
    const int32 Index0 = Indices->Get(TriIndex * 3);
    const int32 Index1 = Indices->Get(TriIndex * 3 + 1);
    const int32 Index2 = Indices->Get(TriIndex * 3 + 2);
    
    const FVector Pos0 = FVector(USkinnedMeshComponent::GetSkinnedVertexPosition(
        Mesh, 
        Index0,
        LODData,
        LODData.SkinWeightVertexBuffer
    ));

    const FVector Pos1 = FVector(USkinnedMeshComponent::GetSkinnedVertexPosition(
        Mesh,
        Index1,
        LODData,
        LODData.SkinWeightVertexBuffer
    ));

    const FVector Pos2 = FVector(USkinnedMeshComponent::GetSkinnedVertexPosition(
        Mesh,
        Index2,
        LODData,
        LODData.SkinWeightVertexBuffer
    ));
    
    const FVector2D UV0 = Mesh->GetVertexUV(Index0, 0);
    const FVector2D UV1 = Mesh->GetVertexUV(Index1, 0);
    const FVector2D UV2 = Mesh->GetVertexUV(Index2, 0);

    const FVector LocalHitPos = Mesh->GetComponentTransform().InverseTransformPosition(Hit.Location);
    const FVector BaryCoords = FMath::ComputeBaryCentric2D(LocalHitPos, Pos0, Pos1, Pos2);

    UV = (BaryCoords.X * UV0) + (BaryCoords.Y * UV1) + (BaryCoords.Z * UV2);

    return true;
}



bool UUVBlueprintLibrary::GetHitResultPointInRefPose(const FHitResult& HitResult, FVector& Point)
{
    if (UPrimitiveComponent* HitPrimComp = HitResult.Component.Get())
    {
        FVector HitPoint = HitResult.ImpactPoint;
        if (USkeletalMeshComponent* SkellComp = Cast<USkeletalMeshComponent>(HitPrimComp))
        {
            USkeletalMesh* SkeletalMesh = SkellComp->GetSkeletalMeshAsset();
            if (!SkeletalMesh) return false;

            const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();
            int32 BoneIndex = RefSkeleton.FindBoneIndex(HitResult.BoneName);
            
            if (BoneIndex != INDEX_NONE)
            {
                FTransform BoneTransform = SkellComp->GetSocketTransform(HitResult.BoneName);
                HitPoint = BoneTransform.InverseTransformPosition(HitPoint);
                Point = GetSkeletalMeshRefPose(SkeletalMesh, BoneIndex).TransformPosition(HitPoint);
                return true;
            }
        }
    }
    return false;
}

FTransform UUVBlueprintLibrary::GetSkeletalMeshRefPose(USkeletalMesh* SkeletalMesh, int32 BoneIndex)
{
    const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();
    FTransform MeshBonePoseTransform = RefSkeleton.GetRefBonePose()[BoneIndex];
    
    int32 ParentBoneIndex = RefSkeleton.GetParentIndex(BoneIndex);
    
    return (ParentBoneIndex == INDEX_NONE) 
        ? MeshBonePoseTransform 
        : MeshBonePoseTransform * GetSkeletalMeshRefPose(SkeletalMesh, ParentBoneIndex);
}