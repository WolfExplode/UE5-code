#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UVBlueprintLibrary.generated.h" // Auto-generated header

UCLASS()
class GAMEANIMATIONSAMPLE_API UUVBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UV Utilities", meta = (WorldContext = "WorldContextObject"))
	    static bool GetUVFromSkeletalMeshHit(
        UObject* WorldContextObject,
        const FHitResult& Hit,
        FVector2D& UV,
        int32 UVChannel = 0  // Added UV channel parameter
    );
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skeletal Mesh Utilities")
	static bool GetHitResultPointInRefPose(const FHitResult& HitResult, FVector& Point);

private:
	static FTransform GetSkeletalMeshRefPose(USkeletalMesh* SkeletalMesh, int32 BoneIndex);
};

