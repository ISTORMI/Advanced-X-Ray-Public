#include "stdafx.h"
#include "UIHudStatesWnd.h"

#include "../Actor.h"
#include "../ActorCondition.h"
#include "../EntityCondition.h"
#include "../CustomOutfit.h"
#include "../ActorHelmet.h"
#include "../inventory.h"
#include "../RadioactiveZone.h"

#include "UIActorMenu.h"
#include "UIStatic.h"
#include "UIProgressBar.h"
#include "UIProgressShape.h"
#include "UIXmlInit.h"
#include "UIHelper.h"
#include "ui_arrow.h"
#include "UIInventoryUtilities.h"
#include "CustomDetector.h"
#include "../ai/monsters/basemonster/base_monster.h"
#include "../PDA.h"
#include "WeaponMagazinedWGrenade.h"
#include "AdvancedXrayGameConstants.h"
#include "../UIGameCustom.h"
#include "../UIGameSP.h"

CUIHudStatesWnd::CUIHudStatesWnd()
:m_b_force_update(true),
	m_timer_1sec(0),
	m_last_health(0.0f)
{
	m_health_blink = pSettings->r_float( "actor_condition", "hud_health_blink" );
	clamp( m_health_blink, 0.0f, 1.0f );

	m_fake_indicators_update = false;
//-	Load_section();
}

CUIHudStatesWnd::~CUIHudStatesWnd()
{
}

void CUIHudStatesWnd::reset_ui()
{
	if ( g_pGameLevel )
	{
		Level().hud_zones_list->clear();
	}
}

void CUIHudStatesWnd::InitFromXml( CUIXml& xml, LPCSTR path )
{
	CUIXmlInit::InitWindow( xml, path, 0, this );
	XML_NODE* stored_root = xml.GetLocalRoot();
	
	XML_NODE* new_root = xml.NavigateToNode( path, 0 );
	xml.SetLocalRoot( new_root );


	m_back            = UIHelper::CreateStatic( xml, "back", this );
	m_ui_health_bar   = UIHelper::CreateProgressBar( xml, "progress_bar_health", this );
	m_ui_stamina_bar  = UIHelper::CreateProgressBar( xml, "progress_bar_stamina", this );
//	m_back_v          = UIHelper::CreateStatic( xml, "back_v", this );
//	m_static_armor    = UIHelper::CreateStatic( xml, "static_armor", this );
	
/*
	m_resist_back[ALife::infl_rad]  = UIHelper::CreateStatic( xml, "resist_back_rad", this );
	m_resist_back[ALife::infl_fire] = UIHelper::CreateStatic( xml, "resist_back_fire", this );
	m_resist_back[ALife::infl_acid] = UIHelper::CreateStatic( xml, "resist_back_acid", this );
	m_resist_back[ALife::infl_psi]  = UIHelper::CreateStatic( xml, "resist_back_psi", this );
	// electra = no has CStatic!!
*/
	m_indik[ALife::infl_rad]  = UIHelper::CreateStatic( xml, "indik_rad", this );
	m_indik[ALife::infl_fire] = UIHelper::CreateStatic( xml, "indik_fire", this );
	m_indik[ALife::infl_acid] = UIHelper::CreateStatic( xml, "indik_acid", this );
	m_indik[ALife::infl_psi]  = UIHelper::CreateStatic( xml, "indik_psi", this );

//	m_lanim_name				= xml.ReadAttrib( "indik_rad", 0, "light_anim", "" );

	m_ui_weapon_cur_ammo		= UIHelper::CreateTextWnd( xml, "static_cur_ammo", this );
	m_ui_weapon_fmj_ammo		= UIHelper::CreateTextWnd( xml, "static_fmj_ammo", this );
	m_ui_weapon_ap_ammo			= UIHelper::CreateTextWnd( xml, "static_ap_ammo", this );
	m_fire_mode					= UIHelper::CreateTextWnd( xml, "static_fire_mode", this );
	m_ui_grenade				= UIHelper::CreateTextWnd( xml, "static_grenade", this );
	
	m_ui_weapon_icon			= UIHelper::CreateStatic( xml, "static_wpn_icon", this );
	m_ui_weapon_icon->SetShader( InventoryUtilities::GetEquipmentIconsShader() );
//	m_ui_weapon_icon->Enable	( false );
	m_ui_weapon_icon_rect		= m_ui_weapon_icon->GetWndRect();

//	m_ui_armor_bar    = UIHelper::CreateProgressBar( xml, "progress_bar_armor", this );

//	m_progress_self = xr_new<CUIProgressShape>();
//	m_progress_self->SetAutoDelete(true);
//	AttachChild( m_progress_self );
//	CUIXmlInit::InitProgressShape( xml, "progress", 0, m_progress_self );

//	m_arrow				= xr_new<UI_Arrow>();
//	m_arrow_shadow		= xr_new<UI_Arrow>();

//	m_arrow->init_from_xml( xml, "arrow", this );
//	m_arrow_shadow->init_from_xml( xml, "arrow_shadow", this );

//	m_back_over_arrow = UIHelper::CreateStatic( xml, "back_over_arrow", this );

/*
	m_bleeding_lev1 = UIHelper::CreateStatic( xml, "bleeding_level_1", this );
	m_bleeding_lev1->Show( false );

	m_bleeding_lev2 = UIHelper::CreateStatic( xml, "bleeding_level_2", this );
	m_bleeding_lev2->Show( false );

	m_bleeding_lev3 = UIHelper::CreateStatic( xml, "bleeding_level_3", this );
	m_bleeding_lev3->Show( false );

	m_radiation_lev1 = UIHelper::CreateStatic( xml, "radiation_level_1", this );
	m_radiation_lev1->Show( false );

	m_radiation_lev2 = UIHelper::CreateStatic( xml, "radiation_level_2", this );
	m_radiation_lev2->Show( false );

	m_radiation_lev3 = UIHelper::CreateStatic( xml, "radiation_level_3", this );
	m_radiation_lev3->Show( false );

	for ( int i = 0; i < it_max; ++i )
	{
		m_cur_state_LA[i] = true;
		SwitchLA( false, (ALife::EInfluenceType)i );
	}
*/	
	xml.SetLocalRoot( stored_root );
}

