#include "peeeek_particles.h"

#include <base/math.h>
#include <base/time.h>
#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <game/client/gameclient.h>
#include <game/collision.h>

static IGraphics::CTextureHandle s_aPeeeekTextures[CPeeeekParticles::NUM_TEXTURES];

CPeeeekParticles::CPeeeekParticles()
{
	m_FirstFree = 0;
	m_LastRenderTime = time_get();
	m_WorldParticleSpawnTimer = 0.0f;
	OnReset();
}

void CPeeeekParticles::OnInit()
{
	const char *aTextureNames[NUM_TEXTURES] = {
		"peeeek/particles/bucks1.png",
		"peeeek/particles/core1.png",
		"peeeek/particles/crest1.png",
		"peeeek/particles/cube1.png",
		"peeeek/particles/cubeblast1.png",
		"peeeek/particles/ded1.png",
		"peeeek/particles/heart1.png",
		"peeeek/particles/show1.png",
		"peeeek/particles/snowbag1.png",
		"peeeek/particles/snowblast1.png",
		"peeeek/particles/snowbrich1.png",
		"peeeek/particles/snownew1.png",
		"peeeek/particles/star1.png"
	};

	for(int i = 0; i < NUM_TEXTURES; i++)
	{
		s_aPeeeekTextures[i] = Graphics()->LoadTexture(aTextureNames[i], IStorage::TYPE_ALL);
	}
}

void CPeeeekParticles::OnReset()
{
	for(int i = 0; i < MAX_PARTICLES; i++)
	{
		m_aParticles[i].m_Active = false;
	}
	m_FirstFree = 0;
}

bool CPeeeekParticles::ParticleIsVisibleOnScreen(const vec2 &CurPos, float CurSize)
{
	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);

	// Expand screen by particle size + some margin
	float Margin = CurSize * 2.0f + 100.0f;

	if(CurPos.x < ScreenX0 - Margin || CurPos.x > ScreenX1 + Margin ||
		CurPos.y < ScreenY0 - Margin || CurPos.y > ScreenY1 + Margin)
	{
		return false;
	}

	return true;
}

void CPeeeekParticles::AddHitParticle(vec2 Pos)
{
	if(!g_Config.m_ClPeeeekHitParticles)
		return;

	int Count = std::clamp(g_Config.m_ClPeeeekHitParticlesCount, 1, 30);
	int TexIndex = std::clamp(g_Config.m_ClPeeeekHitParticlesTexture - 1, 0, NUM_TEXTURES - 1);
	float BaseSize = (float)g_Config.m_ClPeeeekHitParticlesSize;
	ColorRGBA Color = color_cast<ColorRGBA>(ColorHSLA(g_Config.m_ClPeeeekHitParticlesColor, true));

	for(int i = 0; i < Count; i++)
	{
		if(m_FirstFree >= MAX_PARTICLES)
			break; // no free slots

		CPeeeekParticle *p = &m_aParticles[m_FirstFree];
		p->SetDefault();
		p->m_Active = true;
		p->m_PhysicsMode = PHYSICS_BOUNCE; // Hit particles always bounce
		
		p->m_TexIndex = TexIndex;
		p->m_Pos = Pos;
		p->m_Vel = random_direction() * random_float(300.0f, 800.0f);
		p->m_LifeSpan = random_float(0.5f, 1.5f);
		p->m_StartSize = BaseSize * random_float(0.8f, 1.2f);
		p->m_EndSize = 0.0f;
		p->m_Rot = random_angle();
		p->m_Rotspeed = random_float(-pi, pi);
		p->m_Gravity = 800.0f;
		p->m_Friction = 0.7f;
		p->m_Color = Color;
		p->m_StartAlpha = 1.0f;
		p->m_EndAlpha = 0.0f;

		// find next free slot
		for(int j = m_FirstFree + 1; j < MAX_PARTICLES; j++)
		{
			if(!m_aParticles[j].m_Active)
			{
				m_FirstFree = j;
				break;
			}
		}
		if(m_aParticles[m_FirstFree].m_Active)
			m_FirstFree = MAX_PARTICLES;
	}
}

