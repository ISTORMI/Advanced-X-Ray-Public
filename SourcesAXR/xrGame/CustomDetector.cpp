#include "stdafx.h"
#include "customdetector.h"
#include "ui/ArtefactDetectorUI.h"
#include "hudmanager.h"
#include "inventory.h"
#include "level.h"
#include "map_manager.h"
#include "ActorEffector.h"
#include "actor.h"
#include "ui/UIWindow.h"
#include "player_hud.h"
#include "weapon.h"
#include "AdvancedXrayGameConstants.h"
#include "../xrEngine/LightAnimLibrary.h"
#include "Battery.h"

ITEM_INFO::ITEM_INFO()
{
	pParticle	= NULL;
	curr_ref	= NULL;
}

ITEM_INFO::~ITEM_INFO()
{
	if(pParticle)
		CParticlesObject::Destroy(pParticle);
}

bool CCustomDetector::CheckCompatibilityInt(CHudItem* itm, u16* slot_to_activate)
{
	if(itm==NULL)
		return true;

	CInventoryItem& iitm			= itm->item();
	u32 slot						= iitm.BaseSlot();
	bool bres = (slot==INV_SLOT_2 || slot==KNIFE_SLOT || slot==BOLT_SLOT);
	if(!bres && slot_to_activate)
	{
		*slot_to_activate	= NO_ACTIVE_SLOT;
		if(m_pInventory->ItemFromSlot(BOLT_SLOT))
			*slot_to_activate = BOLT_SLOT;

		if(m_pInventory->ItemFromSlot(KNIFE_SLOT))
			*slot_to_activate = KNIFE_SLOT;

		if(m_pInventory->ItemFromSlot(INV_SLOT_3) && m_pInventory->ItemFromSlot(INV_SLOT_3)->BaseSlot()!=INV_SLOT_3)
			*slot_to_activate = INV_SLOT_3;

		if(m_pInventory->ItemFromSlot(INV_SLOT_2) && m_pInventory->ItemFromSlot(INV_SLOT_2)->BaseSlot()!=INV_SLOT_3)
			*slot_to_activate = INV_SLOT_2;

		if(*slot_to_activate != NO_ACTIVE_SLOT)
			bres = true;
	}

	if(itm->GetState()!=CHUDState::eShowing)
		bres = bres && !itm->IsPending();

	if(bres)
	{
		CWeapon* W = smart_cast<CWeapon*>(itm);
		if(W)
			bres =	bres								&& 
					(W->GetState()!=CHUDState::eBore)	&& 
					(W->GetState()!=CWeapon::eReload) && 
					(W->GetState()!=CWeapon::eSwitch) && 
					!W->IsZoomed();
	}
	return bres;
}

bool  CCustomDetector::CheckCompatibility(CHudItem* itm)
{
	if(!inherited::CheckCompatibility(itm) )	
		return false;

	if(!CheckCompatibilityInt(itm, NULL))
	{
		HideDetector	(true);
		return			false;
	}
	return true;
}

void CCustomDetector::HideDetector(bool bFastMode)
{
	if(GetState()==eIdle)
		ToggleDetector(bFastMode);
}

void CCustomDetector::ShowDetector(bool bFastMode)
{
	if(GetState()==eHidden)
		ToggleDetector(bFastMode);
}

void CCustomDetector::ToggleDetector(bool bFastMode)
{
	m_bNeedActivation		= false;
	m_bFastAnimMode			= bFastMode;

	if(GetState()==eHidden)
	{
		PIItem iitem = m_pInventory->ActiveItem();
		CHudItem* itm = (iitem)?iitem->cast_hud_item():NULL;
		u16 slot_to_activate = NO_ACTIVE_SLOT;

		if(CheckCompatibilityInt(itm, &slot_to_activate))
		{
			if(slot_to_activate!=NO_ACTIVE_SLOT)
			{
				m_pInventory->Activate(slot_to_activate);
				m_bNeedActivation		= true;
			}else
			{
				SwitchState				(eShowing);
				TurnDetectorInternal	(true);
			}
		}
	}else
	if(GetState()==eIdle)
		SwitchState					(eHiding);

}

