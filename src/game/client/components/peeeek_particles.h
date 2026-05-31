#ifndef GAME_CLIENT_COMPONENTS_PEEEEK_PARTICLES_H
#define GAME_CLIENT_COMPONENTS_PEEEEK_PARTICLES_H

#include <base/color.h>
#include <base/vmath.h>
#include <game/client/component.h>

struct CPeeeekParticle
{
	void SetDefault()
	{
		m_Pos = vec2(0, 0);
		m_Vel = vec2(0, 0);
		m_LifeSpan = 0;
		m_Life = 0;
		m_StartSize = 32;
		m_EndSize = 32;
		m_StartAlpha = 1;
		m_EndAlpha = 1;
		m_Rot = 0;
		m_Rotspeed = 0;
		m_Gravity = 0;
		m_Friction = 0;
		m_Color = ColorRGBA(1, 1, 1, 1);
		m_TexIndex = 0;
		m_PhysicsMode = 0; // 0=bounce, 1=wind, 2=levitation
		m_Active = false;
	}

	vec2 m_Pos;
	vec2 m_Vel;
	float m_LifeSpan;
	float m_Life;
	float m_StartSize;
	float m_EndSize;
	float m_StartAlpha;
	float m_EndAlpha;
	float m_Rot;
	float m_Rotspeed;
	float m_Gravity;
	float m_Friction;
	ColorRGBA m_Color;
	int m_TexIndex;
	int m_PhysicsMode;

	bool m_Active;
};

class CPeeeekParticles : public CComponent
{
public:
	enum
	{
		MAX_PARTICLES = 512,
		NUM_TEXTURES = 13,
		
		PHYSICS_BOUNCE = 0,
		PHYSICS_WIND = 1,
		PHYSICS_LEVITATION = 2,
	};

	CPeeeekParticles();
	int Sizeof() const override { return sizeof(*this); }

	void OnInit() override;
	void OnRender() override;
	void OnReset() override;

	void AddHitParticle(vec2 Pos);
	void AddWorldParticle(vec2 Pos);

private:
	CPeeeekParticle m_aParticles[MAX_PARTICLES];

	int m_FirstFree;
	int64_t m_LastRenderTime;
	float m_WorldParticleSpawnTimer;

	bool ParticleIsVisibleOnScreen(const vec2 &CurPos, float CurSize);
};

#endif
