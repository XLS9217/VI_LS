#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "VoiceInputManager.generated.h"

UCLASS()
class VI_LS_API UVoiceInputManager : public UObject
{
	GENERATED_BODY()

public:
	UVoiceInputManager();

	// Start recording
	UFUNCTION(BlueprintCallable, Category = "VoiceInput")
	void StartRecord();

	// Stop recording
	UFUNCTION(BlueprintCallable, Category = "VoiceInput")
	void StopRecord();

	// Send recorded audio to ASR server
	UFUNCTION(BlueprintCallable, Category = "VoiceInput")
	void SendToASR(const FString& Url);

private:
	TSharedPtr<class IVoiceCapture> VoiceCapture;
	TArray<uint8> CapturedVoiceData;
	bool bIsRecording;
};
