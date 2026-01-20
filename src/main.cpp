#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "cyCore.h"
#include "cyTriMesh.h"
#include "cyMatrix.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

#define V_SHADER_PATH "res/shaders/vertShader.glsl"
#define F_SHADER_PATH "res/shaders/fragShader.glsl"

/* GLFW user input callbacks wrapper */
class UserIO {
    public:
        inline static bool leftMouseHeld = false;

        static void getCursorPos(GLFWwindow* window, double xMousePos, double yMousePos) { }

        static void getMouseButton(GLFWwindow *window, int button, int action, int mods)
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
            { 
                leftMouseHeld = true; 
            }
            else 
            { 
                leftMouseHeld = false; 
            }
        }

        static void getKeyInput(GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            if (key == GLFW_KEY_ESCAPE) { 
                glfwSetWindowShouldClose(window, true); 
            }
        }
};

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

/* Convert Degrees To Radians */
float deg2Rad(float deg)
{
    return deg * (cy::Pi<float>()/180.0f);
}

/* Get Random Float bewteen 0 and 1 */
float randFloat() 
{
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
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
    int windowWidth = 1280;
    int windowHeight = 960;
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "InteractiveGraphicsProject", NULL, NULL);
    if (!window) {
        std::cerr << "Could not initialize a GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    /* Initialize GLFW keyboard callbacks */
    double xMousePos = 0;
    double yMousePos = 0;
    glfwSetKeyCallback(window, UserIO::getKeyInput);
    glfwSetCursorPosCallback(window, UserIO::getCursorPos);
    glfwSetMouseButtonCallback(window, UserIO::getMouseButton);

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
    glEnableVertexAttribArray(vAttribIdx);

    /* Read Vertex Shaders Code From Files */
    std::string vShaderString;
    bool vShaderFileErr = readFile(V_SHADER_PATH, vShaderString);
    if (!vShaderFileErr)
    {
        std::cerr << "Could not load vertex shader at path " << V_SHADER_PATH << std::endl;
        glfwTerminate();
        return -1;   
    }

    /* Read Fragment Shaders Code From Files */
    std::string fShaderString;
    bool fShaderFileErr = readFile(F_SHADER_PATH, fShaderString);
    if (!vShaderFileErr)
    {
        std::cerr << "Could not load fragment shader at path " << F_SHADER_PATH << std::endl;
        glfwTerminate();
        return -1;   
    }

    /* Compile Vertex Shaders */
    GLuint vShader;
    vShader = glCreateShader(GL_VERTEX_SHADER);

    const GLchar* vshaderCharArray = vShaderString.c_str();
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
        glGetShaderInfoLog(fShader, fInfoLogLength, NULL, &fShaderInfolog[0]);
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

    /* Detach and delete shader objects after they are contained in the program */
    glDetachShader(program, vShader);
    glDetachShader(program, fShader);
    glDeleteShader(vShader);
    glDeleteShader(fShader);

    /* Animation Params */
    std::srand(1);
    const double animationDuration = 1.0;
    double timeElapsed = 0.0;
    double animationStartTime = 0.0;
    float r = 1.0f;
    float g = 0.0f;
    float b = 0.0f;

    /* Execute GLFW Window */
    double lastMousePosX, lastMousePosY;
    double relativeMouseDeltaX, relativeMouseDeltaY;
    float currentAngleY = 0.0f;
    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {
        /* Animate Background */
        if (timeElapsed > animationDuration) 
        {
            animationStartTime = glfwGetTime();
            r = randFloat();
            g = randFloat();
            b = randFloat();
        }
        timeElapsed = glfwGetTime() - animationStartTime;
        
        /* Ready Window To Render Obj */
        glUseProgram(program);
        
        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Get Cursor Position */
        glfwGetCursorPos(window, &xMousePos, &yMousePos); 

        // Could be less than 0 and greater than 1 if mouse leaves window 
        if (UserIO::leftMouseHeld)
        {
            relativeMouseDeltaX = (xMousePos - lastMousePosX) / windowWidth;  
            relativeMouseDeltaY = (yMousePos - lastMousePosY) / windowHeight;
        }
        else
        {
            relativeMouseDeltaX = 0.0;
            relativeMouseDeltaY = 0.0;
        }
        lastMousePosX = xMousePos;
        lastMousePosY = yMousePos;
        
        /* Calculate Final MVP Matrix */

        // Model Matrix (move mesh center to origin)
        cy::Vec3f origin = cy::Vec3f(0, 0, 0);
        cy::Matrix4f meshRotationX = cy::Matrix4f::RotationX(deg2Rad(-90));
        
        // Account for that rotation changes mesh center
        cy::Vec3f meshCenter = (mesh.GetBoundMax() + mesh.GetBoundMin())/2; 
        meshCenter = cy::Vec3f(meshRotationX * cy::Vec4f(meshCenter, 1.0f));
        cy::Vec3f meshCenterToOrigin = cy::Vec3f(0, origin.y - meshCenter.y, 0);
        cy::Matrix4f meshTranslation = cy::Matrix4f::Translation(meshCenterToOrigin);

        cy::Matrix4f modelMatrix = meshTranslation * meshRotationX;

        // View Matrix (final matrix not calculated yet)
        currentAngleY += static_cast<float>(relativeMouseDeltaX)*deg2Rad(360);

        float cameraOffset = 2;
        float cameraPositionX = cameraOffset * std::cosf(currentAngleY) + origin.x;
        float cameraPositionZ = cameraOffset * std::sinf(static_cast<float>(currentAngleY)) + origin.z;
        float cameraPositionY = 0;
        cy::Vec3f cameraPosition(cameraPositionX, cameraPositionY, cameraPositionZ);
        cy::Vec3f up(0, 1, 0);
        cy::Matrix4f viewMatrix;
        viewMatrix.SetView(cameraPosition, origin, up);
        
        // Perspective Matrix
        float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
        float fovRadians = deg2Rad(75);
        float zFar = 1000;
        float zNear = 0.1f;
        cy::Matrix4f perspectiveMatrix = cy::Matrix4f::Perspective(fovRadians, aspect, zNear, zFar);

        // Final MVP Passed as Uniform Value to Shaders
        cy::Matrix4f mvp = perspectiveMatrix * viewMatrix * modelMatrix;

        int numMvps = 1;           // To know how much space to buffer
        bool mvpTranspose = false; // Would be true if cy::Matrix was row major instead
        GLint mvpLocation = glGetUniformLocation(program, "mvp");
        glUniformMatrix4fv(mvpLocation, numMvps, mvpTranspose, &mvp.cell[0]);

        /* Draw Obj */
        glBindVertexArray(vArr);
        glDrawArrays(GL_POINTS, 0, mesh.NV());

        /* Display Final Render */
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

