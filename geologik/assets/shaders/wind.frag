#version 430 core

in vec4 windData;
in vec2 scaledPos;

uniform float noiseZ;
uniform float patternSkipValue;

out vec4 color;

vec3 hash(vec3 p)
{
  return fract(
      sin(vec3(dot(p, vec3(1.0, 57.0, 113.0)), dot(p, vec3(57.0, 113.0, 1.0)),
               dot(p, vec3(113.0, 1.0, 57.0)))) *
      43758.5453);
}

vec3 voronoi3d(const in vec3 x) {
  vec3 p = floor(x);
  vec3 f = fract(x);

  float id = 0.0;
  vec2 res = vec2(100.0);
  for (int k = -1; k <= 1; k++) {
    for (int j = -1; j <= 1; j++) {
      for (int i = -1; i <= 1; i++) {
        vec3 b = vec3(float(i), float(j), float(k));
        vec3 r = vec3(b) - f + hash(p + b);
        float d = dot(r, r);

        float cond = max(sign(res.x - d), 0.0);
        float nCond = 1.0 - cond;

        float cond2 = nCond * max(sign(res.y - d), 0.0);
        float nCond2 = 1.0 - cond2;

        id = (dot(p + b, vec3(1.0, 57.0, 113.0)) * cond) + (id * nCond);
        res = vec2(d, res.x) * cond + res * nCond;

        res.y = cond2 * d + nCond2 * res.y;
      }
    }
  }

  return vec3(sqrt(res), abs(id));
}

void main()
{
  float windStrength = clamp(length(windData.xy) * 12, 0, 1);
  float alpha = 1 - windStrength;
  alpha = alpha * alpha;
  alpha = 1 - alpha;
  alpha = max(alpha - 0.2, 0) * 0.7 - 0.1;
  
  vec3 outColor = mix(vec3(0.2, 0.75, 0.9), vec3(1), windStrength);
  //color = vec4(windData.xy * 5 + 0.5, 0.5, 0.7);
  
  float dotColor = 0;

  vec3 noisePos = vec3(scaledPos, noiseZ + windData.z * 0.001);

  int steps = int(windStrength * 10);
  steps -= (steps > 4) ? 0 : 8;
  steps = 5;

  vec2 windDir = normalize(windData.xy) * 4;

  for (int i = 0; i < steps; i++)
  {
    if (i < int(patternSkipValue + 1) && i > int(patternSkipValue - 1))
      continue;
    
    float dot_ = (1 - voronoi3d(noisePos).x) - 0.8;
    dot_ *= 1000;
    dot_ = max(dot_, 0);
    dotColor += dot_;

    noisePos.xy += windDir;
  }

  dotColor = smoothstep(0, 1, clamp(dotColor, 0, 1));

  color = vec4(min(outColor + vec3(dotColor * 0.1), 1), alpha * (1 + dotColor * 5 * alpha));
  color = vec4(vec3(dotColor), 1);
}
