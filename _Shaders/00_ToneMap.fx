//Tone Mapping
//DX
#include "00_Math.fx"

cbuffer DXToneMapping_Desc
{
    float Luminance;
};

static const float fMiddleGray = 0.18f;
static const float fWhiteCutoff = 0.9f;

static const float3 LUM_FACTOR = float3(0.299, 0.587, 0.114);
static const float4 LUM_FACTOR2 = float4(0.299, 0.587, 0.114,1);

float luminance(float3 v)
{
    return dot(v, LUM_FACTOR);
}

float3 change_luminance(float3 c_in, float l_out)
{
    float l_in = luminance(c_in);
    return c_in * (l_out / l_in);
}


void Reinhard_Extended_ToneMapping(inout float3 color)
{
    float l_old = luminance(color.xyz);
    float numerator = l_old * (1.0f + (l_old / (LumWhite * LumWhite)));
    float l_new = numerator / (1.0f + l_old);
    color = float3(change_luminance(color.xyz, l_new));    
}

void ReinhardToneMapping(inout float3 color)
{
    color = color / (color + float3(1, 1, 1));
}

void DirectXToneMapping(inout float3 color)
{
    
    float l = dot(color.xyz, LUM_FACTOR);
    float lp = l / (9.6 * AvgLum[0] + 0.0001);    
    float sc = float((lp * (1 + lp / (LumWhite * LumWhite))) / (1 + lp));    
    color.xyz *= sc;
}


void DXToneMapping(inout float4 color)
{
    color = pow(color, 1.f / 2.2f) * fMiddleGray / (Luminance + 0.001f);
    color *= (1.0f + (color / (fWhiteCutoff * fWhiteCutoff)));
    color /= (1.f + color);
    color.a = 1;
}

void EAToneMapping(inout float4 color)
{
    float3 Color = pow(color, 2.2f).xyz;
    float3 x = max(0.0f, Color - 0.004f);
    Color = (x * (6.2f * x + 0.5f)) / (x * (6.2f * x + 1.7f) + 0.06f);
    color = float4(Color, 1.0f);
}


float3 UnchartedToneMappingCal (float3 x)
{
    float A = 0.15f;
    float B = 0.5f;
    float C = 0.1f;
    float D = 0.2f;
    float E = 0.02f;
    float F = 0.3f;
    return ((x * (A * x + C * B)) + D * E) / (x * (A * x + B) + D * F) - E / F;
}

void UnchartedToneMapping(inout float3 color)
{
    // color = pow(color, 2.2f);
    float3 Diffuse = UnchartedToneMappingCal(2 * color);
    float3 WhiteScale = 1.0f / UnchartedToneMappingCal(11.2f);
    color = Diffuse * WhiteScale;
}

static const float3x3 aces_input_matrix =
{
    float3(0.59719f, 0.35458f, 0.04823f),
    float3(0.07600f, 0.90834f, 0.01566f),
    float3(0.02840f, 0.13383f, 0.83777f)
};

static const float3x3 aces_output_matrix =
{
    float3(1.60475f, -0.53108f, -0.07367f),
    float3(-0.10208f, 1.10813f, -0.00605f),
    float3(-0.00327f, -0.07276f, 1.07602f)
};

float3 rtt_and_odt_fit(float3 v)
{
    float3 a = v * (v + 0.0245786f) - 0.000090537f;
    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

void Aces_ToneMapping(inout float3 color)
{
    float3 val = mul(aces_input_matrix, color.xyz);
    val = rtt_and_odt_fit(val);
    color = mul(aces_output_matrix, val);    
    color = saturate(color);
}

float3 PS_ReinhardSimple(float3 color)
{
    float3 result = color;
    ReinhardToneMapping(result);
    return result;
}


float3 PS_ReinharEX(float3 color)
{
    float3 result = color;    
    Reinhard_Extended_ToneMapping(result);
    return result;
}

float3 PS_Uncharted(float3 color)
{
    float3 result = color;        
    UnchartedToneMapping(result);
    return result;
}

float3 PS_ACES(float3 color)
{
    float3 result = color;            
    Aces_ToneMapping(result);
    return result;
}

float3 PS_DXTone(float3 color)
{
    float3 result = color;
    DirectXToneMapping(result);
    return result;
}

float3 PS_ToneMap(float3 color)
{
    float3 result;
    
    switch (ToneMapType)
    {
        case 0:
            result = PS_ReinhardSimple(color).xyz;
            break;
        case 1:
            result = PS_ReinharEX(color).xyz;
            break;
        case 2:
            result = PS_Uncharted(color).xyz;
            break;
        case 3:
            result = PS_ACES(color).xyz;
            break;
        case 4:
            result = PS_DXTone(color).xyz;
            break;
    }
    
    return result;
}