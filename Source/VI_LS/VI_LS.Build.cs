// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VI_LS : ModuleRules
{
	public VI_LS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"EnhancedInput",
			"WebSockets",
			"HTTP",
			"Json",
			"JsonUtilities",
			"SoundUtilities",
			"AudioMixer",
			"Voice",
			"AudioCapture"
		});
	}
}
