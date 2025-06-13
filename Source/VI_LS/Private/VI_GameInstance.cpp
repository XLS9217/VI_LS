// Fill out your copyright notice in the Description page of Project Settings.

#include "WebSocketsModule.h"
#include <HttpModule.h>
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"
#include "VI_GameInstance.h"

#include "Sound/SoundWaveProcedural.h"


void UVI_GameInstance::Init()
{
	Super::Init();

	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
	}

	SoundWaveProcedural = NewObject<USoundWaveProcedural>();
	SoundWaveProcedural->SetSampleRate(16000); 
	SoundWaveProcedural->NumChannels = 1;
	SoundWaveProcedural->Duration = INDEFINITELY_LOOPING_DURATION;
	SoundWaveProcedural->bLooping = false;
	SoundWaveProcedural->SoundGroup = SOUNDGROUP_Voice;
}

void UVI_GameInstance::Shutdown()
{
	if (SocketHandle.IsValid() && SocketHandle->IsConnected())
	{
		SocketHandle->Close();
	}
	SocketHandle = nullptr;

	Super::Shutdown();
}

void UVI_GameInstance::ConnectToWebSocket(const FString& URL)
{
	if (SocketHandle.IsValid() && SocketHandle->IsConnected())
	{
		SocketHandle->Close();
	}

	SocketHandle = FWebSocketsModule::Get().CreateWebSocket(URL);

	SocketHandle->OnConnected().AddLambda([this, URL]()
	{
		UE_LOG(LogTemp, Log, TEXT("WebSocket connected to [%s]"), *URL);

		const FString RoleMessage = TEXT("{\"role\":\"displayer\"}");
		SocketHandle->Send(RoleMessage);

		UE_LOG(LogTemp, Log, TEXT("Sent role message: %s"), *RoleMessage);
	});

	SocketHandle->OnConnectionError().AddLambda([URL](const FString& Error)
	{
		UE_LOG(LogTemp, Error, TEXT("WebSocket connection error [%s]: %s"), *URL, *Error);
		
	});

	SocketHandle->OnClosed().AddLambda([URL](int32 StatusCode, const FString& Reason, bool bWasClean)
	{
		UE_LOG(LogTemp, Warning, TEXT("WebSocket [%s] closed. Reason: %s"), *URL, *Reason);
	});

	SocketHandle->OnMessage().AddLambda([this, URL](const FString& Message)
	{
		UE_LOG(LogTemp, Log, TEXT("WebSocket [%s] message received: %s"), *URL, *Message);

		FControlMessageBase ControlMessage = UStructForger::Forge(Message);
		OnControlMessage.Broadcast(ControlMessage);
	});

	SocketHandle->Connect();
}

void UVI_GameInstance::SendMessageViaSocket(const FString& Message)
{
	if (SocketHandle.IsValid() && SocketHandle->IsConnected())
	{
		SocketHandle->Send(Message);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot send message: WebSocket is not connected."));
	}
}

void UVI_GameInstance::RequestForTTS(const FString& SpeechText, const FString& URL, const FString& ResultID)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	Request->SetURL(URL);
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");

	FString Payload = FString::Printf(TEXT("{\"text\": \"%s\"}"), *SpeechText);
	Request->SetContentAsString(Payload);
	UE_LOG(LogTemp, Log, TEXT("Sending TTS request"));
	
	SoundWaveProcedural->ResetAudio();
	
	Request->OnProcessRequestComplete().BindLambda([this, ResultID](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
	{
		if (!bSuccess || !Resp.IsValid() || Resp->GetResponseCode() != 200)
		{
			UE_LOG(LogTemp, Error, TEXT("TTS request failed"));
			return;
		}

		//const TArray<uint8>& AudioData = Resp->GetContent();

		const TArray<uint8>& RespData = Resp->GetContent();
		TArray<uint8> AudioData = RespData;
		
		constexpr int32 ZeroSamplesCount = 256 * sizeof(int16);
		int32 ZeroCount = FMath::Min(ZeroSamplesCount, AudioData.Num());
		FMemory::Memzero(AudioData.GetData(), ZeroCount);
		
		SoundWaveProcedural->QueueAudio(AudioData.GetData(), AudioData.Num());

		FTTSFeedback Feedback;
		Feedback.SoundWave = SoundWaveProcedural;
		Feedback.ResultID = ResultID;

		OnTTSFeedback.Broadcast(Feedback);

		UE_LOG(LogTemp, Log, TEXT("TTS Feedback broadcasted, ResultID: %s"), *ResultID);
	});

	Request->ProcessRequest();
}
