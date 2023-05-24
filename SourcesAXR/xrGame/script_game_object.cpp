////////////////////////////////////////////////////////////////////////////
//	Module 		: script_game_object.cpp
//	Created 	: 25.09.2003
//  Modified 	: 29.06.2004
//	Author		: Dmitriy Iassenev
//	Description : Script game object class
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_game_object.h"
#include "script_game_object_impl.h"
#include "script_entity_action.h"
#include "ai_space.h"
#include "script_engine.h"
#include "script_entity.h"
#include "physicsshellholder.h"
#include "helicopter.h"
#include "holder_custom.h"
#include "inventoryowner.h"
#include "movement_manager.h"
#include "entity_alive.h"
#include "weaponmagazined.h"
#include "xrmessages.h"
#include "inventory.h"
#include "script_ini_file.h"
#include "../Include/xrRender/Kinematics.h"
#include "HangingLamp.h"
#include "patrol_path_manager.h"
#include "ai_object_location.h"
#include "custommonster.h"
#include "entitycondition.h"
#include "space_restrictor.h"
#include "detail_path_manager.h"
#include "level_graph.h"
#include "actor.h"
#include "actor_memory.h"
#include "visual_memory_manager.h"
#include "smart_cover_object.h"
#include "smart_cover.h"
#include "smart_cover_description.h"
#include "physics_shell_scripted.h"
#include "PDA.h"
#include "ai/phantom/phantom.h"
#include "UIGameCustom.h"
#include "ui/UIActorMenu.h"
#include "InventoryBox.h"
#include "player_hud.h"
#include "ai/stalker/ai_stalker.h"

class CScriptBinderObject;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
Fvector	CScriptGameObject::Center()
{
	Fvector c;
	m_game_object->Center(c);
	return	c;
}

