/*
* Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef BULLET_TEST_H
#define BULLET_TEST_H

class BulletTest : public Test
{
  public:
	BulletTest()
	{
		{
			b2BodyDef bd;

			b2PolygonShape box;

			bd.position.Set(0.0f, 4.0f);
			bd.angle = 0.0;
			box.SetAsBox(4.0f, 0.3f);
			m_body1 = m_world->CreateBody(&bd);
			auto fix = m_body1->CreateFixture(&box, 1.0f);
			fix->SetRestitution(0.0);

			bd.position.Set(0.0f, 8.0f);
			bd.angle = 0.0;
			box.SetAsBox(3.0f, 0.2f);
			m_body2 = m_world->CreateBody(&bd);
			fix = m_body2->CreateFixture(&box, 1.0f);
			fix->SetRestitution(0.0);

			bd.position.Set(0.0f, 12.0f);
			bd.angle = 0.0;
			box.SetAsBox(2.0f, 0.1f);
			m_body3 = m_world->CreateBody(&bd);
			fix = m_body3->CreateFixture(&box, 1.0f);
			fix->SetRestitution(0.0);

			bd.position.Set(0.0f, 16.0f);
			bd.angle = 0.0;
			box.SetAsBox(1.0f, 0.1f);
			m_body4 = m_world->CreateBody(&bd);
			fix = m_body4->CreateFixture(&box, 1.0f);
			fix->SetRestitution(0.0);
		}
		{
			b2BodyDef bd;
			bd.type = b2_dynamicBody;
			bd.position.Set(0.0f, 4.0f);

			b2PolygonShape box;

			box.SetAsBox(0.25f, 0.25f);
			b2CircleShape shape;
			shape.m_radius = 0.25f;
			//m_x = RandomFloat(-1.0f, 1.0f);
			m_x = 0.20352793f;
			bd.position.Set(m_x, 10.0f);
			bd.bullet = true;

			m_bullet = m_world->CreateBody(&bd);
			auto fix = m_bullet->CreateFixture(&shape, 100.0f);
			fix->SetRestitution(0.0);

			m_bullet->SetLinearVelocity(b2Vec2(0.0f, -1.0f));
		}
	}

	void Launch()
	{
		printf("LAUNCH CALLED\n");
		//m_body->SetTransform(b2Vec2(0.0f, 4.0f), 0.0f);
		//m_body->SetLinearVelocity(b2Vec2_zero);
		//m_body->SetAngularVelocity(0.0f);

		extern int32 b2_gjkCalls, b2_gjkIters, b2_gjkMaxIters;
		extern int32 b2_toiCalls, b2_toiIters, b2_toiMaxIters;
		extern int32 b2_toiRootIters, b2_toiMaxRootIters;

		b2_gjkCalls = 0;
		b2_gjkIters = 0;
		b2_gjkMaxIters = 0;

		b2_toiCalls = 0;
		b2_toiIters = 0;
		b2_toiMaxIters = 0;
		b2_toiRootIters = 0;
		b2_toiMaxRootIters = 0;
	}

	void Setup(Settings *settings)
	{
		m_body1->SetTransform(settings->o1, settings->r1);
		m_body1->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
		m_body1->SetAngularVelocity(0.0f);

		m_body2->SetTransform(settings->o2, settings->r2);
		m_body2->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
		m_body2->SetAngularVelocity(0.0f);

		m_body3->SetTransform(settings->o3, settings->r3);
		m_body3->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
		m_body3->SetAngularVelocity(0.0f);

		m_body4->SetTransform(settings->o4, settings->r4);
		m_body4->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
		m_body4->SetAngularVelocity(0.0f);

		m_bullet->SetTransform(b2Vec2(-30.0f, 40.0f), 0.0f);
		m_bullet->SetLinearVelocity(b2Vec2(0.0f, -1.0f));
		m_bullet->SetAngularVelocity(0.0f);
	}
	void Step(Settings *settings)
	{
		Test::Step(settings);
		auto p = m_bullet->GetPosition();
		auto v = m_bullet->GetLinearVelocity();

		settings->p1 = p;
		settings->v1 = v;
		if (settings->doGUI)
			printf("%f %f %f %f\n", p.x, p.y, v.x, v.y);
		extern int32 b2_gjkCalls, b2_gjkIters, b2_gjkMaxIters;
		extern int32 b2_toiCalls, b2_toiIters;
		extern int32 b2_toiRootIters, b2_toiMaxRootIters;
		if (settings->doGUI)
		{
			if (b2_gjkCalls > 0)
			{
				g_debugDraw.DrawString(5, m_textLine, "gjk calls = %d, ave gjk iters = %3.1f, max gjk iters = %d",
									   b2_gjkCalls, b2_gjkIters / float32(b2_gjkCalls), b2_gjkMaxIters);
				m_textLine += DRAW_STRING_NEW_LINE;
			}

			if (b2_toiCalls > 0)
			{
				g_debugDraw.DrawString(5, m_textLine, "toi calls = %d, ave toi iters = %3.1f, max toi iters = %d",
									   b2_toiCalls, b2_toiIters / float32(b2_toiCalls), b2_toiMaxRootIters);
				m_textLine += DRAW_STRING_NEW_LINE;

				g_debugDraw.DrawString(5, m_textLine, "ave toi root iters = %3.1f, max toi root iters = %d",
									   b2_toiRootIters / float32(b2_toiCalls), b2_toiMaxRootIters);
				m_textLine += DRAW_STRING_NEW_LINE;
			}
		}
	}

	static Test *Create()
	{
		return new BulletTest;
	}

	b2Body *m_body1, *m_body2, *m_body3, *m_body4;
	b2Body *m_bullet;
	float32 m_x;
};

#endif
