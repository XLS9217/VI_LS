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

		// Check for "control" type and "payload"
		if (TypeValue.Equals(TEXT("control"), ESearchCase::IgnoreCase) && JsonObject->HasField(TEXT("payload")))
		{
			TSharedPtr<FJsonObject> PayloadObj = JsonObject->GetObjectField(TEXT("payload"));
			FSpeakPayload SpeakPayload;
			FThinkingPayload ThinkingPayload;	

			if (PayloadObj->HasField(TEXT("action")))
			{
				const FString ActionValue = PayloadObj->GetStringField(TEXT("action"));
				Result.Action = ActionValue;

				/*
				 * If this is a speak action should send to tts for further processing
				 */
				if (ActionValue.Equals(TEXT("speak"), ESearchCase::IgnoreCase))
				{
					
					if (PayloadObj->HasField(TEXT("content")))
					{
						SpeakPayload.Content = PayloadObj->GetStringField(TEXT("content"));
					}
					if (PayloadObj->HasField(TEXT("body_language")))
					{
						SpeakPayload.BodyLanguage = PayloadObj->GetStringField(TEXT("body_language"));
					}
				}
				/*
				 * Show something for thinking
				 */
				else if (ActionValue.Equals(TEXT("thinking"), ESearchCase::IgnoreCase))
				{
					if (PayloadObj->HasField(TEXT("content")))
					{
						ThinkingPayload.bIsThinking = PayloadObj->GetBoolField(TEXT("content"));

						// ✅ Add log here to output the boolean value
						UE_LOG(LogTemp, Log, TEXT("ThinkingPayload.bIsThinking: %s"), ThinkingPayload.bIsThinking ? TEXT("true") : TEXT("false"));
					}
				}
			}
			

			Result.Type = TypeValue;
			Result.SpeakPayload = SpeakPayload;
			Result.ThinkingPayload = ThinkingPayload;
		}
	}
	return Result;
}

