#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "cyTriMesh.h"
#include <iostream>

/* GLFW user input callbacks */
void keyInput(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE) 
    {
        glfwSetWindowShouldClose(window, true);
    }
}

int main(int argc, char** argv) 
{
    /* Parse Command Line Arguements */
    if(argc != 2) 
    {
        std::cout << "A single argument of a path to a obj file is expected" << std::endl;
        return -1;
    }

    /* Declare Variables */
    cy::TriMesh mesh;
    const char* const objPath = argv[1];
    const double animationDuration = 1.0;
    double timeElapsed = 0.0;
    double animationStartTime = 0.0;
    float r = 1.0f;
    float g = 0.0f;
    float b = 0.0f;

    /* Initialize a GLFW window */
    if (!glfwInit()) 
    {
        std::cout << "Could not initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(640, 480, "InteractiveGraphicsProject", NULL, NULL);
    if (!window) {
        std::cout << "Could not initialize a GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    /* Initialize GLFW keyboard callbacks */
    glfwSetKeyCallback(window, keyInput);


    /* Initialize GLEW for using OpenGL functions */
    GLenum err = glewInit();
    if (GLEW_OK != err) 
    {
        std::cout << "Could not initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    /* Load Obj File Specified and Store the Vertices */
    if (!mesh.LoadFromFileObj(objPath))
    {
        std::cout << "Could not load mesh from obj file at path " << objPath << std::endl;
        glfwTerminate();
        return -1;        
    }

    /* Create Vertex Buffer Object and Vertex Array Object */
    GLuint vBuf, vArr;
    glGenBuffers(1, &vBuf); 
    glGenVertexArrays(1, &vArr);
    
    glBindVertexArray(vArr);
    glBindBuffer(GL_ARRAY_BUFFER, vBuf);
    glBufferData(GL_ARRAY_BUFFER, mesh.NV()*sizeof(cy::Vec3f), &mesh.V(0), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(cy::Vec3f), 0);
    glEnableVertexAttribArray(vArr);

    /* Create Shaders */
    GLuint vShader;
    vShader = glCreateShader(GL_VERTEX_SHADER);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);

    /* Seed random number generator*/
    std::srand(1);

    /* Execute GLFW Window */
    while (!glfwWindowShouldClose(window))
    {
        if (timeElapsed > animationDuration) 
        {
            animationStartTime = glfwGetTime();
            r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
            g = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
            b = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        }
        timeElapsed = glfwGetTime() - animationStartTime;

        glBindVertexArray(vArr);
        glDrawArrays(GL_POINTS, 0, mesh.NV());

        //glClearColor(r, g, b, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glfwTerminate();
    return 0;
}

