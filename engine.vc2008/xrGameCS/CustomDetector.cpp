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

bool  CCustomDetector::CheckCompatibilityInt(CHudItem* itm)
{
	if(itm==NULL)
		return true;

	CInventoryItem iitm				= itm->item();
	u32 slot						= iitm.GetSlot();
	bool bres = (slot==PISTOL_SLOT || slot==KNIFE_SLOT || slot==BOLT_SLOT);

	if(itm->GetState()!=CHUDState::eShowing)
		bres = bres && !itm->IsPending();

	if(bres)
	{
		CWeapon* W = smart_cast<CWeapon*>(itm);
		if(W)
			bres = bres && (W->GetState()!=CHUDState::eBore) && !W->IsZoomed();
	}
	return bres;
}

bool  CCustomDetector::CheckCompatibility(CHudItem* itm)
{
	if(!inherited::CheckCompatibility(itm) )	
		return false;

	if(!CheckCompatibilityInt(itm))
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
	m_bFastAnimMode = bFastMode;
	if(GetState()==eHidden)
	{
		PIItem iitem = m_pInventory->ActiveItem();
		CHudItem* itm = (iitem)?iitem->cast_hud_item():NULL;
		if(CheckCompatibilityInt(itm))
		{
			SwitchState				(eShowing);
			TurnDetectorInternal	(true);
		}
	}else
	if(GetState()==eIdle)
		SwitchState					(eHiding);

	m_bNeedActivation = false;
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
			PlayHUDMotion				(m_bFastAnimMode?"anm_show_fast":"anm_show", TRUE, this, GetState());
			SetPending					(TRUE);
		}break;
	case eHiding:
		{
			m_sounds.PlaySound			("sndHide", Fvector().set(0,0,0), this, true, false);
			PlayHUDMotion				(m_bFastAnimMode?"anm_hide_fast":"anm_hide", TRUE, this, GetState());
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
	UpdateAf				();
	m_ui->update			();
}

void CCustomDetector::UpdateVisibility()
{


	//check visibility
	attachable_hud_item* i0		= g_player_hud->attached_item(0);
	if(i0 && HudItemData())
	{
		CWeapon* wpn			= smart_cast<CWeapon*>(i0->m_parent_hud_item);
		if(wpn)
		{
			u32 state			= wpn->GetState();
			bool bClimb			= ( (Actor()->MovingState()&mcClimb) != 0 );
			if(bClimb || wpn->IsZoomed() || state==CWeapon::eReload || state==CWeapon::eSwitch)
			{
				HideDetector		(true);
				m_bNeedActivation	= true;
			}
		}
	}else
	if(m_bNeedActivation)
	{
		attachable_hud_item* i0		= g_player_hud->attached_item(0);
		bool bClimb					= ( (Actor()->MovingState()&mcClimb) != 0 );
		if(!bClimb)
		{
			CWeapon* wpn			= (i0)?smart_cast<CWeapon*>(i0->m_parent_hud_item) : NULL;
			if(	!wpn || 
				(	!wpn->IsZoomed() && 
					wpn->GetState()!=CWeapon::eReload && 
					wpn->GetState()!=CWeapon::eSwitch
				)
			)
			{
				ShowDetector		(true);
			}
		}
	}
}

void CCustomDetector::UpdateCL() 
{
	inherited::UpdateCL();

	if (GameConstants::GetArtDetectorUseBattery())
		UpdateChargeLevel();

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


void CCustomDetector::OnMoveToRuck(EItemPlace prev)
{
	inherited::OnMoveToRuck	(prev);
	if(GetState()==eIdle)
	{
		SwitchState					(eHidden);
		g_player_hud->detach_item	(this);
	}
	TurnDetectorInternal	(false);
}

void CCustomDetector::OnMoveToSlot()
{
	inherited::OnMoveToSlot	();
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

		clamp(m_fCurrentChargeLevel, 0.f, 1.f);
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
	float condition = 1.f * m_fCurrentChargeLevel / m_fUnchargeSpeed;
	SetCondition(condition);
}

void CCustomDetector::Recharge(float val)
{
	m_fCurrentChargeLevel = m_fCurrentChargeLevel + val;

	SetCondition(m_fCurrentChargeLevel);

	//Msg("��������� � �������� �����: %f", val); //��� ������

	if (m_fCurrentChargeLevel > m_fMaxChargeLevel)
		m_fCurrentChargeLevel = m_fMaxChargeLevel;
}

BOOL CAfList::feel_touch_contact	(CObject* O)
{
	CLASS_ID	clsid			= O->CLS_ID;
	TypesMapIt it				= m_TypesMap.find(clsid);

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

	result |= process_if_exists(section, "af_radius", &CInifile::r_float, m_fAfDetectRadius, test);
	result |= process_if_exists(section, "af_vis_radius", &CInifile::r_float, m_fAfVisRadius, test);
	result |= process_if_exists(section, "max_charge_level", &CInifile::r_float, m_fMaxChargeLevel, test);
	result |= process_if_exists(section, "uncharge_speed", &CInifile::r_float, m_fUnchargeSpeed, test);
	result |= process_if_exists(section, "inv_weight", &CInifile::r_float, m_weight, test);

	return result;
}