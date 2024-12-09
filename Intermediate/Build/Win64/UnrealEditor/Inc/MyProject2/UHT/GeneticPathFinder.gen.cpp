// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "MyProject2/GeneticPathFinder.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeGeneticPathFinder() {}

// Begin Cross Module References
ENGINE_API UClass* Z_Construct_UClass_AActor();
MYPROJECT2_API UClass* Z_Construct_UClass_AGeneticPathFinder();
MYPROJECT2_API UClass* Z_Construct_UClass_AGeneticPathFinder_NoRegister();
UPackage* Z_Construct_UPackage__Script_MyProject2();
// End Cross Module References

// Begin Class AGeneticPathFinder
void AGeneticPathFinder::StaticRegisterNativesAGeneticPathFinder()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(AGeneticPathFinder);
UClass* Z_Construct_UClass_AGeneticPathFinder_NoRegister()
{
	return AGeneticPathFinder::StaticClass();
}
struct Z_Construct_UClass_AGeneticPathFinder_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "IncludePath", "GeneticPathFinder.h" },
		{ "ModuleRelativePath", "GeneticPathFinder.h" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AGeneticPathFinder>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_AGeneticPathFinder_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AActor,
	(UObject* (*)())Z_Construct_UPackage__Script_MyProject2,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AGeneticPathFinder_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AGeneticPathFinder_Statics::ClassParams = {
	&AGeneticPathFinder::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	0,
	0,
	0x009000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AGeneticPathFinder_Statics::Class_MetaDataParams), Z_Construct_UClass_AGeneticPathFinder_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_AGeneticPathFinder()
{
	if (!Z_Registration_Info_UClass_AGeneticPathFinder.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AGeneticPathFinder.OuterSingleton, Z_Construct_UClass_AGeneticPathFinder_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AGeneticPathFinder.OuterSingleton;
}
template<> MYPROJECT2_API UClass* StaticClass<AGeneticPathFinder>()
{
	return AGeneticPathFinder::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(AGeneticPathFinder);
AGeneticPathFinder::~AGeneticPathFinder() {}
// End Class AGeneticPathFinder

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_anast_Documents_Unreal_Projects_MyProject2_Source_MyProject2_GeneticPathFinder_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AGeneticPathFinder, AGeneticPathFinder::StaticClass, TEXT("AGeneticPathFinder"), &Z_Registration_Info_UClass_AGeneticPathFinder, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AGeneticPathFinder), 1321616113U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_anast_Documents_Unreal_Projects_MyProject2_Source_MyProject2_GeneticPathFinder_h_1072244493(TEXT("/Script/MyProject2"),
	Z_CompiledInDeferFile_FID_Users_anast_Documents_Unreal_Projects_MyProject2_Source_MyProject2_GeneticPathFinder_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_anast_Documents_Unreal_Projects_MyProject2_Source_MyProject2_GeneticPathFinder_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
