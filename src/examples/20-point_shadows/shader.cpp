const char* VERTEX_Z_PASS_SRC = "#version 330 core\n"
                                "layout(location=0) in vec3 position;"
                                "layout(location=1) in vec3 normal;"
                                "layout(location=2) in vec2 texCoords;"
                                "uniform mat4 model;"
                                "uniform mat4 view;"
                                "uniform mat4 projection;"
                                "void main()"
                                "{"
                                "    gl_Position = projection * view * model * vec4(position, 1.0);"
                                "}";

const char* FRAGMENT_Z_PASS_SRC = "#version 330 core\n"
                                  "void main()"
                                  "{"
                                  "}";

const char* VERTEX_LIGHT_SRC = "#version 330 core\n"
                               "layout(location=0) in vec3 position;"
                               "layout(location=1) in vec3 normal;"
                               "layout(location=2) in vec2 texCoords;"
                               "out vec3 fNormal;"
                               "out vec3 fPosition;"
                               "out vec2 fTexCoords;"
                               "uniform mat4 model;"
                               "uniform mat4 view;"
                               "uniform mat4 projection;"
                               "void main()"
                               "{"
                               "    gl_Position = projection * view * model * vec4(position, 1.0);"
                               "    fPosition = (model * vec4(position, 1.0)).xyz;"
                               "    fNormal = mat3(transpose(inverse(model))) * normal;"
                               "    fTexCoords = texCoords;"
                               "}";

const char* FRAGMENT_LIGHT_SRC = "#version 330 core\n"
                                 "in vec3 fNormal;"
                                 "in vec3 fPosition;"
                                 "in vec2 fTexCoords;"
                                 "out vec4 outputColor;"
                                 "uniform vec3 lightPosition;"
                                 "uniform vec3 lightColor;"
                                 "uniform vec3 lightAtt;"
                                 "uniform vec3 viewPos;"
                                 "uniform samplerCube depthMap;"
                                 "uniform float far_plane;"
                                 "float ShadowCalculation(vec3 fragPos)"
                                 "{"
                                 "    vec3 fragToLight = fragPos - lightPosition;"
                                 "    float currentDepth = length(fragToLight);"
                                 "    float closestDepth = texture(depthMap, fragToLight).r * far_plane;"
                                 "    float bias = 0.05;"
                                 "    return currentDepth -  bias > closestDepth ? 1.0 : 0.0;"
                                 "}"
                                 "void main()"
                                 "{"
                                 "    vec3 ambient = vec3(0.1, 0.1, 0.1);"
                                 "    float dist = length(lightPosition - fPosition);"
                                 "    float attenuation = 1.0f / (lightAtt.x + lightAtt.y * dist + lightAtt.z * dist * dist);"
                                 "    float shadow = ShadowCalculation(fPosition);"
                                 "    outputColor = vec4(ambient + (1.0 - shadow) * lightColor * attenuation, 1.0);"
                                 "}";

const char* VERTEX_SHADOW_SRC = "#version 330 core\n"
                                "layout(location=0) in vec3 position;"
                                "uniform mat4 model;"
                                "void main()"
                                "{"
                                "    gl_Position = model * vec4(position, 1.0);"
                                "}";

const char* GEOM_SHADOW_SRC = "#version 330 core\n"
                              "layout(triangles) in;"
                              "layout(triangle_strip, max_vertices=18) out;" // 18 = 6 * 3
                              "uniform mat4 shadowMatrices[6];" // One matrix per face
                              "out vec4 fPosition;"
                              "void main()"
                              "{"
                              "    for(int face = 0; face < 6; ++face)" // 6 faces
                              "    {"
                              "        gl_Layer = face;" // gl_Layer is built in: which cubemap part
                              "        for(int i = 0; i < 3; ++i)" // 3 vertices per face
                              "        {"
                              "            fPosition = gl_in[i].gl_Position;"
                              "            gl_Position = shadowMatrices[face] * fPosition;"
                              "            EmitVertex();"
                              "        }"
                              "        EndPrimitive();"
                              "    }"
                              "}";

const char* FRAGMENT_SHADOW_SRC = "#version 330 core\n"
                                  "in vec4 fPosition;"
                                  "uniform vec3 lightPos;"
                                  "uniform float far_plane;" // camera frustum
                                  "void main()"
                                  "{"
                                  "    float dist = length(fPosition.xyz - lightPos);"
                                  "    dist = dist / far_plane;" // Linearize
                                  "    gl_FragDepth = dist;"
                                  "}";
