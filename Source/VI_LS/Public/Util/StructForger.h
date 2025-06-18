#pragma once

#include "CoreMinimal.h"
#include "StructForger.generated.h"

class USoundWaveProcedural;
//-----------------------------------------Sound related
USTRUCT(BlueprintType)
struct FTTSFeedback
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	USoundWaveProcedural* SoundWave;

	UPROPERTY(BlueprintReadWrite)
	FString ResultID;

	UPROPERTY(BlueprintReadWrite)
	float SoundLength;  
};


//-----------------------------------------Socket Control related
USTRUCT(BlueprintType)
struct FSpeakPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Content;
	
	UPROPERTY(BlueprintReadWrite)
	FString BodyLanguage;
};

USTRUCT(BlueprintType)
struct FThinkingPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	bool bIsThinking;
};



USTRUCT(BlueprintType)
struct FControlMessageBase
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FString Type;

	//The rest of the body, use based on action
	
	UPROPERTY(BlueprintReadWrite)
	FString Action;
	
	UPROPERTY(BlueprintReadWrite)
	FSpeakPayload SpeakPayload;

	UPROPERTY(BlueprintReadWrite)
	FThinkingPayload ThinkingPayload;
};





UCLASS()
class VI_LS_API UStructForger : public UObject
{
	GENERATED_BODY()

public:

	/*
	 * ControlMessage -> a json string
	 */
	UFUNCTION(BlueprintCallable, Category = "Forge")
	static FControlMessageBase Forge(const FString& ControlMessage);
};
