#include "geometric_primitive.hlsli"

//cbuffer TestBuffer : register(b10)
//{
//    int t = 0;
//};

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = pin.color;
	
    //color.rgb *= t * 0.1f; // t‚Ì’l‚É‰‚¶‚ÄF‚ğ•Ï‰»‚³‚¹‚é—á
    
	return color;
}