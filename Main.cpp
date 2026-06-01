#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// stb_image para cargar la textura
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

// Vértices: posición y coordenadas 

GLfloat vertices[] =
{
    // Cara frontal
    -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
     0.0f,  0.5f,  0.0f,    0.5f, 1.0f,

     // Cara derecha
      0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
      0.5f, -0.5f, -0.5f,    1.0f, 0.0f,
      0.0f,  0.5f,  0.0f,    0.5f, 1.0f,

      // Cara trasera
       0.5f, -0.5f, -0.5f,    0.0f, 0.0f,
      -0.5f, -0.5f, -0.5f,    1.0f, 0.0f,
       0.0f,  0.5f,  0.0f,    0.5f, 1.0f,

       // Cara izquierda
       -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,
       -0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
        0.0f,  0.5f,  0.0f,    0.5f, 1.0f,

        // Base - primer triángulo
        -0.5f, -0.5f,  0.5f,    0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,    1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,

        // Base - segundo triángulo
        -0.5f, -0.5f,  0.5f,    0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,    1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,    1.0f, 0.0f,
};

// Caras
GLuint indices[] =
{
     0,  1,  2,
     3,  4,  5,
     6,  7,  8,
     9, 10, 11,
    12, 13, 14,
    15, 16, 17,
};

// Curva de Bézier 
glm::vec3 bezierP0 = glm::vec3(-3.0f, 1.5f, 3.0f);
glm::vec3 bezierP1 = glm::vec3(-3.0f, 2.5f, -1.0f);
glm::vec3 bezierP2 = glm::vec3(3.0f, 2.5f, -1.0f);
glm::vec3 bezierP3 = glm::vec3(3.0f, 1.5f, 3.0f);

glm::vec3 evaluarBezier(float t)
{
    float u = 1.0f - t;
    return (u * u * u) * bezierP0
        + (3 * u * u * t) * bezierP1
        + (3 * u * t * t) * bezierP2
        + (t * t * t) * bezierP3;
}

// Variables de cámara
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 2.5f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float speed = 0.01f;
bool  enBezier = true;
float bezierT = 0.0f;
float bezierSpeed = 0.15f;

void processInput(GLFWwindow* window, float deltaTime)
{
    bool teclaPresionada =
        glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

    if (teclaPresionada && enBezier)
    {
        enBezier = false;
        cameraFront = glm::normalize(glm::vec3(0.0f) - cameraPos);
    }

    if (!enBezier)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += speed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= speed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "PIRAMIDE OPENGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "No se pudo crear la ventana" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glViewport(0, 0, 800, 800);
    glEnable(GL_DEPTH_TEST);

    Shader shaderProgram("default.vert", "default.frag");

    VAO VAO1;
    VAO1.Bind();

    VBO VBO1(vertices, sizeof(vertices));
    EBO EBO1(indices, sizeof(indices));

    // Atributo 0: posición (3 floats)
    VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 5 * sizeof(float), (void*)0);

    // Atributo 1: coordenadas UV (2 floats)
    VAO1.LinkAttrib(VBO1, 1, 2, GL_FLOAT, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    VAO1.Unbind();
    VBO1.Unbind();
    EBO1.Unbind();

    // Cargar la textura
    GLuint textura;
    glGenTextures(1, &textura);
    glBindTexture(GL_TEXTURE_2D, textura);

    // Modo de repetición y filtrado
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);

    int anchoTex, altoTex, canalesTex;
    unsigned char* datos = stbi_load(
        "G:\\Universidad\\Computacion\\Project1\\Textura\\TexturaCage.PNG",
        &anchoTex, &altoTex, &canalesTex, 0
    );

    if (datos)
    {
        GLenum formato = (canalesTex == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, formato, anchoTex, altoTex, 0, formato, GL_UNSIGNED_BYTE, datos);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Error al cargar la textura" << std::endl;
    }
    stbi_image_free(datos);

    // Indicar al shader que use la unidad de textura 0
    shaderProgram.Activate();
    glUniform1i(glGetUniformLocation(shaderProgram.ID, "tex0"), 0);

    float tiempoAnterior = glfwGetTime();

    // Bucle de renderizado principal
    while (!glfwWindowShouldClose(window))
    {
        float tiempoActual = glfwGetTime();
        float deltaTime = tiempoActual - tiempoAnterior;
        tiempoAnterior = tiempoActual;

        processInput(window, deltaTime);

        if (enBezier)
        {
            bezierT += bezierSpeed * deltaTime;
            if (bezierT >= 1.0f) { bezierT = 1.0f; bezierSpeed = -bezierSpeed; }
            if (bezierT <= 0.0f) { bezierT = 0.0f; bezierSpeed = -bezierSpeed; }

            cameraPos = evaluarBezier(bezierT);
            cameraFront = glm::normalize(glm::vec3(0.0f) - cameraPos);
        }

        glClearColor(0.04f, 0.16f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderProgram.Activate();

        // Activar la textura en la unidad 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textura);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, tiempoActual, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        VAO1.Bind();
        glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Liberar recursos
    VAO1.Delete();
    VBO1.Delete();
    EBO1.Delete();
    glDeleteTextures(1, &textura);
    shaderProgram.Delete();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}