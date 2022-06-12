#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{    
    FragColor = texture(texture_diffuse1, TexCoords);
}

//#ifdef GL_ES
//precision highp float;
//#endif

//uniform vec2 u_resolution;

//void main(){
//    vec2 position = gl_FragCoord.xy / u_resolution;
//    vec4 color = vec4(0.97, 0.1, 0.53, 1.0);
    
//    // center (hopefully)
//    vec2 P1 = vec2(0.0,0.0);
//    // top right
//    vec2 P2 = vec2(1.0,1.0);
    
//    // generate 100 points between P1...P2
//    for(float i = 0.0; i < 1.0; i+=0.01) {
//        float lerpX = mix(P1.x, P2.x, i);
//        float lerpY = mix(P1.y, P2.y, i);
//        vec2 interpolatedPoint = vec2(lerpX, lerpY);

//        // check if current fragment is one of the
//        // interpolated points and color it
//        if (distance(position, interpolatedPoint) <= 0.01) {
//            gl_FragColor = color;
//        }
//    }
//}