void CUIHudStatesWnd::on_connected()
{
	Load_section();
}

void CUIHudStatesWnd::Load_section()
{
	VERIFY( g_pGameLevel );
	if ( !Level().hud_zones_list )
	{
		Level().create_hud_zones_list();
		VERIFY( Level().hud_zones_list );
	}
	
//	m_actor_radia_factor = pSettings->r_float( "radiation_zone_detector", "actor_radia_factor" );
	Level().hud_zones_list->load( "all_zone_detector", "zone" );

	Load_section_type( ALife::infl_rad,     "radiation_zone_detector" );
	Load_section_type( ALife::infl_fire,    "fire_zone_detector" );
	Load_section_type( ALife::infl_acid,    "acid_zone_detector" );
	Load_section_type( ALife::infl_psi,     "psi_zone_detector" );
	Load_section_type( ALife::infl_electra, "electra_zone_detector" );	//no uistatic
}

void CUIHudStatesWnd::Load_section_type( ALife::EInfluenceType type, LPCSTR section )
{
	/*m_zone_max_power[type] = pSettings->r_float( section, "max_power" );
	if ( m_zone_max_power[type] <= 0.0f )
	{
		m_zone_max_power[type] = 1.0f;
	}*/
	CurrentGameUI()->m_zone_feel_radius[type] = pSettings->r_float( section, "zone_radius" );
	if (CurrentGameUI()->m_zone_feel_radius[type] <= 0.0f )
	{
		CurrentGameUI()->m_zone_feel_radius[type] = 1.0f;
	}
	if (CurrentGameUI()->m_zone_feel_radius_max < CurrentGameUI()->m_zone_feel_radius[type] )
	{
		CurrentGameUI()->m_zone_feel_radius_max = CurrentGameUI()->m_zone_feel_radius[type];
	}
	CurrentGameUI()->m_zone_threshold[type] = pSettings->r_float( section, "threshold" );
}

