Buffer<uint> g_Input : register(t0);
RWBuffer<uint> g_Data : register(u0);

cbuffer CB : register(b0)
{
    uint g_Level;        // 2^��Ҫ��������
    uint g_DescendMask;  // �½���������
    uint g_MatrixWidth;  // ������(Ҫ����>=�߶��Ҷ�Ϊ2�ı���)
    uint g_MatrixHeight; // ����߶�
}