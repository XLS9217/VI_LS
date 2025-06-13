// Fill out your copyright notice in the Description page of Project Settings.


#include "Util/StructForger.h"

// #include "Dom/JsonObject.h"
// #include "Serialization/JsonReader.h"
// #include "Serialization/JsonSerializer.h"

FControlMessageBase UStructForger::Forge(const FString& ControlMessage)
{
	FControlMessageBase Result;

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(ControlMessage);
	if (!(FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid()))
	{
		return Result;
	}

	if (JsonObject->HasField(TEXT("type")))
	{
		const FString TypeValue = JsonObject->GetStringField(TEXT("type"));
		if (TypeValue.Equals(TEXT("control"), ESearchCase::IgnoreCase) && JsonObject->HasField(TEXT("payload")))
		{
			TSharedPtr<FJsonObject> PayloadObj = JsonObject->GetObjectField(TEXT("payload"));
			FSpeakPayload Payload;

			if (PayloadObj->HasField(TEXT("action")))
			{
				Payload.Action = PayloadObj->GetStringField(TEXT("action"));
			}
			if (PayloadObj->HasField(TEXT("content")))
			{
				Payload.Content = PayloadObj->GetStringField(TEXT("content"));
			}

			Result.Type = TypeValue;
			Result.SpeakPayload = Payload;
		}
	}
	return Result;
}