void CCustomDetector::OnStateSwitch(u32 S)
{
	inherited::OnStateSwitch(S);

	switch(S)
	{
	case eShowing:
		{
			g_player_hud->attach_item	(this);
			m_sounds.PlaySound			("sndShow", Fvector().set(0,0,0), this, true, false);
			PlayHUDMotion				(m_bFastAnimMode ? "anm_show_fast" : "anm_show", FALSE/*TRUE*/, this, GetState());
			SetPending					(TRUE);
		}break;
	case eHiding:
		{
			m_sounds.PlaySound			("sndHide", Fvector().set(0,0,0), this, true, false);
			PlayHUDMotion				(m_bFastAnimMode ? "anm_hide_fast" : "anm_hide", FALSE/*TRUE*/, this, GetState());
			SetPending					(TRUE);
		}break;
	case eIdle:
		{
			PlayAnimIdle				();
			SetPending					(FALSE);
		}break;
}
}

void CCustomDetector::OnAnimationEnd(u32 state)
{
	inherited::OnAnimationEnd	(state);
	switch(state)
	{
	case eShowing:
		{
			SwitchState					(eIdle);
		} break;
	case eHiding:
		{
			SwitchState					(eHidden);
			TurnDetectorInternal		(false);
			g_player_hud->detach_item	(this);
		} break;
	}
}

void CCustomDetector::UpdateXForm()
{
	CInventoryItem::UpdateXForm();
}

void CCustomDetector::OnActiveItem()
{
	return;
}

void CCustomDetector::OnHiddenItem()
{
}

CCustomDetector::CCustomDetector() 
{
	m_ui				= NULL;
	m_bFastAnimMode		= false;
	m_bNeedActivation	= false;

	m_fMaxChargeLevel = 0.0f;
	m_fCurrentChargeLevel = 1.0f;
	m_fUnchargeSpeed = 0.0f;
}

CCustomDetector::~CCustomDetector() 
{
	m_artefacts.destroy		();
	TurnDetectorInternal	(false);
	xr_delete				(m_ui);
	detector_light.destroy	();
	detector_glow.destroy	();
}

BOOL CCustomDetector::net_Spawn(CSE_Abstract* DC) 
{
	TurnDetectorInternal(false);
	return		(inherited::net_Spawn(DC));
}

void CCustomDetector::Load(LPCSTR section) 
{
	inherited::Load			(section);

	m_fAfDetectRadius		= pSettings->r_float(section,"af_radius");
	m_fAfVisRadius			= pSettings->r_float(section,"af_vis_radius");
	m_artefacts.load		(section, "af");

	m_sounds.LoadSound( section, "snd_draw", "sndShow");
	m_sounds.LoadSound( section, "snd_holster", "sndHide");

	m_fMaxChargeLevel = READ_IF_EXISTS(pSettings, r_float, section, "max_charge_level", 1.0f);
	m_fUnchargeSpeed = READ_IF_EXISTS(pSettings, r_float, section, "uncharge_speed", 0.0f);

	if (GameConstants::GetArtDetectorUseBattery())
	{
		float rnd_charge = ::Random.randF(0.0f, m_fMaxChargeLevel);
		m_fCurrentChargeLevel = rnd_charge;
	}

	m_bLightsEnabled = READ_IF_EXISTS(pSettings, r_string, section, "light_enabled", false);

	if (!detector_light && m_bLightsEnabled)
	{
		m_bLightsAlways = READ_IF_EXISTS(pSettings, r_string, section, "light_always", false);

		detector_light = ::Render->light_create();
		detector_light->set_shadow(READ_IF_EXISTS(pSettings, r_string, section, "light_shadow", false));

		m_bVolumetricLights = READ_IF_EXISTS(pSettings, r_bool, section, "volumetric_lights", false);
		m_fVolumetricQuality = READ_IF_EXISTS(pSettings, r_float, section, "volumetric_quality", 1.0f);
		m_fVolumetricDistance = READ_IF_EXISTS(pSettings, r_float, section, "volumetric_distance", 0.3f);
		m_fVolumetricIntensity = READ_IF_EXISTS(pSettings, r_float, section, "volumetric_intensity", 0.5f);

		m_iLightType = READ_IF_EXISTS(pSettings, r_u8, section, "light_type", 1);
		light_lanim = LALib.FindItem(READ_IF_EXISTS(pSettings, r_string, section, "color_animator", ""));

		const Fcolor clr = READ_IF_EXISTS(pSettings, r_fcolor, section, "light_color", (Fcolor{ 1.0f, 0.0f, 0.0f, 1.0f }));
		
		fBrightness = clr.intensity();
		detector_light->set_color(clr);

		const float range = READ_IF_EXISTS(pSettings, r_float, section, "light_range", 1.f);

		detector_light->set_range(range);
		detector_light->set_hud_mode(true);
		detector_light->set_type((IRender_Light::LT)m_iLightType);
		detector_light->set_cone(deg2rad(READ_IF_EXISTS(pSettings, r_float, section, "light_spot_angle", 1.f)));
		detector_light->set_texture(READ_IF_EXISTS(pSettings, r_string, section, "spot_texture", nullptr));

		detector_light->set_volumetric(m_bVolumetricLights);
		detector_light->set_volumetric_quality(m_fVolumetricQuality);
		detector_light->set_volumetric_distance(m_fVolumetricDistance);
		detector_light->set_volumetric_intensity(m_fVolumetricIntensity);

		//Glow
		m_bGlowEnabled = READ_IF_EXISTS(pSettings, r_string, section, "glow_enabled", false);

		if (!detector_glow && m_bGlowEnabled)
		{
			detector_glow = ::Render->glow_create();
			detector_glow->set_texture(READ_IF_EXISTS(pSettings, r_string, section, "glow_texture", nullptr));
			detector_glow->set_color(clr);
			detector_glow->set_radius(READ_IF_EXISTS(pSettings, r_float, section, "glow_radius", 0.3f));
		}
	}
}


