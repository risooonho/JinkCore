// Copyright 2015-2016 Piperift. All Rights Reserved.

#include "JinkCorePrivatePCH.h"
#include "VideoSettingsLib.h"


// Get a list of screen resolutions supported by video adapter
// NOTE: This function needs "RHI" to be added to <Project>.Build.cs file!
bool UVideoSettingsLib::GetSupportedScreenResolutions(TArray<FString>& Resolutions)
{
	FScreenResolutionArray ResolutionsArray;

	if (RHIGetAvailableResolutions(ResolutionsArray, true))  // needs the "RHI" dependency
	{
		for (const FScreenResolutionRHI& Resolution : ResolutionsArray)
		{
			if (Resolution.Width < MIN_SCREEN_WIDTH || Resolution.Height < MIN_SCREEN_HEIGHT)
			{
				continue;
			}

			FString Str = FString::FromInt(Resolution.Width) + "x" + FString::FromInt(Resolution.Height);
			Resolutions.AddUnique(Str);
		}

		return true;
	}

	return false;  // failed to obtain screen resolutions
}


// Get currently set screen resolution
FString UVideoSettingsLib::GetScreenResolution()
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return FString("");
	}

	FIntPoint Resolution = Settings->GetScreenResolution();
	return FString::FromInt(Resolution.X) + "x" + FString::FromInt(Resolution.Y);
}


// Check whether or not we are currently running in fullscreen mode
bool UVideoSettingsLib::IsInFullscreen()
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	return Settings->GetFullscreenMode() == EWindowMode::Fullscreen;
}


// Set the desired screen resolution (does not change it yet)
bool UVideoSettingsLib::SetScreenResolution(const int32 Width, const int32 Height, const bool Fullscreen)
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->SetScreenResolution(FIntPoint(Width, Height));
	Settings->SetFullscreenMode(Fullscreen ? EWindowMode::Fullscreen : EWindowMode::Windowed);
	return true;
}


// Change the current screen resolution
bool UVideoSettingsLib::ChangeScreenResolution(const int32 Width, const int32 Height, const bool Fullscreen)
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	EWindowMode::Type WindowMode = Fullscreen ? EWindowMode::Fullscreen : EWindowMode::Windowed;
	Settings->RequestResolutionChange(Width, Height, WindowMode, false);
	return true;
}


// Get the current video quality settings
bool UVideoSettingsLib::GetVideoQualitySettings(EAntiAliasing& AntiAliasing, EQuality& Effects, EQuality& PostProcess,
	int32& Resolution, EQuality& Shadow, EQuality& Texture, int32& ViewDistance)
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	AntiAliasing = static_cast<EAntiAliasing>(Settings->ScalabilityQuality.AntiAliasingQuality);
	Effects = static_cast<EQuality>(Settings->ScalabilityQuality.EffectsQuality);
	PostProcess = static_cast<EQuality>(Settings->ScalabilityQuality.PostProcessQuality);
	Resolution = Settings->ScalabilityQuality.ResolutionQuality;
	Shadow = static_cast<EQuality>(Settings->ScalabilityQuality.ShadowQuality);
	Texture = static_cast<EQuality>(Settings->ScalabilityQuality.TextureQuality);
	ViewDistance = Settings->ScalabilityQuality.ViewDistanceQuality;
	return true;
}


// Set the quality settings (not applied nor saved yet)
bool UVideoSettingsLib::SetVideoQualitySettings(const EAntiAliasing AntiAliasing, const EQuality Effects, const EQuality PostProcess,
	const int32 Resolution, const EQuality Shadow, const EQuality Texture, const int32 ViewDistance)
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->ScalabilityQuality.AntiAliasingQuality = (int32)AntiAliasing;
	Settings->ScalabilityQuality.EffectsQuality = (int32)Effects;
	Settings->ScalabilityQuality.PostProcessQuality = (int32)PostProcess;
	Settings->ScalabilityQuality.ResolutionQuality = Resolution;
	Settings->ScalabilityQuality.ShadowQuality = (int32)Shadow;
	Settings->ScalabilityQuality.TextureQuality = (int32)Texture;
	Settings->ScalabilityQuality.ViewDistanceQuality = ViewDistance;
	return true;
}


// Check whether or not we have vertical sync enabled
bool UVideoSettingsLib::IsVSyncEnabled()
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	return Settings->IsVSyncEnabled();
}


// Set the vertical sync flag
bool UVideoSettingsLib::SetVSyncEnabled(const bool VSync)
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->SetVSyncEnabled(VSync);
	return true;
}


// Confirm and save current video mode (resolution and fullscreen/windowed)
bool UVideoSettingsLib::SaveVideoModeAndQuality()
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->ConfirmVideoMode();
	Settings->ApplyNonResolutionSettings();
	Settings->SaveSettings();
	return true;
}


// Revert to original video settings
bool UVideoSettingsLib::RevertVideoMode()
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->RevertVideoMode();
	return true;
}


//---- PRIVATE METHODS -------------------------------------------------------------------------------

// Try to get the GameUserSettings object from the engine
UGameUserSettings* UVideoSettingsLib::GetGameUserSettings()
{
	if (GEngine != nullptr)
	{
		return GEngine->GameUserSettings;
	}

	return nullptr;
}
