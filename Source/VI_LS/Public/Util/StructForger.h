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
};

//-----------------------------------------Socket Control related
USTRUCT(BlueprintType)
struct FSpeakPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Action;

	UPROPERTY(BlueprintReadWrite)
	FString Content;
	
	UPROPERTY(BlueprintReadWrite)
	FString BodyLanguage;
};



USTRUCT(BlueprintType)
struct FControlMessageBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Type;

	UPROPERTY(BlueprintReadWrite)
	FSpeakPayload SpeakPayload;
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