void CUIHudStatesWnd::Update()
{
	CActor* actor = smart_cast<CActor*>( Level().CurrentViewEntity() );
	if ( !actor )
	{
		return;
	}

	UpdateHealth( actor );
	UpdateActiveItemInfo( actor );
	UpdateIndicators( actor );

	inherited::Update();
}

void CUIHudStatesWnd::UpdateHealth( CActor* actor )
{
//	if ( Device.dwTimeGlobal - m_timer_1sec > 1000 ) // 1 sec
//	{
//		m_timer_1sec = Device.dwTimeGlobal;
//	}
	
	float cur_health = actor->GetfHealth();
	m_ui_health_bar->SetProgressPos(iCeil(cur_health * 100.0f * 35.f) / 35.f);
	if ( _abs(cur_health - m_last_health) > m_health_blink )
	{
		m_last_health = cur_health;
		m_ui_health_bar->m_UIProgressItem.ResetColorAnimation();
	}
	
	float cur_stamina = actor->conditions().GetPower();
	m_ui_stamina_bar->SetProgressPos(iCeil(cur_stamina * 100.0f * 35.f) / 35.f);
	if ( !actor->conditions().IsCantSprint() )
	{
		m_ui_stamina_bar->m_UIProgressItem.ResetColorAnimation();
	}

/*
	CCustomOutfit* outfit = actor->GetOutfit();
	if ( outfit )
	{
		m_static_armor->Show( true );
		m_ui_armor_bar->Show( true );
		m_ui_armor_bar->SetProgressPos( outfit->GetCondition() * 100.0f );
	}
	else
	{
		m_static_armor->Show( false );
		m_ui_armor_bar->Show( false );
	}
*/	
	/*
	float bleeding_speed = actor->conditions().BleedingSpeed();
	if(bleeding_speed > 0.01f)
		m_bleeding_lev1->Show(true);
	else
		m_bleeding_lev1->Show(false);

	if(bleeding_speed > 0.35f)
		m_bleeding_lev2->Show(true);
	else
		m_bleeding_lev2->Show(false);
	
	if(bleeding_speed > 0.7f)
		m_bleeding_lev3->Show(true);
	else
		m_bleeding_lev3->Show(false);
	
	
	if(m_radia_self > 0.01f)
		m_radiation_lev1->Show(true);
	else
		m_radiation_lev1->Show(false);

	if(m_radia_self > 0.35f)
		m_radiation_lev2->Show(true);
	else
		m_radiation_lev2->Show(false);
	
	if(m_radia_self > 0.7f)
		m_radiation_lev3->Show(true);
	else
		m_radiation_lev3->Show(false);
		*/
}

void CUIHudStatesWnd::UpdateActiveItemInfo( CActor* actor )
{
	PIItem item = actor->inventory().ActiveItem();
	if ( item )
	{
		if(m_b_force_update)
		{
			if(item->cast_weapon())
				item->cast_weapon()->ForceUpdateAmmo();
			m_b_force_update		= false;
		}

		item->GetBriefInfo			( m_item_info );

//		UIWeaponBack.SetText		( str_name.c_str() );
		m_fire_mode->SetText		( m_item_info.fire_mode.c_str() );
		SetAmmoIcon					( m_item_info.icon.c_str() );
		
		m_ui_weapon_cur_ammo->Show	( true );
		m_ui_weapon_fmj_ammo->Show	( true );
		m_ui_weapon_ap_ammo->Show	( true );
		m_fire_mode->Show			( true );
		m_ui_grenade->Show			( true );

		m_ui_weapon_cur_ammo->SetText	( m_item_info.cur_ammo.c_str() );
		m_ui_weapon_fmj_ammo->SetText	( m_item_info.fmj_ammo.c_str() );
		m_ui_weapon_ap_ammo->SetText	( m_item_info.ap_ammo.c_str() );
		
		m_ui_grenade->SetText	( m_item_info.grenade.c_str() );

		CWeaponMagazinedWGrenade* wpn = smart_cast<CWeaponMagazinedWGrenade*>(item);
		if(wpn && wpn->m_bGrenadeMode)
		{
			m_ui_weapon_fmj_ammo->SetTextColor(color_rgba(238,155,23,150));
			m_ui_grenade->SetTextColor(color_rgba(238,155,23,255));
		}
		else
		{
			m_ui_weapon_fmj_ammo->SetTextColor(color_rgba(238,155,23,255));
			m_ui_grenade->SetTextColor(color_rgba(238,155,23,150));
		}
	}
	else
	{
		m_ui_weapon_icon->Show		( false );

		m_ui_weapon_cur_ammo->Show	( false );
		m_ui_weapon_fmj_ammo->Show	( false );
		m_ui_weapon_ap_ammo->Show	( false );
		m_fire_mode->Show			( false );
		m_ui_grenade->Show			( false );
	}
}

