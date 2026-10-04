#pragma once
#include <cmath>
constexpr float EASE_PI = 3.1415927f;

float EaseTable(int type, float x)
{
	static constexpr float (*Table[])(float) =
	{
		[](float x) -> float { return x; },
		[](float x) -> float { return std::pow(x, 2); },
		[](float x) -> float { return 1 - std::pow(1 - x, 2); },
		[](float x) -> float { return x < 0.5 ? std::pow(2 * x,2) / 2 : 1 - pow(2 - 2 * x,2) / 2; },
		[](float x) -> float { return std::pow(x, 3); },
		[](float x) -> float { return 1 - std::pow(1 - x, 3); },
		[](float x) -> float { return x < 0.5 ? std::pow(2 * x,3) / 2 : 1 - pow(2 - 2 * x,3) / 2; },
		[](float x) -> float { return std::pow(x, 4); },
		[](float x) -> float { return 1 - std::pow(1 - x, 4); },
		[](float x) -> float { return x < 0.5 ? std::pow(2 * x,4) / 2 : 1 - pow(2 - 2 * x,4) / 2; },
		[](float x) -> float { return std::pow(x, 5); },
		[](float x) -> float { return 1 - std::pow(1 - x, 5); },
		[](float x) -> float { return x < 0.5 ? std::pow(2 * x,5) / 2 : 1 - pow(2 - 2 * x,5) / 2; },
		[](float x) -> float { return 0; },
		[](float x) -> float { return 1; },
		[](float x) -> float { return 1 - std::sqrt(1 - pow(x,2)); },
		[](float x) -> float { return std::sqrt(1 - pow(1 - x,2)); },
		[](float x) -> float { return std::sin(EASE_PI * x / 2); },
		[](float x) -> float { return 1 - std::cos(EASE_PI * x / 2); }
	};
	return Table[type](x);
}