// VI_LSGameMode.h
#pragma once
/*
 * THE AUDIO INPUT IS 
 * ONLY FOR TESTING DO NOT USE IN PRODUCTION
 *
 * 
 */
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AudioCaptureComponent.h"
#include "Sound/SoundSubmix.h"
#include "VI_LSGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnASRResponse, FString, ASR_Response);


UCLASS(minimalapi)
class AVI_LSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AVI_LSGameMode();

protected:
	virtual void BeginPlay() override;

public:
	// Start microphone capture and recording
	UFUNCTION(BlueprintCallable, Category = "Audio|Capture")
	void StartMicCapture();

	// Stop microphone capture and recording
	UFUNCTION(BlueprintCallable, Category = "Audio|Capture")
	void StopMicCapture();

	// Send the internally saved wav file to given server URL
	UFUNCTION(BlueprintCallable, Category = "Audio|Network")
	void SendInternalAudioToServer(const FString& ServerUrl);

	UPROPERTY(BlueprintAssignable)
	FOnASRResponse OnASRResponse;
private:
	UPROPERTY()
	UAudioCaptureComponent* AudioCaptureComponent;

	UPROPERTY(EditAnywhere, Category = "Audio Capture")
	USoundSubmix* CaptureSubmix;

	// Fixed internal wav file save path
	FString GetInternalAudioSavePath();
};
