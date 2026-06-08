#include "geometric_primitive.hlsli"
#include "Lights.hlsli"
#include "ShaderFunctions.hlsli"

VS_OUT main( float3 position : POSITION, float3 normal : NORMAL)
{
    VS_OUT vout;
    vout.position = mul(float4(position, 1.0), mul(world, viewProjection));
    float3 worldPosition = mul(float4(position, 1.0), world).xyz;
    float3 worldNormal = normalize(mul(float4(normal, 0.0), world).xyz);
    
    float3 N = worldNormal;
    float3 L = normalize(-directionalLightDirection.xyz);
    float3 C = directionalLightColor.rgb;
    float3 K = materialColor.rgb;
    
    const float3 V = normalize(cameraPositon.xyz - worldPosition.xyz); // 視線ベクトル
    
    //vout.color.rgb = materialColor.rgb * max(0, LightColor * dot(L, N));
    //vout.color.rgb = CalcLambert(N.xyz, L.xyz, C, K);
    vout.color.rgb = ClacHalfLambert(N, -L, C, K);
    
    // DirectionalLightの適用
    
    // PointLightの適用
    float3 pointDiffuse = float3(0, 0, 0);
    float3 pointSpecular = float3(0, 0, 0);
    CalcPointLights(worldPosition, worldNormal, V, pointDiffuse, pointSpecular);
    vout.color.rgb += pointDiffuse * K;
    
    // SpotLightの適用
    float3 spotDiffuse = float3(0, 0, 0);
    float3 spotSpecular = float3(0, 0, 0);
    CalcSpotLights(worldPosition, spotDiffuse, spotSpecular);
    vout.color.rgb += spotDiffuse * K;
    
    //vout.color.rgb = worldNormal * 0.5f + 0.5f; // 法線を色として出力（デバッグ用）
    // アルファ値は定数
    vout.color.a = materialColor.a;
	return vout;
}