void CUIHudStatesWnd::SetAmmoIcon(const shared_str& sect_name)
{
	if (!sect_name.size())
	{
		m_ui_weapon_icon->Show(false);
		return;
	}
	m_ui_weapon_icon->Show(true);

	Frect texture_rect;
	texture_rect.x1					= pSettings->r_float(sect_name,  "inv_grid_x")		*INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons());
	texture_rect.y1					= pSettings->r_float(sect_name,  "inv_grid_y")		*INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons());
	texture_rect.x2					= pSettings->r_float( sect_name, "inv_grid_width")	*INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons());
	texture_rect.y2					= pSettings->r_float( sect_name, "inv_grid_height")	*INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons());
	texture_rect.rb.add				(texture_rect.lt);
	m_ui_weapon_icon->GetUIStaticItem().SetTextureRect(texture_rect);
	m_ui_weapon_icon->SetStretchTexture(true);

	float h = texture_rect.height() * 0.8f;
	float w = texture_rect.width() * 0.8f;

	// now perform only width scale for ammo, which (W)size >2
	if (GameConstants::GetUseHQ_Icons())
	{
		if (texture_rect.width() > 2.01f * INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons()))
			w = INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons()) * 1.5f / 2;

		m_ui_weapon_icon->SetWidth(w * UI().get_current_kx() / 2);
		m_ui_weapon_icon->SetHeight(h / 2);
	}
	else
	{
		if (texture_rect.width() > 2.01f * INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons()))
			w = INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons()) * 1.5f;

		m_ui_weapon_icon->SetWidth(w * UI().get_current_kx());
		m_ui_weapon_icon->SetHeight(h);
	}
}
// ------------------------------------------------------------------------------------------------

void CUIHudStatesWnd::UpdateIndicators( CActor* actor )
{
	if(m_fake_indicators_update)
		return;

	for ( int i = 0; i < it_max ; ++i ) // it_max = ALife::infl_max_count-1
	{
		UpdateIndicatorType( actor, (ALife::EInfluenceType)i );
	}
}