BIND_FUNCTION10	(&object(),	CScriptGameObject::Position,			CGameObject,	Position,			Fvector,						Fvector());
BIND_FUNCTION10	(&object(),	CScriptGameObject::Direction,			CGameObject,	Direction,			Fvector,						Fvector());
BIND_FUNCTION10	(&object(),	CScriptGameObject::Mass,		CPhysicsShellHolder,	GetMass,			float,							float(-1));
BIND_FUNCTION10	(&object(),	CScriptGameObject::ID,					CGameObject,	ID,					u16,							u16(-1));
BIND_FUNCTION10	(&object(),	CScriptGameObject::getVisible,			CGameObject,	getVisible,			BOOL,							FALSE);
//BIND_FUNCTION01	(&object(),	CScriptGameObject::setVisible,			CGameObject,	setVisible,			BOOL,							BOOL);
BIND_FUNCTION10	(&object(),	CScriptGameObject::getEnabled,			CGameObject,	getEnabled,			BOOL,							FALSE);
//BIND_FUNCTION01	(&object(),	CScriptGameObject::setEnabled,			CGameObject,	setEnabled,			BOOL,							BOOL);
BIND_FUNCTION10	(&object(),	CScriptGameObject::story_id,			CGameObject,	story_id,			ALife::_STORY_ID,				ALife::_STORY_ID(-1));
BIND_FUNCTION10	(&object(),	CScriptGameObject::DeathTime,			CEntity,		GetLevelDeathTime,	u32,							0);
BIND_FUNCTION10	(&object(),	CScriptGameObject::MaxHealth,			CEntity,		GetMaxHealth,		float,							-1);
BIND_FUNCTION10	(&object(),	CScriptGameObject::Accuracy,			CInventoryOwner,GetWeaponAccuracy,	float,							-1);
BIND_FUNCTION10	(&object(),	CScriptGameObject::Team,				CEntity,		g_Team,				int,							-1);
BIND_FUNCTION10	(&object(),	CScriptGameObject::Squad,				CEntity,		g_Squad,			int,							-1);
BIND_FUNCTION10	(&object(),	CScriptGameObject::Group,				CEntity,		g_Group,			int,							-1);
BIND_FUNCTION10	(&object(),	CScriptGameObject::GetFOV,				CEntityAlive,	ffGetFov,			float,							-1);
BIND_FUNCTION10	(&object(),	CScriptGameObject::GetRange,			CEntityAlive,	ffGetRange,			float,							-1);
BIND_FUNCTION10	(&object(),	CScriptGameObject::GetHealth,			CEntityAlive,	conditions().GetHealth,			float,							-1);
BIND_FUNCTION10	(&object(),	CScriptGameObject::GetPsyHealth,		CEntityAlive,	conditions().GetPsyHealth,		float,							-1);
BIND_FUNCTION10	(&object(),	CScriptGameObject::GetPower,			CEntityAlive,	conditions().GetPower,			float,							-1);
BIND_FUNCTION10	(&object(),	CScriptGameObject::GetSatiety,			CEntityAlive,	conditions().GetSatiety,		float,							-1);
BIND_FUNCTION10	(&object(),	CScriptGameObject::GetRadiation,		CEntityAlive,	conditions().GetRadiation,		float,							-1);
BIND_FUNCTION10	(&object(),	CScriptGameObject::GetBleeding,			CEntityAlive,	conditions().BleedingSpeed,		float,							-1);
BIND_FUNCTION10	(&object(),	CScriptGameObject::GetMorale,			CEntityAlive,	conditions().GetEntityMorale,	float,							-1);
BIND_FUNCTION01	(&object(),	CScriptGameObject::SetHealth,			CEntityAlive,	conditions().ChangeHealth,		float,							float);
BIND_FUNCTION01	(&object(),	CScriptGameObject::SetPsyHealth,		CEntityAlive,	conditions().ChangePsyHealth,	float,							float);
BIND_FUNCTION01	(&object(),	CScriptGameObject::SetPower,			CEntityAlive,	conditions().ChangePower,		float,							float);
BIND_FUNCTION01	(&object(),	CScriptGameObject::ChangeSatiety,		CEntityAlive,	conditions().ChangeSatiety,		float,							float);
BIND_FUNCTION01	(&object(),	CScriptGameObject::SetRadiation,		CEntityAlive,	conditions().ChangeRadiation,	float,							float);
BIND_FUNCTION01	(&object(),	CScriptGameObject::SetBleeding,			CEntityAlive,	conditions().ChangeBleeding,	float,							float);
BIND_FUNCTION01	(&object(),	CScriptGameObject::SetCircumspection,	CEntityAlive,	conditions().ChangeCircumspection,float,							float);
BIND_FUNCTION01	(&object(),	CScriptGameObject::SetMorale,			CEntityAlive,	conditions().ChangeEntityMorale,	float,							float);
BIND_FUNCTION02	(&object(),	CScriptGameObject::SetScriptControl,	CScriptEntity,	SetScriptControl,	bool,								LPCSTR,					bool,					shared_str);
BIND_FUNCTION10	(&object(),	CScriptGameObject::GetScriptControl,	CScriptEntity,	GetScriptControl,	bool,								false);
BIND_FUNCTION10	(&object(),	CScriptGameObject::GetScriptControlName,CScriptEntity,GetScriptControlName,LPCSTR,					"");
BIND_FUNCTION10	(&object(),	CScriptGameObject::GetEnemyStrength,	CScriptEntity,	get_enemy_strength,	int,					0);
BIND_FUNCTION10	(&object(),	CScriptGameObject::GetActionCount,		CScriptEntity,	GetActionCount,		u32,					0);
BIND_FUNCTION10	(&object(),	CScriptGameObject::can_script_capture,	CScriptEntity,	can_script_capture,	bool,					0);
BIND_FUNCTION10 (&object(), CScriptGameObject::GetThirst,			CEntityAlive,	conditions().GetThirst,			float,		-1);
BIND_FUNCTION01 (&object(), CScriptGameObject::ChangeThirst,		CEntityAlive,	conditions().ChangeThirst,		float, float);
BIND_FUNCTION10 (&object(), CScriptGameObject::GetIntoxication,		CEntityAlive,	conditions().GetIntoxication,	float, -1);
BIND_FUNCTION01 (&object(), CScriptGameObject::ChangeIntoxication,	CEntityAlive,	conditions().ChangeIntoxication, float, float);
BIND_FUNCTION10	(&object(), CScriptGameObject::GetSleepeness,		CEntityAlive,	conditions().GetSleepeness,		float, -1);
BIND_FUNCTION01	(&object(), CScriptGameObject::ChangeSleepeness,	CEntityAlive,	conditions().ChangeSleepeness,	float, float);
BIND_FUNCTION10	(&object(), CScriptGameObject::GetAlcoholism,		CEntityAlive,	conditions().GetAlcoholism,		float, -1);
BIND_FUNCTION01	(&object(), CScriptGameObject::ChangeAlcoholism,	CEntityAlive,	conditions().ChangeAlcoholism,	float, float);
BIND_FUNCTION10	(&object(), CScriptGameObject::GetHangover,			CEntityAlive,	conditions().GetHangover,		float, -1);
BIND_FUNCTION01	(&object(), CScriptGameObject::ChangeHangover,		CEntityAlive,	conditions().ChangeHangover,	float, float);
BIND_FUNCTION10	(&object(), CScriptGameObject::GetNarcotism,		CEntityAlive,	conditions().GetNarcotism,		float, -1);
BIND_FUNCTION01	(&object(), CScriptGameObject::ChangeNarcotism,		CEntityAlive,	conditions().ChangeNarcotism,	float, float);
BIND_FUNCTION10	(&object(), CScriptGameObject::GetWithdrawal,		CEntityAlive,	conditions().GetWithdrawal,		float, -1);
BIND_FUNCTION01	(&object(), CScriptGameObject::ChangeWithdrawal,	CEntityAlive,	conditions().ChangeWithdrawal,	float, float);

