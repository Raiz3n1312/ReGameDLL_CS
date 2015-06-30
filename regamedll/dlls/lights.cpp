#include "precompiled.h"

/*
* Globals initialization
*/
#ifndef HOOK_GAMEDLL

TYPEDESCRIPTION CLight::m_SaveData[] =
{
	DEFINE_FIELD(CLight, m_iStyle, FIELD_INTEGER),
	DEFINE_FIELD(CLight, m_iszPattern, FIELD_STRING),
};

#else

TYPEDESCRIPTION CLight::m_SaveData2[] =
{
	DEFINE_FIELD(CLight, m_iStyle, FIELD_INTEGER),
	DEFINE_FIELD(CLight, m_iszPattern, FIELD_STRING),
};

TYPEDESCRIPTION (*CLight::m_SaveData)[2];

#endif // HOOK_GAMEDLL

/* <e7ded> ../cstrike/dlls/lights.cpp:48 */
LINK_ENTITY_TO_CLASS(light, CLight);

/* <e7b2c> ../cstrike/dlls/lights.cpp:56 */
IMPLEMENT_SAVERESTORE(CLight, CPointEntity);

// Cache user-entity-field values until spawn is called.

/* <e7bef> ../cstrike/dlls/lights.cpp:62 */
void CLight::KeyValue_(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "style"))
	{
		m_iStyle = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pitch"))
	{
		pev->angles.x = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pattern"))
	{
		m_iszPattern = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue( pkvd );
}

/* <e7b78> ../cstrike/dlls/lights.cpp:92 */
void CLight::Spawn_(void)
{
	// inert light
	if (FStringNull(pev->targetname))
	{
		REMOVE_ENTITY(ENT(pev));
		return;
	}
	
	m_iStartedOff = (pev->spawnflags & SF_LIGHT_START_OFF) != 0;

	if (m_iStyle >= 32)
	{
		if (pev->spawnflags & SF_LIGHT_START_OFF)
			LIGHT_STYLE(m_iStyle, "a");

		else if (m_iszPattern)
			LIGHT_STYLE(m_iStyle, (char *)STRING( m_iszPattern ));
		else
			LIGHT_STYLE(m_iStyle, "m");
	}
}

/* <e7a30> ../cstrike/dlls/lights.cpp:117 */
void CLight::Restart_(void)
{
	if (m_iStyle >= 32)
	{
		if (m_iStartedOff)
		{
			pev->spawnflags |= SF_LIGHT_START_OFF;
			LIGHT_STYLE(m_iStyle, "a");
		}
		else
		{
			pev->spawnflags &= ~SF_LIGHT_START_OFF;

			if (m_iszPattern)
				LIGHT_STYLE(m_iStyle, (char *)STRING(m_iszPattern));
			else
				LIGHT_STYLE(m_iStyle, "m");
		}
	}
}

/* <e7a6b> ../cstrike/dlls/lights.cpp:139 */
void CLight::Use_(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (m_iStyle >= 32)
	{
		if (!ShouldToggle(useType, !(pev->spawnflags & SF_LIGHT_START_OFF)))
			return;

		if (pev->spawnflags & SF_LIGHT_START_OFF)
		{
			if (m_iszPattern)
				LIGHT_STYLE(m_iStyle, (char *)STRING(m_iszPattern));
			else
				LIGHT_STYLE(m_iStyle, "m");

			pev->spawnflags &= ~SF_LIGHT_START_OFF;
		}
		else
		{
			LIGHT_STYLE(m_iStyle, "a");
			pev->spawnflags |= SF_LIGHT_START_OFF;
		}
	}
}

/* <e7eb7> ../cstrike/dlls/lights.cpp:165 */
LINK_ENTITY_TO_CLASS(light_spot, CLight);

/* <e7f81> ../cstrike/dlls/lights.cpp:175 */
LINK_ENTITY_TO_CLASS(light_environment, CEnvLight);

/* <e7d08> ../cstrike/dlls/lights.cpp:177 */
void CEnvLight::KeyValue_(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "_light"))
	{
		int r, g, b, v, j;
		j = sscanf(pkvd->szValue, "%d %d %d %d\n", &r, &g, &b, &v);

		if (j == 1)
			g = b = r;

		else if (j == 4)
		{
			r = r * (v / 255.0);
			g = g * (v / 255.0);
			b = b * (v / 255.0);
		}

		// simulate qrad direct, ambient,and gamma adjustments, as well as engine scaling
		r = pow( r / 114.0, 0.6 ) * 264;
		g = pow( g / 114.0, 0.6 ) * 264;
		b = pow( b / 114.0, 0.6 ) * 264;

		//r = (int)(powf(r / 114.0, 0.6) * 264);
		//g = (int)(powf(g / 114.0, 0.6) * 264);
		//b = (int)(powf(b / 114.0, 0.6) * 264);
		pkvd->fHandled = TRUE;

		char szColor[64];
		Q_sprintf(szColor, "%d", r);
		CVAR_SET_STRING("sv_skycolor_r", szColor);
		Q_sprintf(szColor, "%d", g);
		CVAR_SET_STRING("sv_skycolor_g", szColor);
		Q_sprintf(szColor, "%d", b);
		CVAR_SET_STRING("sv_skycolor_b", szColor);
	}
	else
		CLight::KeyValue(pkvd);
}

/* <e7bb3> ../cstrike/dlls/lights.cpp:215 */
void CEnvLight::Spawn_(void)
{
	char szVector[64];
	UTIL_MakeAimVectors( pev->angles );

	Q_sprintf(szVector, "%f", gpGlobals->v_forward.x);
	CVAR_SET_STRING("sv_skyvec_x", szVector);
	Q_sprintf(szVector, "%f", gpGlobals->v_forward.y);
	CVAR_SET_STRING("sv_skyvec_y", szVector);
	Q_sprintf(szVector, "%f", gpGlobals->v_forward.z);
	CVAR_SET_STRING("sv_skyvec_z", szVector);

	CLight::Spawn();
}

#ifdef HOOK_GAMEDLL

void CLight::Spawn(void)
{
	Spawn_();
}

void CLight::Restart(void)
{
	Restart_();
}

int CLight::Save(CSave &save)
{
	return Save_(save);
}

int CLight::Restore(CRestore &restore)
{
	return Restore_(restore);
}

void CLight::KeyValue(KeyValueData *pkvd)
{
	KeyValue_(pkvd);
}

void CLight::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	Use_(pActivator, pCaller, useType, value);
}

void CEnvLight::Spawn(void)
{
	Spawn_();
}

void CEnvLight::KeyValue(KeyValueData *pkvd)
{
	KeyValue_(pkvd);
}

#endif // HOOK_GAMEDLL