void CUIHudStatesWnd::UpdateIndicatorType( CActor* actor, ALife::EInfluenceType type )
{
	if ( type < ALife::infl_rad || ALife::infl_psi < type )
	{
		VERIFY2( 0, "Failed EIndicatorType for CStatic!" );
		return;
	}

/*	
	u32 c_white  = color_rgba( 255, 255, 255, 255 );
	u32 c_green  = color_rgba( 0, 255, 0, 255 );
	u32 c_yellow = color_rgba( 255, 255, 0, 255 );
	u32 c_red    = color_rgba( 255, 0, 0, 255 );
*/
	LPCSTR texture = "";
	string128 str;
	switch(type)
	{
		case ALife::infl_rad: texture = "ui_inGame2_triangle_Radiation_"; break;
		case ALife::infl_fire: texture = "ui_inGame2_triangle_Fire_"; break;
		case ALife::infl_acid: texture = "ui_inGame2_triangle_Biological_"; break;
		case ALife::infl_psi: texture = "ui_inGame2_triangle_Psy_"; break;
		default: NODEFAULT;
	}
	float           hit_power = CurrentGameUI()->m_zone_cur_power[type];
	ALife::EHitType hit_type  = CurrentGameUI()->m_zone_hit_type[type];

#pragma todo("it crashes in mp, please fix it")
	if (!IsGameTypeSingle())
		return;
	
	CCustomOutfit* outfit = actor->GetOutfit();
	CHelmet* helmet = smart_cast<CHelmet*>(actor->inventory().ItemFromSlot(HELMET_SLOT));
	float protect = (outfit) ? outfit->GetDefHitTypeProtection( hit_type ) : 0.0f;
	protect += (helmet) ? helmet->GetDefHitTypeProtection(hit_type) : 0.0f;
	protect += actor->GetProtection_ArtefactsOnBelt( hit_type );

	CEntityCondition::BOOSTER_MAP cur_booster_influences = actor->conditions().GetCurBoosterInfluences();
	CEntityCondition::BOOSTER_MAP::const_iterator it;
	if(hit_type==ALife::eHitTypeChemicalBurn)
	{
		it = cur_booster_influences.find(eBoostChemicalBurnProtection);
		if(it!=cur_booster_influences.end())
			protect += it->second.fBoostValue;
	}
	else if(hit_type==ALife::eHitTypeRadiation)
	{
		it = cur_booster_influences.find(eBoostRadiationProtection);
		if(it!=cur_booster_influences.end())
			protect += it->second.fBoostValue;
	}
	else if(hit_type==ALife::eHitTypeTelepatic)
	{
		it = cur_booster_influences.find(eBoostTelepaticProtection);
		if(it!=cur_booster_influences.end())
			protect += it->second.fBoostValue;
	}

//	float max_power = actor->conditions().GetZoneMaxPower( hit_type );
//	protect = protect / max_power; // = 0..1

	if ( hit_power < EPS )
	{
		m_indik[type]->Show(false);
//		m_indik[type]->SetTextureColor( c_white );
//		SwitchLA( false, type );
		actor->conditions().SetZoneDanger( 0.0f, type );
		return;
	}

	m_indik[type]->Show(true);
	if ( hit_power <= protect )
	{
//		m_indik[type]->SetTextureColor( c_green );
//		SwitchLA( false, type );
		xr_sprintf(str, sizeof(str), "%s%s", texture, "green");
		texture = str;
		m_indik[type]->InitTexture(texture);
		actor->conditions().SetZoneDanger( 0.0f, type );
		return;
	}
	if ( hit_power - protect < CurrentGameUI()->m_zone_threshold[type] )
	{
//		m_indik[type]->SetTextureColor( c_yellow );
//		SwitchLA( false, type );
		xr_sprintf(str, sizeof(str), "%s%s", texture, "yellow");
		texture = str;
		m_indik[type]->InitTexture(texture);
		actor->conditions().SetZoneDanger( 0.0f, type );
		return;
	}
//	m_indik[type]->SetTextureColor( c_red );
//	SwitchLA( true, type );
	xr_sprintf(str, sizeof(str), "%s%s", texture, "red");
	texture = str;
	m_indik[type]->InitTexture(texture);
	VERIFY(actor->conditions().GetZoneMaxPower(hit_type));
	actor->conditions().SetZoneDanger((hit_power-protect)/actor->conditions().GetZoneMaxPower(hit_type), type);
}
/*
void CUIHudStatesWnd::SwitchLA( bool state, ALife::EInfluenceType type )
{
	if ( state == m_cur_state_LA[type] )
	{
		return;
	}

	if ( state )
	{
		m_indik[type]->SetColorAnimation( m_lanim_name.c_str(), LA_CYCLIC|LA_TEXTURECOLOR);
		m_cur_state_LA[type] = true;
	}
	else
	{
		m_indik[type]->SetColorAnimation( NULL, 0);//off
		m_cur_state_LA[type] = false;
	}
}
*/