u32	CScriptGameObject::level_vertex_id		() const
{
	return						(object().ai_location().level_vertex_id());
}

u32 CScriptGameObject::game_vertex_id		() const
{
	return						(object().ai_location().game_vertex_id());
}

CScriptIniFile *CScriptGameObject::spawn_ini			() const
{
	return			((CScriptIniFile*)object().spawn_ini());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CGameObject &CScriptGameObject::object	() const
{
#ifdef DEBUG
	__try {
		if (m_game_object && m_game_object->lua_game_object() == this)
			return	(*m_game_object);
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
	}

	ai().script_engine().script_log(eLuaMessageTypeError,"you are trying to use a destroyed object [%x]",m_game_object);
	R_ASSERT(m_game_object && m_game_object->lua_game_object() == this,"Probably, you are trying to use a destroyed object!");
#endif // #ifdef DEBUG
	return			(*m_game_object);
}

void CScriptGameObject::ResetActionQueue()
{
	MakeObj(CScriptEntity,monster);
	monster->ClearActionQueue();
}

CScriptEntityAction	*CScriptGameObject::GetCurrentAction	() const
{
	CScriptEntity		*monster = smart_cast<CScriptEntity*>(&object());
	if (!monster)
		RCAST_ERR(CScriptEntity,monster,NULL);
	else
		if (monster->GetCurrentAction())
			return		(xr_new<CScriptEntityAction>(monster->GetCurrentAction()));
	return				(0);
}

void CScriptGameObject::AddAction	(const CScriptEntityAction *tpEntityAction, bool bHighPriority)
{
	MakeObj(CScriptEntity,monster);
	monster->AddAction(tpEntityAction, bHighPriority);
}

const CScriptEntityAction *CScriptGameObject::GetActionByIndex(u32 action_index)
{
	RMakeObj(CScriptEntity,monster,0);
	return			(monster->GetActionByIndex(action_index));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

u16 CScriptGameObject::get_bone_id(LPCSTR bone_name) const
{
	return object().Visual()->dcast_PKinematics()->LL_BoneID(bone_name);
}

cphysics_shell_scripted* CScriptGameObject::get_physics_shell() const
{
	RMakeObj(CPhysicsShellHolder,ph_shell_holder,nullptr);
	if(! ph_shell_holder->PPhysicsShell() ) 
		return nullptr;
	return get_script_wrapper<cphysics_shell_scripted>(*ph_shell_holder->PPhysicsShell());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CHelicopter* CScriptGameObject::get_helicopter	()
{
	CHelicopter		*helicopter = smart_cast<CHelicopter*>(&object());
	if (!helicopter) {
		CAST_ERR(CHelicopter,helicopter);
		NODEFAULT;
	}
	return helicopter;
}


CHangingLamp* CScriptGameObject::get_hanging_lamp()
{
	CHangingLamp*	lamp = smart_cast<CHangingLamp*>(&object());
	if (!lamp) {
		CAST_ERR(CHangingLamp,lamp);
		NODEFAULT;
	}
	return lamp;
}

CHolderCustom* CScriptGameObject::get_custom_holder()
{
	RMakeObj(CHolderCustom,holder,nullptr);
	return holder;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

LPCSTR CScriptGameObject::WhoHitName()
{
	RMakeObj(CEntityAlive,entity_alive,NULL);
	return			entity_alive->conditions().GetWhoHitLastTime()?(*entity_alive->conditions().GetWhoHitLastTime()->cName()):NULL;
}

LPCSTR CScriptGameObject::WhoHitSectionName()
{
	RMakeObj(CEntityAlive,entity_alive,NULL);
	return			entity_alive->conditions().GetWhoHitLastTime()?(*entity_alive->conditions().GetWhoHitLastTime()->cNameSect()):NULL;
}

bool CScriptGameObject::CheckObjectVisibility(const CScriptGameObject *tpLuaGameObject)
{
	CEntityAlive		*entity_alive = smart_cast<CEntityAlive*>(&object());
	if (entity_alive && !entity_alive->g_Alive()) {
		Msg("! CScriptGameObject: cannot check visibility of dead object");
		return			(false);
	}

	CScriptEntity		*script_entity = smart_cast<CScriptEntity*>(&object());
	if (!script_entity) {
		RMakeObj(CActor,actor,false);
		return			(actor->memory().visual().visible_now(&tpLuaGameObject->object()));
	}

	return				(script_entity->CheckObjectVisibility(&tpLuaGameObject->object()));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CScriptBinderObject	*CScriptGameObject::binded_object	()
{
	return									(object().object());
}

void CScriptGameObject::bind_object			(CScriptBinderObject *game_object)
{
	object().set_object	(game_object);
}

void CScriptGameObject::set_previous_point	(int point_index)
{
	MakeObj(CCustomMonster,monster);
	monster->movement().patrol().set_previous_point(point_index);
}

void CScriptGameObject::set_start_point	(int point_index)
{
	MakeObj(CCustomMonster,monster);
	monster->movement().patrol().set_start_point(point_index);
}

u32 CScriptGameObject::get_current_patrol_point_index()
{
	RMakeObj(CCustomMonster,monster,u32(-1));
	return				(monster->movement().patrol().get_current_point_index());
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Fvector CScriptGameObject::bone_position(LPCSTR bone_name, bool bHud) const
{
	IKinematics* k = nullptr;

	CHudItem* itm = smart_cast<CHudItem*>(&object());
	if (bHud && itm && itm->HudItemData())
		k = itm->HudItemData()->m_model;
	else
		k = object().Visual()->dcast_PKinematics();

	u16 bone_id;
	if (xr_strlen(bone_name))
	{
		bone_id = k->LL_BoneID(bone_name);
		if (bone_id == BI_NONE)
			bone_id = k->LL_GetBoneRoot();
	}
	else
		bone_id = k->LL_GetBoneRoot();

	Fmatrix matrix;
	matrix.mul_43((bHud && itm && itm->HudItemData()) ? itm->HudItemData()->m_item_transform : object().XFORM(),
	              k->LL_GetBoneInstance(bone_id).mTransform);
	return (matrix.c);
}

Fvector CScriptGameObject::bone_direction(LPCSTR bone_name, bool bHud) const
{
    IKinematics* k = nullptr;

    CHudItem* itm = smart_cast<CHudItem*>(&object());
    if (bHud && itm)
        k = itm->HudItemData()->m_model;
    else
        k = object().Visual()->dcast_PKinematics();

    u16 bone_id;
    if (xr_strlen(bone_name))
    {
        bone_id = k->LL_BoneID(bone_name);
        if (bone_id == BI_NONE)
            bone_id = k->LL_GetBoneRoot();
    }
    else
        bone_id = k->LL_GetBoneRoot();

    Fmatrix matrix;
    Fvector res;
    matrix.mul_43((bHud && itm) ? itm->HudItemData()->m_item_transform : object().XFORM(),
                  k->LL_GetTransform(bone_id));
    matrix.getHPB(res);
    return (res);
}

LPCSTR CScriptGameObject::bone_name(u16 id, bool bHud)
{
	IKinematics* k = nullptr;

	CHudItem* itm = smart_cast<CHudItem*>(&object());
	if (bHud && itm && itm->HudItemData())
		k = itm->HudItemData()->m_model;
	else
		k = object().Visual()->dcast_PKinematics();

	return (k->LL_BoneName_dbg(id));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

u32 CScriptGameObject::GetAmmoElapsed()
{
	RMakeObj(CWeapon,weapon,0);
	return			(weapon->GetAmmoElapsed());
}

void CScriptGameObject::SetAmmoElapsed(int ammo_elapsed)
{
	MakeObj(CWeapon,weapon);
	weapon->SetAmmoElapsed(ammo_elapsed);
}

//Alundaio
int CScriptGameObject::GetAmmoCount(u8 type)
{
	RMakeObj(CWeapon,weapon,0);
	if (type < weapon->m_ammoTypes.size())
		return weapon->GetAmmoCount_forType(weapon->m_ammoTypes[type]);

	return 0;
}

void CScriptGameObject::SetAmmoType(u8 type)
{
	MakeObj(CWeapon,weapon);
	weapon->SetAmmoType(type);
}

u8 CScriptGameObject::GetAmmoType()
{
	RMakeObj(CWeapon,weapon,255);
	return weapon->GetAmmoType();
}

void CScriptGameObject::SetMainWeaponType(u32 type)
{
	MakeObj(CWeapon,weapon);
	weapon->set_ef_main_weapon_type(type);
}

void CScriptGameObject::SetWeaponType(u32 type)
{
	MakeObj(CWeapon,weapon);
	weapon->set_ef_weapon_type(type);
}

u32 CScriptGameObject::GetMainWeaponType()
{
	RMakeObj(CWeapon,weapon,255);
	return weapon->ef_main_weapon_type();
}

u32 CScriptGameObject::GetWeaponType()
{
	RMakeObj(CWeapon,weapon,255);
	return weapon->ef_weapon_type();
}

bool CScriptGameObject::HasAmmoType(u8 type)
{
	RMakeObj(CWeapon,weapon,false);
	return type < weapon->m_ammoTypes.size();
}

u8 CScriptGameObject::GetWeaponSubstate()
{
	RMakeObj(CWeapon,weapon,255);
	return weapon->m_sub_state;
}

//-Alundaio

u32 CScriptGameObject::GetSuitableAmmoTotal() const
{
	RMakeObj(CWeapon,weapon,0);
	return			(weapon->GetSuitableAmmoTotal(true));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CScriptGameObject::SetQueueSize(u32 queue_size)
{
	MakeObj(CWeaponMagazined,weapon);
	weapon->SetQueueSize	(queue_size);
}

////////////////////////////////////////////////////////////////////////////
//// Inventory Owner
////////////////////////////////////////////////////////////////////////////

u32	CScriptGameObject::Cost			() const
{
	RMakeObj(CInventoryItem,inventory_item,0);
	return				(inventory_item->Cost());
}

float CScriptGameObject::GetCondition	() const
{
	RMakeObj(CInventoryItem,inventory_item,0.0f);
	return				(inventory_item->GetCondition());
}

void CScriptGameObject::SetCondition	(float val)
{
	MakeObj(CInventoryItem,inventory_item);
	val					-= inventory_item->GetCondition();
	inventory_item->ChangeCondition			(val);
}

float CScriptGameObject::GetPsyFactor() const
{
	RMakeObj(CPda,pda,0.0f);
	return (pda->m_psy_factor);
}

void CScriptGameObject::SetPsyFactor(float val)
{
	MakeObj(CPda,pda);
	pda->m_psy_factor = val;
}

void CScriptGameObject::eat				(CScriptGameObject *item)
{
	if(!item) {
		Msg("! CSciptEntity: cannot access class member eat");
		return;
	}
	MakeObj2(CInventoryItem,inventory_item,&item->object());
	MakeObj(CInventoryOwner,inventory_owner);
	inventory_owner->inventory().Eat(inventory_item);
}

bool CScriptGameObject::inside					(const Fvector &position, float epsilon) const
{
	RMakeObj(CSpaceRestrictor,space_restrictor,false);
	Fsphere				sphere;
	sphere.P			= position;
	sphere.R			= epsilon;
	return				(space_restrictor->inside(sphere));
}

bool CScriptGameObject::inside					(const Fvector &position) const
{
	return				(inside(position,EPS_L));
}

void CScriptGameObject::set_patrol_extrapolate_callback(const luabind::functor<bool> &functor)
{
	MakeObj(CCustomMonster,monster);
	monster->movement().patrol().extrapolate_callback().set(functor);
}

void CScriptGameObject::set_patrol_extrapolate_callback(const luabind::functor<bool> &functor, const luabind::object &object)
{
	MakeObj2(CCustomMonster,monster,&this->object());
	monster->movement().patrol().extrapolate_callback().set(functor,object);
}

void CScriptGameObject::set_patrol_extrapolate_callback()
{
	MakeObj(CCustomMonster,monster);
	monster->movement().patrol().extrapolate_callback().clear();
}

void CScriptGameObject::extrapolate_length		(float extrapolate_length)
{
	MakeObj(CCustomMonster,monster);
	monster->movement().detail().extrapolate_length(extrapolate_length);
}

float CScriptGameObject::extrapolate_length		() const
{
	RMakeObj(CCustomMonster,monster,0.f);
	return					(monster->movement().detail().extrapolate_length());
}

void CScriptGameObject::set_fov					(float new_fov)
{
	MakeObj(CCustomMonster,monster);
	monster->set_fov		(new_fov);
}

void CScriptGameObject::set_range				(float new_range)
{
	MakeObj(CCustomMonster,monster);
	monster->set_range		(new_range);
}

u32	CScriptGameObject::vertex_in_direction(u32 level_vertex_id, Fvector direction, float max_distance) const
{
	RMakeObj(CCustomMonster,monster,u32(-1));
	if (!monster->movement().restrictions().accessible(level_vertex_id)) {
		Msg("! CCustomMonster::vertex_in_direction - start vertex id is not accessible");
		return		(u32(-1));
	}

	direction.normalize_safe();
	direction.mul	(max_distance);
	Fvector			start_position = ai().level_graph().vertex_position(level_vertex_id);
	Fvector			finish_position = Fvector(start_position).add(direction);
	u32				result = u32(-1);
	monster->movement().restrictions().add_border(level_vertex_id,max_distance);
	ai().level_graph().farthest_vertex_in_direction(level_vertex_id,start_position,finish_position,result,0,true);
	monster->movement().restrictions().remove_border();
	return			(ai().level_graph().valid_vertex_id(result) ? result : level_vertex_id);
}

bool CScriptGameObject::invulnerable		() const
{
	RMakeObj(CCustomMonster,monster,false);
	return			(monster->invulnerable());
}

void CScriptGameObject::invulnerable		(bool invulnerable)
{
	MakeObj(CCustomMonster,monster);
	monster->invulnerable	(invulnerable);
}

LPCSTR CScriptGameObject::get_smart_cover_description	() const {
	RMakeObj(smart_cover::object,smart_cover_object,NULL);
	return smart_cover_object->cover().description()->table_id().c_str();
}

#include "stalker_animation_manager.h"
#include "CharacterPhysicsSupport.h"
#include "PhysicsShellHolder.h"

void CScriptGameObject::set_visual_name(LPCSTR visual, bool bForce)
{
	if (strcmp(visual, object().cNameVisual().c_str()) == 0)
		return;

	NET_Packet P;
	object().u_EventGen(P, GE_CHANGE_VISUAL, object().ID());
	P.w_stringZ(visual);
	object().u_EventSend(P);

	CActor* actor = smart_cast<CActor*>(&object());
	if (actor)
	{
		actor->ChangeVisual(visual);
		return;
	}

	CAI_Stalker* stalker = smart_cast<CAI_Stalker*>(&object());
	if (stalker)
	{
		stalker->ChangeVisual(visual);

		IKinematicsAnimated* V = smart_cast<IKinematicsAnimated*>(stalker->Visual());
		if (V)
		{
			if (!stalker->g_Alive())
				stalker->m_pPhysics_support->in_Die(false);
			else
				stalker->CStepManager::reload(stalker->cNameSect().c_str());

			stalker->CDamageManager::reload(*stalker->cNameSect(), "damage", pSettings);
			stalker->ResetBoneProtections(NULL, NULL);
			stalker->reattach_items();
			stalker->m_pPhysics_support->in_ChangeVisual();
			stalker->animation().reload();
		}

		return;
	}

	object().cNameVisual_set(visual);
	object().Visual()->dcast_PKinematics()->CalculateBones_Invalidate();
	object().Visual()->dcast_PKinematics()->CalculateBones(TRUE);
}

LPCSTR CScriptGameObject::get_visual_name				() const {
	return object().cNameVisual().c_str();
}

bool CScriptGameObject::addon_IsActorHideout() const
{
	static bool actorInhideout = true;
	static u32 last_ray_pick_time = Device.dwTimeGlobal;
	if (Device.dwTimeGlobal > (last_ray_pick_time + 500)) {
		last_ray_pick_time = Device.dwTimeGlobal;
		collide::rq_result RQ;
		actorInhideout = !!g_pGameLevel->ObjectSpace.RayPick(Device.vCameraPosition, Fvector().set(0, 1, 0), 50.f, collide::rqtBoth, RQ, g_pGameLevel->CurrentViewEntity());
	}

	return actorInhideout;
}

void CScriptGameObject::PhantomSetEnemy(CScriptGameObject* enemy)
{
	MakeObj(CPhantom,phant);
	phant->SetEnemy(&enemy->object());
}

//Allows to force use an object if passed obj is the actor
bool CScriptGameObject::Use(CScriptGameObject* obj)
{
	bool ret = object().use(&obj->object());

	CActor* actor = smart_cast<CActor*>(&obj->object());
	if (!actor)
		return ret;

	CInventoryOwner* pActorInv = smart_cast<CInventoryOwner*>(actor);
	if (!pActorInv)
		return ret;

	CUIActorMenu& ActorMenu = CurrentGameUI()->ActorMenu();

	CInventoryBox* pBox = smart_cast<CInventoryBox*>(&object());
	if (pBox)
	{
		ActorMenu.SetActor(pActorInv);
		ActorMenu.SetInvBox(pBox);

		ActorMenu.SetMenuMode(mmDeadBodySearch);
		ActorMenu.ShowDialog(true);

		return true;
	}
	else
	{
		CInventoryOwner* pOtherOwner = smart_cast<CInventoryOwner*>(&object());
		if (!pOtherOwner)
			return ret;

		/*
		CEntityAlive* e = smart_cast<CEntityAlive*>(pOtherOwner);
		if (e && e->g_Alive())
		{
			actor->RunTalkDialog(pOtherOwner, false);
			return true;
		}
		*/

		ActorMenu.SetActor(pActorInv);
		ActorMenu.SetPartner(pOtherOwner);

		ActorMenu.SetMenuMode(mmDeadBodySearch);
		ActorMenu.ShowDialog(true);

		return true;
	}

	return false;
}

void CScriptGameObject::StartTrade(CScriptGameObject* obj)
{
	MakeObj2(CActor,actor,&obj->object());
	MakeObj2(CInventoryOwner,pActorInv,actor);
	MakeObj(CInventoryOwner,pOtherOwner);

	CUIActorMenu& ActorMenu = CurrentGameUI()->ActorMenu();

	ActorMenu.SetActor(pActorInv);
	ActorMenu.SetPartner(pOtherOwner);
	ActorMenu.SetMenuMode(mmTrade);
	ActorMenu.ShowDialog(true);
}

void CScriptGameObject::StartUpgrade(CScriptGameObject* obj)
{
	MakeObj2(CActor,actor,&obj->object());
	MakeObj2(CInventoryOwner,pActorInv,actor);
	MakeObj(CInventoryOwner,pOtherOwner);

	CUIActorMenu& ActorMenu = CurrentGameUI()->ActorMenu();

	ActorMenu.SetActor(pActorInv);
	ActorMenu.SetPartner(pOtherOwner);
	ActorMenu.SetMenuMode(mmUpgrade);
	ActorMenu.ShowDialog(true);
}