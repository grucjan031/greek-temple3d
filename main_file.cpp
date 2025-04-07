#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "myTeapot.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraAngle = 0.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float speed_x = 0;
float speed_y = 0;
float aspectRatio = 1;

ShaderProgram* sp;

float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch = 0.0f;
bool firstMouse = true;
const float floorLevel = 0.0f;

float fov = 50.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    const float sensitivity = 0.1f;
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
    
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}


std::vector<glm::vec3> vertices, amphoraVertices, godVertices, sunVertices, atlasVertices;
std::vector<glm::vec2> uvs, amphoraUVs, godUVs, sunUVs, atlasUVs;
std::vector<glm::vec3> normals, amphoraNormals, godNormals, sunNormals, atlasNormals;

GLuint vertexBuffer, uvBuffer, normalBuffer;
GLuint amphoraVertexBuffer, godVertexBuffer, sunVertexBuffer, atlasVertexBuffer;
GLuint amphoraUVBuffer, godUVBuffer, sunUVBuffer, atlasUVBuffer;
GLuint amphoraNormalBuffer, godNormalBuffer, sunNormalBuffer, atlasNormalBuffer;
GLuint tex0;

float floorVertices[] = {

    -175.0f, -10.0f, -175.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
     175.0f, -10.0f, -175.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
    -175.0f, -10.0f,  175.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
     175.0f, -10.0f,  175.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
};
unsigned int floorIndices[] = {
    0, 1, 2,
    1, 3, 2
};
GLuint floorVAO, floorVBO, floorEBO, tex1;

bool loadOBJ(const char* path, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec2>& out_uvs, std::vector<glm::vec3>& out_normals) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path)) {
        std::cerr << err << std::endl;
        return false;
    }

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            // Check vertex index bounds
            if (index.vertex_index >= 0 && index.vertex_index < attrib.vertices.size() / 3) {
                glm::vec3 vertex = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
                out_vertices.push_back(vertex);
            }
            else {
                std::cerr << "Vertex index out of range: " << index.vertex_index << std::endl;
                return false;
            }

            // Check texcoord index bounds, handle missing texcoords
            if (index.texcoord_index >= 0 && index.texcoord_index < attrib.texcoords.size() / 2) {
                glm::vec2 uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                };
                out_uvs.push_back(uv);
            }
            else {
                // If texcoord index is invalid, push a default value
                out_uvs.push_back(glm::vec2(0.0f, 0.0f));
            }

            // Check normal index bounds, handle missing normals
            if (index.normal_index >= 0 && index.normal_index < attrib.normals.size() / 3) {
                glm::vec3 normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
                out_normals.push_back(normal);
            }
            else {
                // If normal index is invalid, push a default value (e.g., up vector)
                out_normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
            }
        }
    }

    // Print number of loaded vertices for debugging
    std::cout << "Loaded " << out_vertices.size() << " vertices from " << path << std::endl;

    return true;
}


// Error callback procedure
void error_callback(int error, const char* description) {
    fputs(description, stderr);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    const float cameraSpeed = 0.5f;

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_W) cameraPos += cameraSpeed * cameraFront;
        if (key == GLFW_KEY_S) cameraPos -= cameraSpeed * cameraFront;
        if (key == GLFW_KEY_A) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (key == GLFW_KEY_D) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (key == GLFW_KEY_LEFT) {
            cameraAngle -= cameraSpeed;
        }
        if (key == GLFW_KEY_RIGHT) {
            cameraAngle += cameraSpeed;
        }
    }
    if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT) speed_x = 0;
        if (key == GLFW_KEY_RIGHT) speed_x = 0;
        if (key == GLFW_KEY_UP) speed_y = 0;
        if (key == GLFW_KEY_DOWN) speed_y = 0;
    }
}

