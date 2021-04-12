#include "StdAfx.h"
#include "AdvancedXrayGameConstants.h"
#include "GamePersistent.h"

bool	m_bDistantSoundsEnabled = true;
bool	m_bKnifeSlotEnabled = false;
bool	m_bBinocularSlotEnabled = false;
bool	m_bTorchSlotEnabled = false;
bool	m_bBackpackSlotEnabled = false;
bool	m_bDosimeterSlotEnabled = false;
bool	m_bPantsSlotEnabled = false;
bool	m_bPdaSlotEnabled = false;
bool	m_bTorchHasBattery = false;
float	m_fDistantSndDistance = 150.f;
float	m_fDistantSndDistanceFar = 250.f;

namespace GameConstants
{
	void LoadConstants()
	{
		m_bDistantSoundsEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "distant_sounds_enabled", true);
		m_bTorchHasBattery = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "torch_has_battery", false);
		m_bKnifeSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_knife_slot", false);
		m_bBinocularSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_binocular_slot", false);
		m_bTorchSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_torch_slot", false);
		m_bBackpackSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_backpack_slot", false);
		m_bDosimeterSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_dosimeter_slot", false);
		m_bPantsSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_pants_slot", false);
		m_bPdaSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_pda_slot", false);
		m_fDistantSndDistance = READ_IF_EXISTS(pAdvancedSettings, r_float, "gameplay", "distant_snd_distance", 150.f);
		m_fDistantSndDistanceFar = READ_IF_EXISTS(pAdvancedSettings, r_float, "gameplay", "distant_snd_distance_far", 250.f);

		Msg("# Advanced X-Ray GameConstants are loaded");
	}
	
	bool GetTorchHasBattery()
	{
		return m_bTorchHasBattery;
	}

	bool GetDistantSoundsEnabled()
	{
		return m_bDistantSoundsEnabled;
	}

	bool GetKnifeSlotEnabled()
	{
		return m_bKnifeSlotEnabled;
	}

	bool GetBinocularSlotEnabled()
	{
		return m_bBinocularSlotEnabled;
	}

	bool GetTorchSlotEnabled()
	{
		return m_bTorchSlotEnabled;
	}

	bool GetBackpackSlotEnabled()
	{
		return m_bBackpackSlotEnabled;
	}

	bool GetDosimeterSlotEnabled()
	{
		return m_bDosimeterSlotEnabled;
	}

	bool GetPantsSlotEnabled()
	{
		return m_bPantsSlotEnabled;
	}

	bool GetPdaSlotEnabled()
	{
		return m_bPdaSlotEnabled;
	}

	float GetDistantSndDistance()
	{
		return m_fDistantSndDistance;
	}

	float GetDistantSndDistanceFar()
	{
		return m_fDistantSndDistanceFar;
	}
}
