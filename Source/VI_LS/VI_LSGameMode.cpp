// VI_LSGameMode.cpp
#include "VI_LSGameMode.h"
#include "VI_LSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

AVI_LSGameMode::AVI_LSGameMode()
	: Super()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	AudioCaptureComponent = CreateDefaultSubobject<UAudioCaptureComponent>(TEXT("AudioCaptureComponent"));
	AudioCaptureComponent->bAutoActivate = false;
}

void AVI_LSGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (CaptureSubmix)
	{
		AudioCaptureComponent->SoundSubmix = CaptureSubmix;
	}
}

void AVI_LSGameMode::StartMicCapture()
{
	if (CaptureSubmix && AudioCaptureComponent)
	{
		UAudioMixerBlueprintLibrary::StartRecordingOutput(this, 0.0f, CaptureSubmix);
		AudioCaptureComponent->Start();
	}
}

void AVI_LSGameMode::StopMicCapture()
{
	if (!AudioCaptureComponent || !CaptureSubmix)
	{
		UE_LOG(LogTemp, Warning, TEXT("AudioCaptureComponent or CaptureSubmix not valid"));
		return;
	}

	AudioCaptureComponent->Stop();

	const FString Name = TEXT("MicCapture_GameMode");
	const FString SavePath = GetInternalAudioSavePath();

	UAudioMixerBlueprintLibrary::StopRecordingOutput(
		this,
		EAudioRecordingExportType::WavFile,
		Name,
		SavePath,
		CaptureSubmix,
		nullptr
	);

	UE_LOG(LogTemp, Log, TEXT("Saved mic capture to: %s"), *SavePath);
}

FString AVI_LSGameMode::GetInternalAudioSavePath()
{
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("MicCapture"));
}

void AVI_LSGameMode::SendInternalAudioToServer(const FString& ServerUrl)
{
	const FString FilePath = GetInternalAudioSavePath();
	FString AbsolutePath = FPaths::ConvertRelativePathToFull(FilePath) + TEXT("/MicCapture_GameMode.wav");


	UE_LOG(LogTemp, Log, TEXT("Attempting to load audio file at: %s"), *AbsolutePath);

	// Check file size
	int64 FileSize = IFileManager::Get().FileSize(*AbsolutePath);
	UE_LOG(LogTemp, Log, TEXT("File size: %lld bytes"), FileSize);
	if (FileSize <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("File is empty or unreadable"));
		return;
	}

	// Try to load file using low-level file handle
	IFileHandle* RawFileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenRead(*AbsolutePath);
	TUniquePtr<IFileHandle> FileHandle(RawFileHandle);
	if (!FileHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to open file handle for %s"), *AbsolutePath);
		return;
	}

	TArray<uint8> FileData;
	FileData.SetNumUninitialized(FileSize);
	if (!FileHandle->Read(FileData.GetData(), FileSize))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read file data from %s"), *AbsolutePath);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Successfully read %lld bytes from %s"), FileSize, *AbsolutePath);

	// Setup HTTP request to send multipart form-data with the audio file
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(ServerUrl);
	HttpRequest->SetVerb(TEXT("POST"));

	const FString Boundary = TEXT("----UnrealFormBoundary123456789");
	const FString ContentType = FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary);
	HttpRequest->SetHeader(TEXT("Content-Type"), ContentType);

	const FString BodyStart = FString::Printf(
		TEXT("--%s\r\nContent-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\nContent-Type: audio/wav\r\n\r\n"),
		*Boundary,
		*FPaths::GetCleanFilename(FilePath)
	);

	const FString BodyEnd = FString::Printf(TEXT("\r\n--%s--\r\n"), *Boundary);

	// Convert header and footer strings to UTF8 bytes
	TArray<uint8> BodyStartBytes;
	FTCHARToUTF8 StartConv(*BodyStart);
	BodyStartBytes.Append((uint8*)StartConv.Get(), StartConv.Length());

	TArray<uint8> BodyEndBytes;
	FTCHARToUTF8 EndConv(*BodyEnd);
	BodyEndBytes.Append((uint8*)EndConv.Get(), EndConv.Length());

	// Construct full request body
	TArray<uint8> RequestBody;
	RequestBody.Append(BodyStartBytes);
	RequestBody.Append(FileData);
	RequestBody.Append(BodyEndBytes);

	HttpRequest->SetContent(RequestBody);

	// Bind lambda to log server response or failure
	HttpRequest->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if (bWasSuccessful && Response.IsValid())
		{
			FString ResponseStr = Response->GetContentAsString();
			UE_LOG(LogTemp, Log, TEXT("Raw server response: %s"), *ResponseStr);

			// Parse JSON
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

			if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
			{
				FString AsrResult;
				if (JsonObject->TryGetStringField(TEXT("asr_result"), AsrResult))
				{
					// Forge new JSON object
					TSharedPtr<FJsonObject> NewPayload = MakeShared<FJsonObject>();
					NewPayload->SetBoolField(TEXT("recognized"), true);
					NewPayload->SetStringField(TEXT("content"), AsrResult);

					TSharedPtr<FJsonObject> NewRoot = MakeShared<FJsonObject>();
					NewRoot->SetStringField(TEXT("type"), TEXT("user_chat"));
					NewRoot->SetObjectField(TEXT("payload"), NewPayload);

					// Serialize forged JSON to string
					FString NewJsonStr;
					TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&NewJsonStr);
					FJsonSerializer::Serialize(NewRoot.ToSharedRef(), Writer);

					UE_LOG(LogTemp, Log, TEXT("Forged JSON: %s"), *NewJsonStr);

					OnASRResponse.Broadcast(NewJsonStr);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Field 'asr_result' not found in response JSON"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to parse server response as JSON"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("HTTP request failed"));
		}
	});

	HttpRequest->ProcessRequest();
}