void processInput(GLFWwindow* window) {
    const float cameraSpeed = 2.5f * deltaTime;

    glm::vec3 cameraFrontXZ = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFrontXZ;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFrontXZ;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFrontXZ, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFrontXZ, cameraUp)) * cameraSpeed;

    cameraPos.y = 0.0f;
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    if (height == 0) return;
    aspectRatio = (float)width / (float)height;
    glViewport(0, 0, width, height);
}

GLuint readTexture(const char* filename) {
    GLuint tex;
    glActiveTexture(GL_TEXTURE0);

    std::vector<unsigned char> image;
    unsigned width, height;
    unsigned error = lodepng::decode(image, width, height, filename);
    if (error) {
        std::cerr << "Error loading texture " << filename << ": " << lodepng_error_text(error) << std::endl;
        return 0;
    }

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLint success;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &success);
    if (success == 0) {
        std::cerr << "Texture loading failed for " << filename << std::endl;
        return 0;
    }

    return tex;
}

void initBuffers() {
    if (!vertices.empty()) {
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &uvBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
        glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

        glGenBuffers(1, &normalBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    }

    if (!amphoraVertices.empty()) {
        glGenBuffers(1, &amphoraVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, amphoraVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, amphoraVertices.size() * sizeof(glm::vec3), &amphoraVertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &amphoraUVBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, amphoraUVBuffer);
        glBufferData(GL_ARRAY_BUFFER, amphoraUVs.size() * sizeof(glm::vec2), &amphoraUVs[0], GL_STATIC_DRAW);

        glGenBuffers(1, &amphoraNormalBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, amphoraNormalBuffer);
        glBufferData(GL_ARRAY_BUFFER, amphoraNormals.size() * sizeof(glm::vec3), &amphoraNormals[0], GL_STATIC_DRAW);
    }

    if (!godVertices.empty()) {
        glGenBuffers(1, &godVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, godVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, godVertices.size() * sizeof(glm::vec3), &godVertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &godUVBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, godUVBuffer);
        glBufferData(GL_ARRAY_BUFFER, godUVs.size() * sizeof(glm::vec2), &godUVs[0], GL_STATIC_DRAW);

        glGenBuffers(1, &godNormalBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, godNormalBuffer);
        glBufferData(GL_ARRAY_BUFFER, godNormals.size() * sizeof(glm::vec3), &godNormals[0], GL_STATIC_DRAW);
    }
    if (!sunVertices.empty()) {
        glGenBuffers(1, &sunVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, sunVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sunVertices.size() * sizeof(glm::vec3), &sunVertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &sunUVBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, sunUVBuffer);
        glBufferData(GL_ARRAY_BUFFER, sunUVs.size() * sizeof(glm::vec2), &sunUVs[0], GL_STATIC_DRAW);

        glGenBuffers(1, &sunNormalBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, sunNormalBuffer);
        glBufferData(GL_ARRAY_BUFFER, sunNormals.size() * sizeof(glm::vec3), &sunNormals[0], GL_STATIC_DRAW);
    }

    if (!atlasVertices.empty()) {
        glGenBuffers(1, &atlasVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, atlasVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, atlasVertices.size() * sizeof(glm::vec3), &atlasVertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &atlasUVBuffer);  
        glBindBuffer(GL_ARRAY_BUFFER, atlasUVBuffer);
        glBufferData(GL_ARRAY_BUFFER, atlasUVs.size() * sizeof(glm::vec2), &atlasUVs[0], GL_STATIC_DRAW);

        glGenBuffers(1, &atlasNormalBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, atlasNormalBuffer);
        glBufferData(GL_ARRAY_BUFFER, atlasNormals.size() * sizeof(glm::vec3), &atlasNormals[0], GL_STATIC_DRAW);
    }



    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glGenBuffers(1, &floorEBO);

    glBindVertexArray(floorVAO);

    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}




GLuint tex2, tex3, tex4, tex5, tex6;

void initOpenGLProgram(GLFWwindow* window) {
    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
    glfwSetWindowSizeCallback(window, windowResizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouse_callback);

    sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");

    if (!loadOBJ("templo.obj", vertices, uvs, normals)) {
        std::cerr << "Failed to load templo.obj" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!loadOBJ("amphora.obj", amphoraVertices, amphoraUVs, amphoraNormals)) {
        std::cerr << "Failed to load amphora.obj" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!loadOBJ("god.obj", godVertices, godUVs, godNormals)) {
        std::cerr << "Failed to load god.obj" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (!loadOBJ("sun.obj", sunVertices, sunUVs, sunNormals)) {
        std::cerr << "Failed to load sun.obj" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (!loadOBJ("Atlas.obj", atlasVertices, atlasUVs, atlasNormals)) {
        std::cerr << "Failed to load atlas.obj" << std::endl;
        exit(EXIT_FAILURE);
    }

    tex0 = readTexture("grass.png");
    if (tex0 == 0) {
        std::cerr << "Failed to load texture grass.png" << std::endl;
        exit(EXIT_FAILURE);
    }

    tex1 = readTexture("marmur.png");
    if (tex1 == 0) {
        std::cerr << "Failed to load texture marmur.png" << std::endl;
        exit(EXIT_FAILURE);
    }

    tex2 = readTexture("sky.png");
    if (tex2 == 0) {
        std::cerr << "Failed to load texture sky.png" << std::endl;
        exit(EXIT_FAILURE);
    }
    tex3 = readTexture("marmur2.png");
    if (tex3 == 0) {
        std::cerr << "Failed to load texture test.png" << std::endl;
        exit(EXIT_FAILURE);
    }
    tex4 = readTexture("bronze.png");
    if (tex4 == 0) {
        std::cerr << "Failed to load texture test.png" << std::endl;
        exit(EXIT_FAILURE);
    }
    tex5 = readTexture("marble.png");
    if (tex5 == 0) {
        std::cerr << "Failed to load texture marble.png" << std::endl;
        exit(EXIT_FAILURE);
    }
    tex6 = readTexture("sun.png");
    if (tex5 == 0) {
        std::cerr << "Failed to load texture sun.png" << std::endl;
        exit(EXIT_FAILURE);
    }


    initBuffers();
}



void freeOpenGLProgram(GLFWwindow* window) {
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &uvBuffer);
    glDeleteBuffers(1, &normalBuffer);
    glDeleteBuffers(1, &amphoraVertexBuffer);
    glDeleteBuffers(1, &amphoraUVBuffer);
    glDeleteBuffers(1, &amphoraNormalBuffer);
    glDeleteBuffers(1, &godVertexBuffer);
    glDeleteBuffers(1, &godUVBuffer);
    glDeleteBuffers(1, &godNormalBuffer);
    glDeleteBuffers(1, &sunVertexBuffer);
    glDeleteBuffers(1, &sunUVBuffer);
    glDeleteBuffers(1, &sunNormalBuffer);
    glDeleteBuffers(1, &atlasVertexBuffer);
    glDeleteBuffers(1, &atlasUVBuffer);
    glDeleteBuffers(1, &atlasNormalBuffer);
    delete sp;
}

void drawFloor(glm::mat4 P, glm::mat4 V) {
    glm::mat4 floorModel = glm::mat4(1.0f);

    sp->use();

    glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
    glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(floorModel));

    glUniform1i(sp->u("modelType"), 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex0);
    glUniform1i(sp->u("tex0"), 0);

    glBindVertexArray(floorVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void drawModel(glm::mat4 P, glm::mat4 V, glm::mat4 model, std::vector<glm::vec3>& vertices, GLuint vertexBuffer, GLuint uvBuffer, GLuint normalBuffer, GLuint texture, int modelType) {
    if (vertices.empty()) {
        std::cerr << "Error: Vertex buffer is empty." << std::endl;
        return;
    }

    sp->use();

    glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
    glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(model));

    glUniform1i(sp->u("modelType"), modelType);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(sp->u("tex0"), 0);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

void drawAmphora(glm::mat4 P, glm::mat4 V) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, -4.0f, -70.0f));
    model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
    drawModel(P, V, model, amphoraVertices, amphoraVertexBuffer, amphoraUVBuffer, amphoraNormalBuffer, tex1, 0);
}
void drawAmphora2(glm::mat4 P, glm::mat4 V) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(6.1f, 12.5f, -63.0f));
    model = glm::scale(model, glm::vec3(0.027f, 0.027f, 0.027f));
    float angle = glm::radians(90.0f);
    glm::vec3 rotationAxis(0.0f, 0.2f, 1.0f);
    model = glm::rotate(model, angle, rotationAxis);
    drawModel(P, V, model, amphoraVertices, amphoraVertexBuffer, amphoraUVBuffer, amphoraNormalBuffer, tex5, 0);
}

