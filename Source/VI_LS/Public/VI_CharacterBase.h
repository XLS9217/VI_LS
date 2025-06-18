#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VI_CharacterBase.generated.h"



// Structure to store action info
USTRUCT(BlueprintType)
struct FCharacterActionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FString ActionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	int32 BlendPoseID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	bool IsBodyLanguage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FString Description;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTalkingPoseUpdated, int32, TalkingPoseID);//send -1 for not talking

UCLASS()
class VI_LS_API AVI_CharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AVI_CharacterBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	// List of actions available to this character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Actions")
	TArray<FCharacterActionData> CharacterActions;


	UPROPERTY(BlueprintAssignable, Category = "Character|Actions")
	FOnTalkingPoseUpdated OnTalkingPoseUpdated;
	
	
	// Function to generate JSON from the CharacterActions array
	UFUNCTION(BlueprintCallable, Category = "Character|Actions")
	FString GenerateActionsJSON(const FString& ActionType);

	UFUNCTION(BlueprintCallable, Category = "Character|Actions")
	int32 GetBlendPoseIDByActionName(const FString& ActionName) const;

	UFUNCTION(BlueprintCallable, Category = "Character|Actions")
	void BroadcastPoseUpdate(const FString& ActionName) const;
};
