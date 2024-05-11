#include "LightHelper.hlsli"

Texture2D g_DiffuseMap : register(t0);          // ��������
Texture2D g_DisplacementMap : register(t1);     // λ����ͼ
SamplerState g_SamLinearWrap : register(s0);    // ���Թ���+Wrap������
SamplerState g_SamPointClamp : register(s1);    // �����+Clamp������

cbuffer CBChangesEveryInstanceDrawing : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose;
    matrix g_TexTransform;
}

cbuffer CBChangesEveryObjectDrawing : register(b1)
{
    Material g_Material;
}

cbuffer CBChangesEveryFrame : register(b2)
{
    matrix g_View;
    float3 g_EyePosW;
    float g_Pad;
}

cbuffer CBDrawingStates : register(b3)
{
    float4 g_FogColor;
    int g_FogEnabled;
    float g_FogStart;
    float g_FogRange;
    int g_TextureUsed;
    
    int g_WavesEnabled;                     // �������˻���
    float2 g_DisplacementMapTexelSize;      // λ����ͼ�����������ض�Ӧ����֮���x,y������
    float g_GridSpatialStep;                // դ��ռ䲽��
}

cbuffer CBChangesOnResize : register(b4)
{
    matrix g_Proj;
}

cbuffer CBChangesRarely : register(b5)
{
    DirectionalLight g_DirLight[5];
    PointLight g_PointLight[5];
    SpotLight g_SpotLight[5];
}

struct VertexPosNormalTex
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
};

struct InstancePosNormalTex
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
    matrix World : World;
    matrix WorldInvTranspose : WorldInvTranspose;
};

struct VertexPosHWNormalTex
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION; // �������е�λ��
    float3 NormalW : NORMAL; // �������������еķ���
    float2 Tex : TEXCOORD;
};




