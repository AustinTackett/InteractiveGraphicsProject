#include <GL/glew.h>
#include <GLFW/glfw3.h>
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
    double timeElapsed = 0.0;
    double animationStartTime = 0.0;
    double animationDuration = 1.0;
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

        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glfwTerminate();
    return 0;
}