void CCustomDetector::shedule_Update(u32 dt) 
{
	inherited::shedule_Update(dt);
	
	if( !IsWorking() )			return;

	Position().set(H_Parent()->Position());

	Fvector						P; 
	P.set						(H_Parent()->Position());
	m_artefacts.feel_touch_update(P,m_fAfDetectRadius);
}


bool CCustomDetector::IsWorking()
{
	return m_bWorking && H_Parent() && H_Parent()==Level().CurrentViewEntity();
}

void CCustomDetector::UpfateWork()
{
	if (m_fCurrentChargeLevel > 0)
		UpdateAf();

	m_ui->update			();
}

void CCustomDetector::UpdateLights()
{
	if (detector_light)
	{
		if (!detector_light->get_active() && m_fCurrentChargeLevel > 0 && IsWorking() && (m_artefacts.m_ItemInfos.size() > 0 || m_bLightsAlways))
		{
			detector_light->set_active(true);

			if (detector_glow && !detector_glow->get_active() && m_bGlowEnabled)
				detector_glow->set_active(true);
		}
		else if (detector_light->get_active() && (m_fCurrentChargeLevel <= 0 || !IsWorking() || (m_artefacts.m_ItemInfos.size() == 0) && !m_bLightsAlways))
		{
			detector_light->set_active(false);

			if (detector_glow && detector_glow->get_active() && m_bGlowEnabled)
				detector_glow->set_active(false);
		}

		if (detector_light->get_active() && HudItemData())
		{
			if (GetHUDmode())
			{
				firedeps fd;
				HudItemData()->setup_firedeps(fd);
				detector_light->set_position(fd.vLastFP2);

				if (detector_glow && detector_glow->get_active())
					detector_glow->set_position(fd.vLastFP2);
			}

			// calc color animator
			if (light_lanim)
			{
				int frame{};
				u32 clr = light_lanim->CalculateRGB(Device.fTimeGlobal, frame);
				Fcolor fclr;
				fclr.set(clr);
				detector_light->set_color(fclr);
			}
		}
	}
}

void CCustomDetector::UpdateVisibility()
{
	//check visibility
	attachable_hud_item* i0		= g_player_hud->attached_item(0);
	if(i0 && HudItemData())
	{
		bool bClimb			= ( (Actor()->MovingState()&mcClimb) != 0 );
		if(bClimb)
		{
			HideDetector		(true);
			m_bNeedActivation	= true;
		}else
		{
			CWeapon* wpn			= smart_cast<CWeapon*>(i0->m_parent_hud_item);
			if(wpn)
			{
				u32 state			= wpn->GetState();
				if(wpn->IsZoomed() || state==CWeapon::eReload || state==CWeapon::eSwitch)
				{
					HideDetector		(true);
					m_bNeedActivation	= true;
				}
			}
		}
	}else
	if(m_bNeedActivation)
	{
		attachable_hud_item* i0		= g_player_hud->attached_item(0);
		bool bClimb					= ( (Actor()->MovingState()&mcClimb) != 0 );
		if(!bClimb)
		{
			CHudItem* huditem		= (i0)?i0->m_parent_hud_item : NULL;
			bool bChecked			= !huditem || CheckCompatibilityInt(huditem, 0);
			
			if(	bChecked )
				ShowDetector		(true);
		}
	}
}

void CCustomDetector::UpdateCL() 
{
	inherited::UpdateCL();
	UpdateLights();

	if (GameConstants::GetArtDetectorUseBattery())
		UpdateChargeLevel();

	if(H_Parent()!=Level().CurrentEntity() )			return;

	UpdateVisibility		();
	if( !IsWorking() )		return;
	UpfateWork				();
}