void drawSky(glm::mat4 P, glm::mat4 V) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, -4.0f, -50.0f));
    model = glm::scale(model, glm::vec3(0.52f, 0.52f, 0.52f));
    drawModel(P, V, model, amphoraVertices, amphoraVertexBuffer, amphoraUVBuffer, amphoraNormalBuffer, tex2, 0);
}
void drawSky2(glm::mat4 P, glm::mat4 V) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 100.0f, -50.0f));
    model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
    drawModel(P, V, model, amphoraVertices, amphoraVertexBuffer, amphoraUVBuffer, amphoraNormalBuffer, tex6, 0);
}


void drawGod(glm::mat4 P, glm::mat4 V) {
    float angle = glm::radians(270.0f);
    glm::vec3 axis(1.0f, 0.0f, 0.0f);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-14.0f, -7.7f, -70.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.7f, 1.7f));
    model = glm::rotate(model, angle, axis);
    drawModel(P, V, model, godVertices, godVertexBuffer, godUVBuffer, godNormalBuffer, tex3, 0);
}


void drawAtlas(glm::mat4 P, glm::mat4 V) {
   // float angle = glm::radians(270.0f);
    glm::vec3 axis(1.0f, 0.0f, 0.0f);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(8.0f, -7.7f, -70.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.7f, 1.7f));
   // model = glm::rotate(model, angle, axis);
    drawModel(P, V, model, atlasVertices, atlasVertexBuffer, atlasUVBuffer, atlasNormalBuffer, tex4, 0); 
}
void drawTemple(glm::mat4 P, glm::mat4 V) {
    glm::mat4 templeModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -10.0f, 0.0f));
    templeModel = glm::scale(templeModel, glm::vec3(1.0f, 2.0f, 1.0f));
    drawModel(P, V, templeModel, vertices, vertexBuffer, uvBuffer, normalBuffer, tex1, 0);
}

void drawScene(GLFWwindow* window, float angle_x, float angle_y) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 V = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glm::mat4 P = glm::perspective(glm::radians(fov), aspectRatio, 0.01f, 150.0f);

    drawTemple(P, V);
    drawAmphora(P, V);
    drawAmphora2(P, V);

    drawGod(P, V);
    drawAtlas(P, V);
    drawFloor(P, V);
    drawSky(P, V);
    drawSky2(P, V);
    glfwSwapBuffers(window);
}


int main(void) {
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        fprintf(stderr, "Cannot initialize GLFW.\n");
        exit(EXIT_FAILURE);
    }

    window = glfwCreateWindow(1920, 1080, "OpenGL", NULL, NULL);

    if (!window) {
        fprintf(stderr, "Cannot create window.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Cannot initialize GLEW.\n");
        exit(EXIT_FAILURE);
    }

    initOpenGLProgram(window);

    float angle_x = 0;
    float angle_y = 0;
    glfwSetTime(0);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        angle_x += speed_x * glfwGetTime();
        angle_y += speed_y * glfwGetTime();
        glfwSetTime(0);
        drawScene(window, angle_x, angle_y);
        glfwPollEvents();
    }

    freeOpenGLProgram(window);

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

