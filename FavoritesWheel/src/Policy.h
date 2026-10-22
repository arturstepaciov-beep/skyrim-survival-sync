#pragma once
#include <algorithm>
#include <cmath>
#include <numbers>
namespace WheelPolicy {
inline constexpr int Slots = 10;
inline int Wrap(int page, int count) { return count > 0 ? (page % count + count) % count : 0; }
inline int Pages(int count) { return std::max(1, (count + Slots - 1) / Slots); }
inline int Hit(float x, float y, float deadzone, int count) {
 if (count <= 0 || !std::isfinite(x) || !std::isfinite(y) || x*x+y*y < deadzone*deadzone) return -1;
 const float tau = 2.f * std::numbers::pi_v<float>;
 float a = std::atan2(y,x) + std::numbers::pi_v<float>/2.f;
 a = std::fmod(a + tau + tau/(2.f*count), tau);
 return std::min(count-1, static_cast<int>(a/tau*count));
}
}
