// Fill out your copyright notice in the Description page of Project Settings.


#include "Util/VoiceInputManager.h"

#include "VoiceModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "Misc/Base64.h"

// WAV header struct
struct FWavHeader
{
	char RIFF[4] = { 'R','I','F','F' };
	uint32 FileSize;
	char WAVE[4] = { 'W','A','V','E' };
	char Fmt[4] = { 'f','m','t',' ' };
	uint32 FmtSize = 16;
	uint16 FormatTag = 1; // PCM
	uint16 Channels = 1;
	uint32 SamplesPerSec = 16000;
	uint32 AvgBytesPerSec;
	uint16 BlockAlign;
	uint16 BitsPerSample = 16;
	char Data[4] = { 'd','a','t','a' };
	uint32 DataSize;
};

UVoiceInputManager::UVoiceInputManager()
{

	VoiceCapture = FVoiceModule::Get().CreateVoiceCapture(TEXT("Default Device"), 16000, 1);
	if (!VoiceCapture.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create VoiceCapture device"));
	}
	
	bIsRecording = false;
}



void UVoiceInputManager::StartRecord()
{
	if (VoiceCapture.IsValid())
	{
		VoiceCapture->Start();
		bIsRecording = true;
		CapturedVoiceData.Reset();
		UE_LOG(LogTemp, Log, TEXT("Voice capture started."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("VoiceCapture not valid"));
	}
}

void UVoiceInputManager::StopRecord()
{
	if (VoiceCapture.IsValid() && bIsRecording)
	{
		VoiceCapture->Stop();
		bIsRecording = false;

		uint32 AvailableBytes = 0;
		EVoiceCaptureState::Type CaptureState = VoiceCapture->GetCaptureState(AvailableBytes);

		if (CaptureState == EVoiceCaptureState::Ok && AvailableBytes > 0)
		{
			TArray<uint8> TempBuffer;
			TempBuffer.SetNumUninitialized(AvailableBytes);
			uint32 BytesWritten = 0;
			VoiceCapture->GetVoiceData(TempBuffer.GetData(), AvailableBytes, BytesWritten);
			CapturedVoiceData = TempBuffer;
			UE_LOG(LogTemp, Log, TEXT("Voice capture stopped. Captured %d bytes."), BytesWritten);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No data captured or voice capture not OK."));
		}
	}
}

TArray<uint8> WrapPCMToWav(const TArray<uint8>& PCMData)
{
	FWavHeader Header;
	Header.DataSize = PCMData.Num();
	Header.FileSize = sizeof(FWavHeader) - 8 + Header.DataSize;
	Header.AvgBytesPerSec = Header.SamplesPerSec * Header.Channels * Header.BitsPerSample / 8;
	Header.BlockAlign = Header.Channels * Header.BitsPerSample / 8;

	TArray<uint8> WavData;
	WavData.SetNum(sizeof(FWavHeader) + PCMData.Num());
	FMemory::Memcpy(WavData.GetData(), &Header, sizeof(FWavHeader));
	FMemory::Memcpy(WavData.GetData() + sizeof(FWavHeader), PCMData.GetData(), PCMData.Num());

	// Debug save to E:/unreal_record.wav
	FFileHelper::SaveArrayToFile(WavData, TEXT("E:/unreal_record.wav"));

	return WavData;
}

void UVoiceInputManager::SendToASR(const FString& Url)
{
	if (CapturedVoiceData.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No voice data to send."));
		return;
	}

	TArray<uint8> WavData = WrapPCMToWav(CapturedVoiceData);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "multipart/form-data");
	Request->SetContent(WavData);

	Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
	{
		if (bSuccess && Resp.IsValid() && Resp->GetResponseCode() == 200)
		{
			UE_LOG(LogTemp, Log, TEXT("ASR Response: %s"), *Resp->GetContentAsString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to send ASR request."));
		}
	});

	Request->ProcessRequest();
}
