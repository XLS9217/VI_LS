// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include <IWebSocket.h>

#include "Util/StructForger.h"

#include "VI_GameInstance.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnControlMessage, const FControlMessageBase&, ControlMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTTSFeedback, const FTTSFeedback&, TTSFeedback);

/**
 * 
 */
UCLASS()
class VI_LS_API UVI_GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	virtual void Shutdown() override;
	
	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void ConnectToWebSocket(const FString& URL);
	
	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void SendMessageViaSocket(const FString& Message);
	
	UFUNCTION(BlueprintCallable, Category = "WebSocket") 
	void RequestForTTS(const FString& SpeechText , const FString& URL, const FString& ResultID);

	
	UPROPERTY(BlueprintAssignable, Category = "WebSocket") 
	FOnControlMessage OnControlMessage;
	
	UPROPERTY(BlueprintAssignable, Category = "WebSocket") 
	FOnTTSFeedback OnTTSFeedback;

	UPROPERTY(BlueprintReadOnly)
	USoundWaveProcedural* SoundWaveProcedural;
	
private:
	TSharedPtr<IWebSocket> SocketHandle;
};
