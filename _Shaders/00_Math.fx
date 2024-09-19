

// Color math 
float3 RGBtoXYZ(float3 rgb)
{
    float cr = rgb.r;
    float cg = rgb.g;
    float cb = rgb.b;
    if (cr > 0.04045)
        cr = pow((cr + 0.055) / 1.055, 2.4);
    else
        cr = cr / 12.92;
    if (cg > 0.04045)
        cg = pow((cg + 0.055) / 1.055, 2.4);
    else
        cg = cg / 12.92;
    if (cb > 0.04045)
        cb = pow((cb + 0.055) / 1.055, 2.4);
    else
        cb = cb / 12.92;
    
    cr = cr * 100;
    cg = cg * 100;
    cb = cb * 100;
    
    float3 result;
    result.x = cr * 0.4124 + cg * 0.3576 + cb * 0.1805;
    result.y = cr * 0.2126 + cg * 0.7152 + cb * 0.0722;
    result.z = cr * 0.0193 + cg * 0.1192 + cb * 0.9505;
    return result;
}

float3 XYZtoRGB(float3 val)
{
    float x = val.x;
    float y = val.y;
    float z = val.z;
    
    float r = x * 3.2406 + y * -1.5372 + z * -0.4986;
    float g = x * -0.9689 + y * 1.8758 + z * 0.0415;
    float b = x * 0.0557 + y * -0.2040 + z * 1.0570;
    
    if (r > 0.0031308)
        r = 1.055 * (pow(r, 1 / 2.4)) - 0.055;
    else
        r = 12.92 * r;
    if (g > 0.0031308)
        g = 1.055 * (pow(g, 1 / 2.4)) - 0.055;
    else
        g = 12.92 * g;
    if (b > 0.0031308)
        b = 1.055 * (pow(b, 1 / 2.4)) - 0.055;
    else
        b = 12.92 * b;
    return float3(r, g, b);
}

float3 XYZtoYxy(float3 val)
{
    float3 result;
    result.x = val.y; // Y
    result.y = val.x / (val.x + val.y + val.z); // x
    result.z = val.y / (val.x + val.y + val.z); // y
    return result;
}

float3 YxytoXYZ(float3 val) //val component is Y:x x:y y:z
{
    float3 result;
    float Y = val.x;
    float x = val.y;
    float y = val.z;
    result.x = x * (Y / y);
    result.y = Y;
    result.z = (1 - x - y) * (Y / y);
    return result;
}

float3 RGBtoYxy(float3 val)
{
    return XYZtoYxy(RGBtoXYZ(val));
}

float3 YxytoRGB(float3 val)
{
    return XYZtoRGB(YxytoXYZ(val));
}