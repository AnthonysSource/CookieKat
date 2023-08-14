#pragma once

namespace CKE::ECS
{
	struct PositionComponent
	{
		f32 x;
		f32 y;
		f32 z;

		PositionComponent() = default;

		PositionComponent(f32 x, f32 y, f32 z)
			: x{ x },
			y{ y },
			z{ z } {}
	};

	struct AccelerationComponent
	{
		f32 x;
		f32 y;
		f32 z;

		AccelerationComponent() = default;

		AccelerationComponent(f32 x, f32 y, f32 z)
			: x{ x },
			y{ y },
			z{ z } {}
	};

	struct VelocityComponent
	{
		f32 x;
		f32 y;
		f32 z;

		VelocityComponent() = default;

		VelocityComponent(f32 x, f32 y, f32 z)
			: x{ x },
			y{ y },
			z{ z } {}
	};
}