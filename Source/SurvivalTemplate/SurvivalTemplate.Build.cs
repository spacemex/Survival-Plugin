// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SurvivalTemplate : ModuleRules
{
	public SurvivalTemplate(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(["Core"]);

		PrivateDependencyModuleNames.AddRange(["CoreUObject", "Engine", "Slate", "SlateCore",
		"GameplayTags","EnhancedInput","InputCore"
		]);
	}
}