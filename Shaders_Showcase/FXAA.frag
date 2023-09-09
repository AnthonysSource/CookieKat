#version 450

#define EDGE_THRESHOLD_MIN 0.0312
#define EDGE_THRESHOLD_MAX  0.125
#define QUALITY(q) ((q) < 5 ? 1.0 : ((q) > 5 ? ((q) < 10 ? 2.0 : ((q) < 11 ? 4.0 : 8.0)) : 1.5))
#define ITERATIONS 12
#define SUBPIXEL_QUALITY 0.75

//--------------------------------------------------------------------

layout(set = 0, binding = 0) uniform sampler2D u_Image;
//layout(set = 0, binding = 1) uniform vec2 u_ScreenSize;

// Vertex Stage Input
//--------------------------------------------------------------------

layout(location = 0) in vec2 in_UV;

// Framebuffer Output
//--------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

//--------------------------------------------------------------------

float Rgb2Luma(vec3 rgb){
    return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}

void main()
{
    vec2 u_ScreenSize = vec2(1920, 1080);

    vec3 colorCenter = texture(u_Image, in_UV).rgb;

    float lumaCenter = Rgb2Luma(colorCenter);

    float lumaTop = Rgb2Luma( textureOffset(u_Image, in_UV, ivec2(0, 1) ).rgb );
    float lumaRight = Rgb2Luma( textureOffset(u_Image, in_UV, ivec2(0, 0) ).rgb );
    float lumaBot = Rgb2Luma( textureOffset(u_Image, in_UV, ivec2(0, -1) ).rgb );
    float lumaLeft = Rgb2Luma( textureOffset(u_Image, in_UV, ivec2(-1, 0) ).rgb );

    float lumaMax = max(lumaCenter, max( max(lumaBot, lumaTop), max(lumaLeft, lumaRight) ));
    float lumaMin = min(lumaCenter, min( min(lumaBot, lumaTop), min(lumaLeft, lumaRight) ));

    float lumaRange = lumaMax - lumaMin;

    // If the range is not big enough we don't do any AA
    if(lumaRange < max( EDGE_THRESHOLD_MIN, lumaMax * EDGE_THRESHOLD_MAX )){
        outColor = vec4(colorCenter, 1.0);
        return;
    }

    // Individual corners
    float lumaTopRight = Rgb2Luma( textureOffset(u_Image, in_UV, ivec2(1, 1) ).rgb );
    float lumaBotRight = Rgb2Luma( textureOffset(u_Image, in_UV, ivec2(-1, 1) ).rgb );
    float lumaBotLeft = Rgb2Luma( textureOffset(u_Image, in_UV, ivec2(-1, -1) ).rgb );
    float lumaTopLeft = Rgb2Luma( textureOffset(u_Image, in_UV, ivec2(-1, 1) ).rgb );

    // Combined Edges
    float lumaTopBot = lumaBot + lumaTop;
    float lumaLeftRight = lumaLeft + lumaRight;

    float lumaLeftCorners = lumaBotLeft + lumaTopLeft;
    float lumaRightCorners = lumaTopRight + lumaBotRight;
    float lumaTopCorners = lumaTopRight + lumaTopLeft;
    float lumaBotCorners = lumaBotRight + lumaBotLeft;

    // Check if edge is horizontal or vertical
    float edgeHorizontal = abs(-2.0 * lumaLeft + lumaLeftCorners) + abs(-2.0 * lumaCenter + lumaTopBot) * 2.0 + abs(-2.0 * lumaRight + lumaRightCorners);
    float edgeVertical = abs(-2.0 * lumaTop + lumaTopCorners) + abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0 + abs(-2.0 * lumaBot + lumaBotCorners);

    bool isHorizontal = (edgeHorizontal >= edgeVertical);

    float luma1 = isHorizontal ? lumaBot : lumaLeft;
    float luma2 = isHorizontal ? lumaTop : lumaRight;
    float gradient1 = luma1 - lumaCenter;
    float gradient2 = luma2 - lumaCenter;

    bool is1Steepest = abs(gradient1) >= abs(gradient2);

    float gradientScaled = 0.25*max( abs(gradient1), abs(gradient2) );

    vec2 inverseScreenSize = 1.0/u_ScreenSize;
    float stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;

    float lumaLocalAverage = 0.0;

    if(is1Steepest){
        stepLength = -stepLength;
        lumaLocalAverage = 0.5*(luma1 + lumaCenter); 
    }else{
        lumaLocalAverage = 0.5*(luma2 + lumaCenter);
    }

    vec2 currentUv = in_UV;
    if(isHorizontal){
        currentUv.y += stepLength * 0.5;
    } else {
        currentUv.x += stepLength * 0.5;
    }

    // Compute offset (for each iteration step) in the right direction.
    vec2 offset = isHorizontal ? vec2(inverseScreenSize.x,0.0) : vec2(0.0,inverseScreenSize.y);
    // Compute UVs to explore on each side of the edge, orthogonally. The QUALITY allows us to step faster.
    vec2 uv1 = currentUv - offset;
    vec2 uv2 = currentUv + offset;

    // Read the lumas at both current extremities of the exploration segment, and compute the delta wrt to the local average luma.
    float lumaEnd1 = Rgb2Luma(texture(u_Image,uv1).rgb);
    float lumaEnd2 = Rgb2Luma(texture(u_Image,uv2).rgb);
    lumaEnd1 -= lumaLocalAverage;
    lumaEnd2 -= lumaLocalAverage;

    // If the luma deltas at the current extremities are larger than the local gradient, we have reached the side of the edge.
    bool reached1 = abs(lumaEnd1) >= gradientScaled;
    bool reached2 = abs(lumaEnd2) >= gradientScaled;
    bool reachedBoth = reached1 && reached2;

    // If the side is not reached, we continue to explore in this direction.
    if(!reached1){
        uv1 -= offset;
    }
    if(!reached2){
        uv2 += offset;
    }

    // If both sides have not been reached, continue to explore.
    if(!reachedBoth){

        for(int i = 2; i < ITERATIONS; i++){
            // If needed, read luma in 1st direction, compute delta.
            if(!reached1){
                lumaEnd1 = Rgb2Luma(texture(u_Image, uv1).rgb);
                lumaEnd1 = lumaEnd1 - lumaLocalAverage;
            }
            // If needed, read luma in opposite direction, compute delta.
            if(!reached2){
                lumaEnd2 = Rgb2Luma(texture(u_Image, uv2).rgb);
                lumaEnd2 = lumaEnd2 - lumaLocalAverage;
            }
            // If the luma deltas at the current extremities is larger than the local gradient, we have reached the side of the edge.
            reached1 = abs(lumaEnd1) >= gradientScaled;
            reached2 = abs(lumaEnd2) >= gradientScaled;
            reachedBoth = reached1 && reached2;

            // If the side is not reached, we continue to explore in this direction, with a variable quality.
            if(!reached1){
                uv1 -= offset * QUALITY(i);
            }
            if(!reached2){
                uv2 += offset * QUALITY(i);
            }

            // If both sides have been reached, stop the exploration.
            if(reachedBoth){ break;}
        }
        }

    // Compute the distances to each extremity of the edge.
    float distance1 = isHorizontal ? (in_UV.x - uv1.x) : (in_UV.y - uv1.y);
    float distance2 = isHorizontal ? (uv2.x - in_UV.x) : (uv2.y - in_UV.y);

    // In which direction is the extremity of the edge closer ?
    bool isDirection1 = distance1 < distance2;
    float distanceFinal = min(distance1, distance2);

    // Length of the edge.
    float edgeThickness = (distance1 + distance2);

    // UV offset: read in the direction of the closest side of the edge.
    float pixelOffset = - distanceFinal / edgeThickness + 0.5;


    // Is the luma at center smaller than the local average ?
    bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;

    // If the luma at center is smaller than at its neighbour, the delta luma at each end should be positive (same variation).
    // (in the direction of the closer side of the edge.)
    bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;

    // If the luma variation is incorrect, do not offset.
    float finalOffset = correctVariation ? pixelOffset : 0.0;

    // Sub-pixel shifting
    // Full weighted average of the luma over the 3x3 neighborhood.
    float lumaAverage = (1.0/12.0) * (2.0 * (lumaTopBot + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
    // Ratio of the delta between the global average and the center luma, over the luma range in the 3x3 neighborhood.
    float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter)/lumaRange,0.0,1.0);
    float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
    // Compute a sub-pixel offset based on this delta.
    float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;

    // Pick the biggest of the two offsets.
    finalOffset = max(finalOffset,subPixelOffsetFinal);

    // Compute the final UV coordinates.
    vec2 finalUv = in_UV;
    if(isHorizontal){
        finalUv.y += finalOffset * stepLength;
    } else {
        finalUv.x += finalOffset * stepLength;
    }

    // Read the color at the new UV coordinates, and use it.
    vec3 finalColor = texture(u_Image, finalUv).rgb;
    outColor = vec4(finalColor, 1.0);
}
