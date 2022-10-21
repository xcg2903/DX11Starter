#pragma once
#include <DirectXMath.h>

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

struct Light
{
	int type; //0, 1, 2
	DirectX::XMFLOAT3 direction; // For directional/spot
	float range; //For spot/point
	DirectX::XMFLOAT3 position; // For spot/point
	float intensity; //How bright
	DirectX::XMFLOAT3 color; //The RGB value of the light
	float spotFalloff; // cone size for spot lights
	DirectX::XMFLOAT3 padding; // padding to hit the 16-byte boundar
};