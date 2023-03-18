﻿#pragma once
#include "UIWindow.h"
#include "../EntityCondition.h"

class CUIXml;
class CUIStatic;
class CUITextWnd;
class UIBoosterInfoItem;
class CInventoryItem;

class CUIBoosterInfo : public CUIWindow
{
public:
	CUIBoosterInfo();
	virtual			~CUIBoosterInfo();
	void	InitFromXml(CUIXml& xml);
	void	SetInfo(CInventoryItem& pInvItem);

protected:
	enum {
		_item_start = 0,
		_item_satiety_restore_speed = _item_start,
		_item_health_restore_speed,
		_item_radiation_restore_speed,
		_item_power_restore_speed,
		_item_bleeding_restore_speed,

		//M.F.S Team additions
		_item_battery,
		_item_thirst,
		_item_psy_health,
		_item_intoxication,
		_item_sleepeness,

		//HoP
		_item_alcoholism,
		_item_hangover,
		_item_narcotism,
		_item_withdrawal,

		eBoostExplImmunity
	};
	UIBoosterInfoItem* m_booster_items[eBoostExplImmunity];
	UIBoosterInfoItem* m_portions;
	UIBoosterInfoItem* m_filter;
	UIBoosterInfoItem* m_repair_kit_condition;

	CUIStatic* m_Prop_line;

}; // class CUIBoosterInfo

// -----------------------------------

class UIBoosterInfoItem : public CUIWindow
{
public:
	UIBoosterInfoItem();
	virtual		~UIBoosterInfoItem();

	void	Init(CUIXml& xml, LPCSTR section);
	void	SetCaption(LPCSTR name);
	void	SetValue(float value, int vle = 0, float max_val = 0);
	CUIStatic* m_value;

private:
	CUIStatic*	m_caption;
	float		m_magnitude;
	bool		m_show_sign;
	shared_str	m_unit_str;
	shared_str	m_unit_str_max;
	shared_str	m_texture;

	//Color
	u32			m_negative_color;
	u32			m_neutral_color;
	u32			m_positive_color;
	bool		clr_invert;
	bool		use_color;
	bool		clr_dynamic;

}; // class UIBoosterInfoItem