void CPeeeekParticles::AddWorldParticle(vec2 Pos)
{
	if(m_FirstFree >= MAX_PARTICLES)
		return; // no free slots

	int TexIndex = std::clamp(g_Config.m_ClPeeeekWorldParticlesTexture - 1, 0, NUM_TEXTURES - 1);
	float BaseSize = (float)g_Config.m_ClPeeeekWorldParticlesSize;
	ColorRGBA Color = color_cast<ColorRGBA>(ColorHSLA(g_Config.m_ClPeeeekWorldParticlesColor, true));
	int PhysicsMode = std::clamp(g_Config.m_ClPeeeekWorldParticlesPhysics, 0, 2);
	float WindStrength = (float)g_Config.m_ClPeeeekWorldParticlesWindStrength;

	CPeeeekParticle *p = &m_aParticles[m_FirstFree];
	p->SetDefault();
	p->m_Active = true;
	p->m_PhysicsMode = PhysicsMode;
	
	p->m_TexIndex = TexIndex;
	p->m_Pos = Pos;
	p->m_LifeSpan = random_float(2.0f, 5.0f);
	p->m_StartSize = BaseSize * random_float(0.8f, 1.2f);
	p->m_EndSize = p->m_StartSize; // World particles don't shrink by default
	p->m_Rot = random_angle();
	p->m_Color = Color;
	p->m_StartAlpha = 0.0f; // Fade in
	p->m_EndAlpha = 0.0f;   // Fade out

	if(PhysicsMode == PHYSICS_BOUNCE)
	{
		p->m_Vel = random_direction() * random_float(100.0f, 300.0f);
		p->m_Rotspeed = random_float(-pi, pi);
		p->m_Gravity = 500.0f;
		p->m_Friction = 0.8f;
	}
	else if(PhysicsMode == PHYSICS_WIND)
	{
		p->m_Vel = vec2(WindStrength * 10.0f, random_float(-50.0f, 50.0f));
		p->m_Rotspeed = random_float(-pi/2, pi/2);
		p->m_Gravity = 0.0f;
		p->m_Friction = 1.0f; // no friction
	}
	else if(PhysicsMode == PHYSICS_LEVITATION)
	{
		p->m_Vel = vec2(random_float(-20.0f, 20.0f), random_float(-50.0f, -100.0f));
		p->m_Rotspeed = random_float(-pi/4, pi/4);
		p->m_Gravity = 0.0f;
		p->m_Friction = 1.0f;
	}

	// find next free slot
	for(int j = m_FirstFree + 1; j < MAX_PARTICLES; j++)
	{
		if(!m_aParticles[j].m_Active)
		{
			m_FirstFree = j;
			break;
		}
	}
	if(m_aParticles[m_FirstFree].m_Active)
		m_FirstFree = MAX_PARTICLES;
}

