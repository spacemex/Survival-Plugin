// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SurvivalTemplate : ModuleRules
{
	public SurvivalTemplate(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(["Core", "GameplayTags"]);

		PrivateDependencyModuleNames.AddRange(
			[
				"CoreUObject", "Engine", "Slate", "SlateCore",
				"EnhancedInput", "InputCore"
			]
		);
	}
}