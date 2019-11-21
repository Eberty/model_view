#version 120

uniform sampler2D projectionMap;
uniform sampler2D originalTexture;
uniform vec3 projectionNormal;

varying vec4 texCoord;
varying vec4 projCoord;
varying vec3 modelNormal;

void main() {
  vec2 finalCoord = projCoord.st / projCoord.q;

  float dp = modelNormal.x * projectionNormal.x +
             modelNormal.y * projectionNormal.y +
             modelNormal.z * projectionNormal.z;

  vec4 projColor = texture2D(projectionMap, finalCoord + vec2(0.5, 0.5));
  vec4 modelColor = texture2D(originalTexture, texCoord.st);
  bool modelColorSet = false;

  if (projColor.a == 0.0 || projCoord.q <= 0.0 || dp > 0.0) {
    gl_FragColor = modelColor;
    modelColorSet = true;
  } else if (dp < 0.0 && dp > -10.0 && projColor.a != 0 && modelColorSet == false) {
    gl_FragColor = mix(texture2D(originalTexture, texCoord.st), texture2D(projectionMap, finalCoord + vec2(0.5, 0.5)), -dp/10);
  } else {
    gl_FragColor = projColor;
  }

  gl_FragColor *= gl_Color;
}
