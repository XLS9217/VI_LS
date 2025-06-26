#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Util/StructForger.h"
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


	// Native events for handling control messages
	UFUNCTION()
	void HandleControlMessage(const FControlMessageBase& Message);

	UFUNCTION()
	void HandleWebsocketConnect(const bool bSuccess);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Control")
	void OnControlSpeak(const FSpeakPayload& Payload);

	UFUNCTION(BlueprintImplementableEvent, Category = "Control")
	void OnControlThink(const FThinkingPayload& Payload);

	
	// List of actions available to this character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Actions")
	TArray<FCharacterActionData> CharacterActions;

	UPROPERTY(BlueprintAssignable, Category = "Character|Actions")
	FOnTalkingPoseUpdated OnTalkingPoseUpdated;
	
	
	// Function to generate JSON from the CharacterActions array, action is set or add, refer to backend's policy
	UFUNCTION(BlueprintCallable, Category = "Character|Actions")
	FString GenerateActionsJSON(const FString& ActionType);

	UFUNCTION(BlueprintCallable, Category = "Character|Actions")
	int32 GetBlendPoseIDByActionName(const FString& ActionName) const;

	UFUNCTION(BlueprintCallable, Category = "Character|Actions")
	void BroadcastPoseUpdate(const FString& ActionName) const;
};