void CCustomDetector::OnH_A_Chield() 
{
	inherited::OnH_A_Chield		();
}

void CCustomDetector::OnH_B_Independent(bool just_before_destroy) 
{
	inherited::OnH_B_Independent(just_before_destroy);
	
	m_artefacts.clear			();
}


void CCustomDetector::OnMoveToRuck(const SInvItemPlace& prev)
{
	inherited::OnMoveToRuck	(prev);
	if(prev.type==eItemPlaceSlot)
	{
		SwitchState					(eHidden);
		g_player_hud->detach_item	(this);
	}
	TurnDetectorInternal			(false);
	StopCurrentAnimWithoutCallback	();
}

void CCustomDetector::OnMoveToSlot(const SInvItemPlace& prev)
{
	inherited::OnMoveToSlot	(prev);
}

void CCustomDetector::TurnDetectorInternal(bool b)
{
	m_bWorking				= b;
	if(b && m_ui==NULL)
	{
		CreateUI			();
	}else
	{
//.		xr_delete			(m_ui);
	}

	UpdateNightVisionMode	(b);
}



#include "game_base_space.h"
void CCustomDetector::UpdateNightVisionMode(bool b_on)
{
}

void CCustomDetector::save(NET_Packet &output_packet)
{
	inherited::save(output_packet);
	save_data(m_fCurrentChargeLevel, output_packet);

}

void CCustomDetector::load(IReader &input_packet)
{
	inherited::load(input_packet);
	load_data(m_fCurrentChargeLevel, input_packet);
}

void CCustomDetector::UpdateChargeLevel(void)
{
	if (IsWorking())
	{
		float uncharge_coef = (m_fUnchargeSpeed / 16) * Device.fTimeDelta;

		m_fCurrentChargeLevel -= uncharge_coef;

		float condition = 1.f * m_fCurrentChargeLevel;
		SetCondition(condition);

		//Msg("����� ���������: %f", m_fCurrentChargeLevel); //��� ������

		clamp(m_fCurrentChargeLevel, 0.f, m_fMaxChargeLevel);
		SetCondition(m_fCurrentChargeLevel);
	}
	/*else
		SetCondition(m_fCurrentChargeLevel);*/
}

float CCustomDetector::GetUnchargeSpeed() const
{
	return m_fUnchargeSpeed;
}

float CCustomDetector::GetCurrentChargeLevel() const
{
	return m_fCurrentChargeLevel;
}

void CCustomDetector::SetCurrentChargeLevel(float val)
{
	m_fCurrentChargeLevel = val;
	clamp(m_fCurrentChargeLevel, 0.f, m_fMaxChargeLevel);

	float condition = 1.f * m_fCurrentChargeLevel / m_fUnchargeSpeed;
	SetCondition(condition);
}

void CCustomDetector::Recharge(float val)
{
	m_fCurrentChargeLevel += val;
	clamp(m_fCurrentChargeLevel, 0.f, m_fMaxChargeLevel);

	SetCondition(m_fCurrentChargeLevel);

	//Msg("��������� � �������� �����: %f", val); //��� ������
}

BOOL CAfList::feel_touch_contact	(CObject* O)
{
	TypesMapIt it				= m_TypesMap.find(O->cNameSect());

	bool res					 = (it!=m_TypesMap.end());
	if(res)
	{
		CArtefact*	pAf				= smart_cast<CArtefact*>(O);
		
		if(pAf->GetAfRank()>m_af_rank)
			res = false;
	}
	return						res;
}

bool CCustomDetector::install_upgrade_impl(LPCSTR section, bool test)
{
	//Msg("Detector Upgrade");
	bool result = inherited::install_upgrade_impl(section, test);

	result |= process_if_exists(section, "af_radius",			&CInifile::r_float, m_fAfDetectRadius,	test);
	result |= process_if_exists(section, "af_vis_radius",		&CInifile::r_float, m_fAfVisRadius,		test);
	result |= process_if_exists(section, "max_charge_level",	&CInifile::r_float, m_fMaxChargeLevel,	test);
	result |= process_if_exists(section, "uncharge_speed",		&CInifile::r_float, m_fUnchargeSpeed,	test);

	LPCSTR str;

	// name of the ltx-section of hud
	bool result2 = process_if_exists_set(section, "hud", &CInifile::r_string, str, test);
	if (result2 && !test)
		this->ReplaceHudSection(str);

	return result;
}