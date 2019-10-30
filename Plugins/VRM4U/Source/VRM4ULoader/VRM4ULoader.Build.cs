
using UnrealBuildTool;
using System.IO;


public class VRM4ULoader : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
    }

    public VRM4ULoader(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                Path.Combine(ThirdPartyPath, "assimp/include")
                // ... add public include paths required here ...
            }
        );

        PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Slate",
				"SlateCore",
				"Core",
				"CoreUObject",
                "Engine",
                "RHI",
                "RenderCore",
                "AnimGraphRuntime",
                "ProceduralMeshComponent",
                "VRM4U",
            });

		if (Target.bBuildEditor) {
			PrivateDependencyModuleNames.Add("Persona");
		}



		DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
            }
            );

        if (Target.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.Add("VRM4UImporter");
            PrivateIncludePaths.Add("VRM4UImporter/Private");
        }

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "x64" : "x86";
            //string BuildString = (Target.Configuration != UnrealTargetConfiguration.Debug) ? "Release" : "Debug";
            string BuildString = (Target.Configuration != UnrealTargetConfiguration.Debug) ? "Release" : "Release";
            //PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "assimp/lib", PlatformString, "assimp-vc140-mt.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "assimp/lib", PlatformString, BuildString, "assimp-vc140-mt.lib"));

			PublicDelayLoadDLLs.Add("assimp-vc140-mt.dll");
			RuntimeDependencies.Add(Path.Combine(ThirdPartyPath, "assimp/bin", PlatformString, "assimp-vc140-mt.dll"));
        }
        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            string PlatformString = "android";
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "assimp/lib", PlatformString, "libassimp.so"));

            //RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(ThirdPartyPath, "assimp/bin", PlatformString, "assimp-vc140-mt.dll")));
        }
    }
}
