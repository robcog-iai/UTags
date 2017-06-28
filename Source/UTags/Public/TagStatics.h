// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once
#include "CoreMinimal.h"
#include "EngineUtils.h"

/**
* Helper functions for manipulating tags with key value pairs
* Correctly written tag example: "TagType:Key1,Value1;Key2,Value2;Key3,Value3;"
*/
struct FTagStatics
{
	///////////////////////////////////////////////////////////////////////////
	// Return the index where the tag type was found in the array
	static int32 GetTagTypeIndex(const TArray<FName>& InTags, const FString& TagType)
	{
		// Init tag index
		int32 TagIndex = INDEX_NONE;

		// Iterate all the tags, check for keyword TagType
		for (const auto& TagItr : InTags)
		{
			// Increment the position of the tag in the array
			++TagIndex;

			// Check if tag is of given type
			if (TagItr.ToString().StartsWith(TagType))
			{
				return TagIndex;
			}
		}
		// return INDEX_NONE if type was not found 
		return INDEX_NONE;
	}

	// Return the index where the tag type was found in the array
	static int32 GetTagTypeIndex(AActor* Actor, const FString& TagType)
	{
		return GetTagTypeIndex(Actor->Tags, TagType);
	}


	///////////////////////////////////////////////////////////////////////////
	// Get tag key value pairs from tag array
	static TMap<FString, FString> GetKeyValuePairs(const TArray<FName>& InTags, const FString& TagType)
	{
		// Map of the tag properties
		TMap<FString, FString> TagProperties;

		// Iterate all the tags, check for keyword TagType
		for (const auto& TagItr : InTags)
		{
			// Copy of the current tag as FString
			FString CurrTag = TagItr.ToString();

			// Check if tag is related to the TagType
			if (CurrTag.RemoveFromStart(TagType))
			{
				// Split on semicolon
				FString CurrPair;
				while (CurrTag.Split(TEXT(";"), &CurrPair, &CurrTag))
				{
					// Split on comma
					FString CurrKey, CurrValue;
					if (CurrPair.Split(TEXT(","), &CurrKey, &CurrValue))
					{
						if (!CurrKey.IsEmpty() && !CurrValue.IsEmpty())
						{
							TagProperties.Emplace(CurrKey, CurrValue);
						}
					}
				}
			}
		}
		return TagProperties;
	}

	// Get tag key value pairs from actor
	static TMap<FString, FString> GetKeyValuePairs(AActor* Actor, const FString& TagType)
	{
		return FTagStatics::GetKeyValuePairs(Actor->Tags, TagType);
	}

	// Get all actors to tag key value pairs from world
	static TMap<AActor*, TMap<FString, FString>> GetActorsToKeyValuePairs(UWorld* World, const FString& TagType)
	{
		// Map of actors to their tag properties
		TMap<AActor*, TMap<FString, FString>> ActorToTagProperties;
		// Iterate all actors
		for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
		{			
			const TMap<FString, FString> TagProperties = FTagStatics::GetKeyValuePairs(ActorItr->Tags, TagType);
			// If actor has tag type and at least one property
			if (TagProperties.Num() > 0)
			{
				ActorToTagProperties.Emplace(*ActorItr, TagProperties);
			}
		}
		return ActorToTagProperties;
	}


	///////////////////////////////////////////////////////////////////////////
	// Check if key exists in tag
	static bool HasKey(const FName& InTag, const FString& TagKey)
	{
		// Check if key exist in tag
		if (InTag.ToString().Find(TagKey) != INDEX_NONE)
		{
			return true;
		}
		// Key was not found, return false
		return false;
	}

	// Check if key exists tag array
	static bool HasKey(const TArray<FName>& InTags, const FString& TagType, const FString& TagKey)
	{
		// Check if type exists and return index of its location in the array
		int32 TagIndex = FTagStatics::GetTagTypeIndex(InTags, TagType);
		if (TagIndex != INDEX_NONE)
		{
			return FTagStatics::HasKey(InTags[TagIndex], TagKey);
		}
		// Type was not found, return false
		return false;
	}

	// Check if key exists from actor
	static bool HasKey(AActor* Actor, const FString& TagType, const FString& TagKey)
	{
		return FTagStatics::HasKey(Actor->Tags, TagType, TagKey);
	}


	///////////////////////////////////////////////////////////////////////////
	// Get tag key value from tag
	static FString GetKeyValue(const FName& InTag, const FString& TagKey)
	{
		// Copy of the current tag as FString
		FString CurrTag = InTag.ToString();

		// Check the position of the key string in the tag
		int32 KeyPos = CurrTag.Find(TagKey);
		if (KeyPos != INDEX_NONE)
		{
			// Remove from tag with the cut length of: pos of the key + length of the key + length of the comma char 
			CurrTag.RemoveAt(0, KeyPos + TagKey.Len() + 1);
			// Set the tag value as the left side of the string before the semicolon
			return CurrTag.Left(CurrTag.Find(";"));
		}

		// Return empty string if key was not found
		return FString();
	}

	// Get tag key value from tag array
	static FString GetKeyValue(const TArray<FName>& InTags, const FString& TagType, const FString& TagKey)
	{
		// Check if type exists and return index of its location in the array
		int32 TagIndex = FTagStatics::GetTagTypeIndex(InTags, TagType);
		if (TagIndex != INDEX_NONE)
		{
			return FTagStatics::GetKeyValue(InTags[TagIndex], TagKey);
		}

		// If type was not found return an empty string
		return FString();
	}

	// Get tag key value from actor
	static FString GetKeyValue(AActor* Actor, const FString& TagType, const FString& TagKey)
	{
		return FTagStatics::GetKeyValue(Actor->Tags, TagType, TagKey);
	}


	///////////////////////////////////////////////////////////////////////////
	// Add tag key value from tags, if bReplaceExisting is true, replace existing value
	static bool AddKeyValuePair(
		FName& InTag,
		const FString& TagKey,
		const FString& TagValue,
		bool bReplaceExisting = true)
	{
		// Get the key value
		FString CurrVal = GetKeyValue(InTag, TagKey);
		if (CurrVal.IsEmpty())
		{
			// Key does not exist, add new one
			InTag = FName(*InTag.ToString().Append(TagKey).Append(",").Append(TagValue).Append(";"));
			return true;
		}
		else if (bReplaceExisting)
		{
			// Key exist, replace
			InTag = FName(*InTag.ToString().Replace(*CurrVal, *TagValue));
			return true;
		}
		return false;
	}

	// Add tag key value from tags, if bReplaceExisting is true, replace existing value
	static bool AddKeyValuePair(
		TArray<FName>& InTags,
		const FString& TagType,
		const FString& TagKey,
		const FString& TagValue,
		bool bReplaceExisting = true)
	{
		// Check if type exists and return index of its location in the array
		int32 TagIndex = FTagStatics::GetTagTypeIndex(InTags, TagType);
		if (TagIndex != INDEX_NONE)
		{
			return FTagStatics::AddKeyValuePair(InTags[TagIndex], TagKey, TagValue, bReplaceExisting);
		}

		// If type was not found return false
		return false;
	}

	// Add tag key value from actor, if bReplaceExisting is true, replace existing value
	static bool AddKeyValuePair(
		AActor* Actor,
		const FString& TagType,
		const FString& TagKey,
		const FString& TagValue,
		bool bReplaceExisting = true)
	{
		return FTagStatics::AddKeyValuePair(Actor->Tags, TagType, TagKey, TagValue, bReplaceExisting);
	}
};