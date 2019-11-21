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

  if (projColor.a == 0.0 || projCoord.q <= 0.0 || dp >= 0.0) {
    gl_FragColor = modelColor;
  } else if (projColor.a == 0.0 || projCoord.q <= 0.0 || (dp > -4.0 && dp < 0)) {
     gl_FragColor = mix(modelColor, projColor, abs(dp/8));
  } else {
    gl_FragColor = projColor;
  }

  gl_FragColor *= gl_Color;
}