void CUIHudStatesWnd::DrawZoneIndicators()
{
	CActor* actor = smart_cast<CActor*>(Level().CurrentViewEntity());
	if(!actor)
		return;

	UpdateIndicators(actor);

	if(m_indik[ALife::infl_rad]->IsShown())
		m_indik[ALife::infl_rad]->Draw();

	if(m_indik[ALife::infl_fire]->IsShown())
		m_indik[ALife::infl_fire]->Draw();

	if(m_indik[ALife::infl_acid]->IsShown())
		m_indik[ALife::infl_acid]->Draw();

	if(m_indik[ALife::infl_psi]->IsShown())
		m_indik[ALife::infl_psi]->Draw();
}

void CUIHudStatesWnd::FakeUpdateIndicatorType(u8 t, float power)
{
	ALife::EInfluenceType type = (ALife::EInfluenceType)t;
	if ( type < ALife::infl_rad || ALife::infl_psi < type )
	{
		VERIFY2( 0, "Failed EIndicatorType for CStatic!" );
		return;
	}

	CActor* actor = smart_cast<CActor*>( Level().CurrentViewEntity() );
	if(!actor)
		return;

	LPCSTR texture = "";
	string128 str;
	switch(type)
	{
		case ALife::infl_rad: texture = "ui_inGame2_triangle_Radiation_"; break;
		case ALife::infl_fire: texture = "ui_inGame2_triangle_Fire_"; break;
		case ALife::infl_acid: texture = "ui_inGame2_triangle_Biological_"; break;
		case ALife::infl_psi: texture = "ui_inGame2_triangle_Psy_"; break;
		default: NODEFAULT;
	}
	float           hit_power = power;
	ALife::EHitType hit_type  = CurrentGameUI()->m_zone_hit_type[type];
	
	CCustomOutfit* outfit = actor->GetOutfit();
	CHelmet* helmet = smart_cast<CHelmet*>(actor->inventory().ItemFromSlot(HELMET_SLOT));
	float protect = (outfit) ? outfit->GetDefHitTypeProtection( hit_type ) : 0.0f;
	protect += (helmet) ? helmet->GetDefHitTypeProtection(hit_type) : 0.0f;
	protect += actor->GetProtection_ArtefactsOnBelt( hit_type );

	CEntityCondition::BOOSTER_MAP cur_booster_influences = actor->conditions().GetCurBoosterInfluences();
	CEntityCondition::BOOSTER_MAP::const_iterator it;
	if(hit_type==ALife::eHitTypeChemicalBurn)
	{
		it = cur_booster_influences.find(eBoostChemicalBurnProtection);
		if(it!=cur_booster_influences.end())
			protect += it->second.fBoostValue;
	}
	else if(hit_type==ALife::eHitTypeRadiation)
	{
		it = cur_booster_influences.find(eBoostRadiationProtection);
		if(it!=cur_booster_influences.end())
			protect += it->second.fBoostValue;
	}
	else if(hit_type==ALife::eHitTypeTelepatic)
	{
		it = cur_booster_influences.find(eBoostTelepaticProtection);
		if(it!=cur_booster_influences.end())
			protect += it->second.fBoostValue;
	}

	float max_power = actor->conditions().GetZoneMaxPower( hit_type );
	protect = protect / max_power; // = 0..1

	if ( hit_power < EPS )
	{
		m_indik[type]->Show(false);
		actor->conditions().SetZoneDanger( 0.0f, type );
		return;
	}

	m_indik[type]->Show(true);
	if ( hit_power < protect )
	{
		xr_sprintf(str, sizeof(str), "%s%s", texture, "green");
		texture = str;
		m_indik[type]->InitTexture(texture);
		actor->conditions().SetZoneDanger( 0.0f, type );
		return;
	}
	if ( hit_power - protect < CurrentGameUI()->m_zone_threshold[type] )
	{
		xr_sprintf(str, sizeof(str), "%s%s", texture, "yellow");
		texture = str;
		m_indik[type]->InitTexture(texture);
		actor->conditions().SetZoneDanger( 0.0f, type );
		return;
	}
	xr_sprintf(str, sizeof(str), "%s%s", texture, "red");
	texture = str;
	m_indik[type]->InitTexture(texture);
	actor->conditions().SetZoneDanger( hit_power - protect, type );
}

void CUIHudStatesWnd::EnableFakeIndicators(bool enable)
{
	m_fake_indicators_update = enable;
}