#version 120

uniform sampler2D projectionMap;
uniform sampler2D originalTexture;
uniform vec3 projectionNormal;
uniform vec2 projectiveCenter;

varying vec4 texCoord;
varying vec4 projCoord;
varying vec3 modelNormal;

void main() {
  vec2 finalCoord = projCoord.st / projCoord.q;

  float dp = modelNormal.x * projectionNormal.x +
             modelNormal.y * projectionNormal.y +
             modelNormal.z * projectionNormal.z;

  vec4 projColor = texture2D(projectionMap, finalCoord + projectiveCenter);
  vec4 modelColor = texture2D(originalTexture, texCoord.st);

  vec4 finalColor;
  if (projColor.a == 0.0 || projCoord.q <= 0.0 || dp >= 0.0) {
    finalColor = modelColor;
  } else if (projColor.a == 0.0 || projCoord.q <= 0.0 || (dp > -10.0 && dp < 0)) {
    if (modelColor.rgb != vec3(255 / 255.0, 127 / 255.0, 39 / 255.0)) {
      finalColor = mix(modelColor, projColor, abs(dp / 10.0));
   } else {
     finalColor = projColor;
   }
  } else {
    finalColor = projColor;
  }

  if (finalColor.rgb == vec3(255 / 255.0, 127 / 255.0, 39 / 255.0)) {
    discard;
  }

  gl_FragColor = finalColor * gl_Color;
}
