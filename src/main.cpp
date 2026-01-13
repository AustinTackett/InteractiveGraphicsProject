#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "cyTriMesh.h"
#include <iostream>
#include <fstream>
#include <string>

/* GLFW user input callbacks */
void keyInput(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE) 
    {
        glfwSetWindowShouldClose(window, true);
    }
}

/* Reading Glsl Files */
bool readFile(const std::string filePath, std::string& outString)
{
    /* Open File */
    std::ifstream file(filePath);    
    if (!file.is_open())
    {
        return false;
    }

    /* Read From File */
    std::string tempStringLine; 
    outString.clear();
    while(std::getline(file, tempStringLine))
    {
        outString += tempStringLine;
        outString += '\n';
    }

    return true; 
}

int main(int argc, char** argv) 
{
    /* Parse Command Line Arguements */
    const char* const OBJ_PATH = argv[1];
    if(argc != 2) 
    {
        std::cerr << "A single argument of a path to a obj file is expected" << std::endl;
        return -1;
    }

    /* Initialize a GLFW window */
    int glfwErr = glfwInit();
    if (GLFW_TRUE != glfwErr)
    {
        std::cerr << "Could not initialize GLFW" << std::endl;
        return -1;
    }

    /* Create a GLFW window */
    int windowWidth = 640;
    int windowHeight = 480;
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "InteractiveGraphicsProject", NULL, NULL);
    if (!window) {
        std::cerr << "Could not initialize a GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    /* Initialize GLFW keyboard callbacks */
    glfwSetKeyCallback(window, keyInput);

    /* Initialize GLEW for using OpenGL functions */
    GLenum glewErr = glewInit();
    if (GLEW_OK != glewErr) 
    {
        std::cerr << "Could not initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    /* Load Obj File Specified and Store the Vertices */
    cy::TriMesh mesh;
    bool meshErr = mesh.LoadFromFileObj(OBJ_PATH);
    if (!meshErr)
    {
        std::cerr << "Could not load mesh from obj file at path " << OBJ_PATH << std::endl;
        glfwTerminate();
        return -1;        
    }

    /* Create Vertex Buffer Object and Vertex Array Object  */
    GLuint vBuf, vArr;
    GLsizei numVBufs = 1;
    GLsizei numVArrs = 1;
    glGenBuffers(numVBufs, &vBuf); 
    glGenVertexArrays(numVArrs, &vArr);
    
    glBindVertexArray(vArr);
    glBindBuffer(GL_ARRAY_BUFFER, vBuf);
    glBufferData(GL_ARRAY_BUFFER, mesh.NV()*sizeof(cy::Vec3f), &mesh.V(0), GL_STATIC_DRAW);

    GLuint vAttribIdx = 0;
    GLuint numVComponents = 3;
    const void* vBuffStartOffset = 0;
    glVertexAttribPointer(vAttribIdx, numVComponents, GL_FLOAT, GL_FALSE, sizeof(cy::Vec3f), vBuffStartOffset);
    glEnableVertexAttribArray(vArr);

    /* Read Vertex Shaders Code From Files */
    const std::string vShaderPath = "res/shaders/vertShader.glsl";

    std::string vhaderString;
    bool vShaderFileErr = readFile(vShaderPath, vhaderString);
    if (!vShaderFileErr)
    {
        std::cerr << "Could not load vertex shader at path " << vShaderPath << std::endl;
        glfwTerminate();
        return -1;   
    }

    /* Read Fragment Shaders Code From Files */
    const std::string fShaderPath = "res/shaders/fragShader.glsl";

    std::string fShaderString;
    bool fShaderFileErr = readFile(fShaderPath, fShaderString);
    if (!vShaderFileErr)
    {
        std::cerr << "Could not load fragment shader at path " << fShaderPath << std::endl;
        glfwTerminate();
        return -1;   
    }

    /* Compile Vertex Shaders */
    GLuint vShader;
    vShader = glCreateShader(GL_VERTEX_SHADER);

    const GLchar* vshaderCharArray = vhaderString.c_str();
    glShaderSource(vShader, 1, &vshaderCharArray, NULL);

    GLint vshaderCompileErr;
    const unsigned int vInfoLogLength = 1024;
    char vShaderInfolog[vInfoLogLength];

    glCompileShader(vShader);
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &vshaderCompileErr);
    if (!vshaderCompileErr) 
    {
        glGetShaderInfoLog(vShader, vInfoLogLength, NULL, &vShaderInfolog[0]);
        std::cerr << "Could not compile vertex shaders\n" << vShaderInfolog << std::endl;

        glDeleteShader(vShader);
        glfwTerminate();
        return -1;     
    }

    /* Compile Fragment Shaders */
    GLuint fShader;
    fShader = glCreateShader(GL_FRAGMENT_SHADER);

    const GLchar* fShaderCharArray = fShaderString.c_str();
    glShaderSource(fShader, 1, &fShaderCharArray, NULL);

    GLint fshaderCompileErr;
    const unsigned int fInfoLogLength = 1024;
    char fShaderInfolog[fInfoLogLength];

    glCompileShader(fShader);
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &fshaderCompileErr);
    if (!fshaderCompileErr) 
    {
        glGetShaderInfoLog(fShader, 1024, NULL, &fShaderInfolog[0]);
        std::cerr << "Could not compile fragment shaders\n" << fShaderInfolog << std::endl;
        
        glDeleteShader(vShader);
        glDeleteShader(fShader);
        glfwTerminate();
        return -1;     
    }

    /* Link Shaders to OpenGL program */
    GLuint program = glCreateProgram();
    GLint linkProgramErr;

    const unsigned int programInfoLogLength = 1024;
    char programInfolog[programInfoLogLength];

    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linkProgramErr);
    if (!linkProgramErr)
    {
        glGetProgramInfoLog(program, programInfoLogLength, NULL, &programInfolog[0]);
        std::cerr << "Could not link opengl program\n" << programInfolog << std::endl;

        glDeleteShader(vShader);
        glDeleteShader(fShader);
        glDeleteProgram(program);
        glfwTerminate();
        return -1;      
    }

    /* Assign Uniform Shaders */
    GLint location = glGetUniformLocation(program,"mvp");
    //glUniformMatrix4fv()

    /* Detach and delete shader objects after they are contained in the program */
    glDetachShader(program, vShader);
    glDetachShader(program, fShader);
    glDeleteShader(vShader);
    glDeleteShader(fShader);

    /* Seed random number generator for animating background */
    std::srand(1);
    const double animationDuration = 1.0;
    double timeElapsed = 0.0;
    double animationStartTime = 0.0;
    float r = 1.0f;
    float g = 0.0f;
    float b = 0.0f;

    /* Execute GLFW Window */
    while (!glfwWindowShouldClose(window))
    {
        /* Animate Background */
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

        /* Render Obj */
        glUseProgram(program);
        glBindVertexArray(vArr);
        glDrawArrays(GL_POINTS, 0, mesh.NV());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glfwTerminate();
    return 0;
}

