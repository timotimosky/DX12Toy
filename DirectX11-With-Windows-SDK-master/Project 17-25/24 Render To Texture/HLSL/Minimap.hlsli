
Texture2D g_Tex : register(t0);
SamplerState g_Sam : register(s0);

cbuffer CBChangesEveryFrame : register(b0)
{
    float3 g_EyePosW;            // �����λ��
    float g_Pad;
}

cbuffer CBDrawingStates : register(b1)
{
    int g_FogEnabled;            // �Ƿ�Χ����
    float g_VisibleRange;        // 3D������ӷ�Χ
    float2 g_Pad2;
    float4 g_RectW;              // С��ͼxOzƽ���Ӧ3D�����������(Left, Front, Right, Back)
    float4 g_InvisibleColor;     // ����������µ���ɫ
}


struct VertexPosTex
{
    float3 PosL : POSITION;
    float2 Tex : TEXCOORD;
};

struct VertexPosHTex
{
    float4 PosH : SV_POSITION;
    float2 Tex : TEXCOORD;
};