void CPeeeekParticles::OnRender()
{
	int64_t Now = time_get();
	float TimePassed = (Now - m_LastRenderTime) / (float)time_freq();
	m_LastRenderTime = Now;

	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
		if(pInfo->m_Paused)
			TimePassed = 0.0f;
		else
			TimePassed *= pInfo->m_Speed;
	}

	if(TimePassed <= 0.0f)
		TimePassed = 0.0001f; // Prevent division by zero

	// Handle World Particle Spawning
	if(g_Config.m_ClPeeeekWorldParticles)
	{
		int TargetCount = std::clamp(g_Config.m_ClPeeeekWorldParticlesCount, 1, 100);
		
		// Count current active world particles
		int CurrentWorldCount = 0;
		for(int i = 0; i < MAX_PARTICLES; i++)
		{
			if(m_aParticles[i].m_Active && m_aParticles[i].m_LifeSpan > 1.5f && m_aParticles[i].m_PhysicsMode == g_Config.m_ClPeeeekWorldParticlesPhysics) // Heuristic to identify world particles
				CurrentWorldCount++;
		}

		if(CurrentWorldCount < TargetCount)
		{
			m_WorldParticleSpawnTimer += TimePassed;
			float SpawnInterval = 0.1f; // max 10 per second
			if(m_WorldParticleSpawnTimer > SpawnInterval)
			{
				m_WorldParticleSpawnTimer = 0.0f;
				
				float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
				Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
				
				// Spawn somewhat randomly around the screen
				vec2 SpawnPos;
				int PhysicsMode = std::clamp(g_Config.m_ClPeeeekWorldParticlesPhysics, 0, 2);
				if(PhysicsMode == PHYSICS_WIND)
				{
					// Spawn on the left edge if wind blows right, or random if we just want them everywhere
					SpawnPos = vec2(ScreenX0 - 50.0f, random_float(ScreenY0, ScreenY1));
				}
				else if(PhysicsMode == PHYSICS_LEVITATION)
				{
					// Spawn at the bottom
					SpawnPos = vec2(random_float(ScreenX0, ScreenX1), ScreenY1 + 50.0f);
				}
				else
				{
					// Random
					SpawnPos = vec2(random_float(ScreenX0, ScreenX1), random_float(ScreenY0, ScreenY1));
				}

				AddWorldParticle(SpawnPos);
			}
		}
	}

	// Setup rendering
	Graphics()->BlendNormal();
	
	// We might have to switch textures, so we sort or just bind per particle.
	// 512 max particles with a few texture binds is fine for modern GPUs, but let's minimize binds.
	for(int TexIdx = 0; TexIdx < NUM_TEXTURES; TexIdx++)
	{
		if(!s_aPeeeekTextures[TexIdx].IsValid())
			continue;

		bool Begun = false;

		for(int i = 0; i < MAX_PARTICLES; i++)
		{
			CPeeeekParticle *p = &m_aParticles[i];
			if(!p->m_Active || p->m_TexIndex != TexIdx)
				continue;

			// Update life
			p->m_Life += TimePassed;
			if(p->m_Life >= p->m_LifeSpan)
			{
				p->m_Active = false;
				if(i < m_FirstFree)
					m_FirstFree = i;
				continue;
			}

			// Calculate progress 0.0 to 1.0
			float Progress = p->m_Life / p->m_LifeSpan;

			// Update physics
			p->m_Vel.y += p->m_Gravity * TimePassed;
			
			// Move
			vec2 OldPos = p->m_Pos;
			p->m_Pos += p->m_Vel * TimePassed;

			// Bounce collision
			if(p->m_PhysicsMode == PHYSICS_BOUNCE)
			{
				vec2 HitPos;
				if(Collision()->IntersectLine(OldPos, p->m_Pos, &HitPos, nullptr))
				{
					// Simple bounce
					p->m_Pos = HitPos;
					p->m_Vel *= -p->m_Friction;
					
					// Stop entirely if too slow
					if(length(p->m_Vel) < 10.0f)
						p->m_Vel = vec2(0, 0);
				}
			}

			// Apply friction
			if(p->m_PhysicsMode != PHYSICS_WIND && p->m_PhysicsMode != PHYSICS_LEVITATION)
			{
				p->m_Vel *= std::pow(p->m_Friction, TimePassed * 10.0f);
			}

			// Rotate
			p->m_Rot += p->m_Rotspeed * TimePassed;

			// Culling
			float CurSize = mix(p->m_StartSize, p->m_EndSize, Progress);
			if(!ParticleIsVisibleOnScreen(p->m_Pos, CurSize))
			{
				// Delete world particles that fly off screen
				p->m_Active = false;
				if(i < m_FirstFree)
					m_FirstFree = i;
				continue;
			}

			// Render
			if(!Begun)
			{
				Graphics()->TextureSet(s_aPeeeekTextures[TexIdx]);
				Graphics()->QuadsBegin();
				Begun = true;
			}

			// Calculate Alpha (fade in/out for world particles, fade out for hits)
			float Alpha = p->m_Color.a;
			if(p->m_PhysicsMode == PHYSICS_WIND || p->m_PhysicsMode == PHYSICS_LEVITATION || p->m_PhysicsMode == PHYSICS_BOUNCE)
			{
				if(Progress < 0.1f)
					Alpha *= (Progress / 0.1f); // fade in
				else if(Progress > 0.8f)
					Alpha *= (1.0f - Progress) / 0.2f; // fade out
			}
			else
			{
				Alpha *= mix(p->m_StartAlpha, p->m_EndAlpha, Progress);
			}

			Graphics()->QuadsSetRotation(p->m_Rot);
			Graphics()->SetColor(p->m_Color.r * Alpha, p->m_Color.g * Alpha, p->m_Color.b * Alpha, Alpha);

			IGraphics::CQuadItem QuadItem(p->m_Pos.x, p->m_Pos.y, CurSize, CurSize);
			Graphics()->QuadsDraw(&QuadItem, 1);
		}

		if(Begun)
		{
			Graphics()->QuadsEnd();
		}
	}
	Graphics()->QuadsSetRotation(0.0f);
}
