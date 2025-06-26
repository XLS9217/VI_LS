#include "VI_CharacterBase.h"

#include "VI_GameInstance.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

// Sets default values
AVI_CharacterBase::AVI_CharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AVI_CharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (UVI_GameInstance* GameInstance = Cast<UVI_GameInstance>(GetGameInstance()))
	{
		GameInstance->OnControlMessage.AddDynamic(this, &AVI_CharacterBase::HandleControlMessage);
		GameInstance->OnWebsocketConnect.AddDynamic(this, &AVI_CharacterBase::HandleWebsocketConnect );
	}
}

void AVI_CharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AVI_CharacterBase::HandleControlMessage(const FControlMessageBase& Message)
{
	if (Message.Type.Equals(TEXT("control") , ESearchCase::IgnoreCase) )
	{
		if (Message.Action.Equals(TEXT("speak") , ESearchCase::IgnoreCase))
		{
			const FString& Content = Message.SpeakPayload.Content;

			OnControlSpeak(Message.SpeakPayload);
			UE_LOG(LogTemp, Warning, TEXT("OnControlSpeak: %s"), *Content);
		}
		else if (Message.Action.Equals(TEXT("thinking") , ESearchCase::IgnoreCase))
		{
			OnControlThink(Message.ThinkingPayload);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Message type %s is not a control"), *Message.Type);
	}
}

void AVI_CharacterBase::HandleWebsocketConnect(const bool bSuccess)
{
	if (bSuccess)
	{
		const FString ActionJsonStr = GenerateActionsJSON(TEXT("set"));
		if (UVI_GameInstance* GameInstance = Cast<UVI_GameInstance>(GetGameInstance()))
		{
			GameInstance->SendMessageViaSocket(ActionJsonStr);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Websocket connection failed"));
	}
	
}


void AVI_CharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


FString AVI_CharacterBase::GenerateActionsJSON(const FString& ActionType)
{
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
	RootObject->SetStringField("type", "display_info");

	TSharedPtr<FJsonObject> PayloadObject = MakeShareable(new FJsonObject);
	PayloadObject->SetStringField("action", ActionType);

	TArray<TSharedPtr<FJsonValue>> ContentArray;

	for (const FCharacterActionData& Action : CharacterActions)
	{
		TSharedPtr<FJsonObject> ActionObject = MakeShareable(new FJsonObject);
		ActionObject->SetStringField("display_name", Action.ActionName);

		// Modify description based on IsBodyLanguage
		FString FinalDescription = Action.Description;
		if (Action.IsBodyLanguage)
		{
			FinalDescription += TEXT(" This is a body language that will display while the character talks.");
		}

		ActionObject->SetStringField("display_description", FinalDescription);
		ContentArray.Add(MakeShareable(new FJsonValueObject(ActionObject)));
	}

	PayloadObject->SetArrayField("content", ContentArray);
	RootObject->SetObjectField("payload", PayloadObject);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	return OutputString;
}

int32 AVI_CharacterBase::GetBlendPoseIDByActionName(const FString& ActionName) const
{
	for (const FCharacterActionData& Action : CharacterActions)
	{
		if (Action.ActionName.Equals(ActionName, ESearchCase::IgnoreCase))
		{
			return Action.BlendPoseID;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("GetBlendPoseIDByActionName: Action '%s' not found in CharacterActions."), *ActionName);
	return -1;
}

void AVI_CharacterBase::BroadcastPoseUpdate(const FString& ActionName) const
{
	int32 PoseID = GetBlendPoseIDByActionName(ActionName);
	OnTalkingPoseUpdated.Broadcast(PoseID);
}
