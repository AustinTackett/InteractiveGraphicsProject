#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "cyCore.h"
#include "cyTriMesh.h"
#include "cyMatrix.h"
#include "cyQuat.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <algorithm>

#define V_SHADER_PATH "res/shaders/vertShader.glsl"
#define F_SHADER_PATH "res/shaders/fragShader.glsl"

/* GLFW user input callbacks wrapper */
class UserIO {
    public:
        inline static bool leftMouseHeld = false;
        inline static bool rightMouseHeld = false;

        static void getCursorPos(GLFWwindow* window, double xMousePos, double yMousePos) { }

        static void getMouseButton(GLFWwindow *window, int button, int action, int mods)
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) { leftMouseHeld = true; }
            else { leftMouseHeld = false; }

            if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) { rightMouseHeld = true; }
            else { rightMouseHeld = false; }
        }

        static void getKeyInput(GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            if (key == GLFW_KEY_ESCAPE) { 
                glfwSetWindowShouldClose(window, true); 
            }
        }
};

class RandomFloat {
    public:
        RandomFloat() 
        {
            std::srand(1);
        }

        // Generate random float between 0 and 1
        float getRandomFloat()
        {
            return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
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
    mesh.ComputeBoundingBox();

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
    RandomFloat randGen;
    const double animationDuration = 1.0;
    double timeElapsed = 0.0;
    double animationStartTime = 0.0;
    float r = 1.0f;
    float g = 0.0f;
    float b = 0.0f;

    /* Execute GLFW Window */
    double lastMousePosX = 0.0;
    double lastMousePosY = 0.0;
    double relativeMouseDeltaX, relativeMouseDeltaY;

    float currentAngleTheta = deg2Rad(90);
    float currentAnglePhi = 0.0f;
    float currentOffset = 3.0;

    cy::Quatf rotationPhi;
    cy::Quatf rotationTheta;

    cy::Vec3f phiDirection(1, 0, 0);
    cy::Vec3f thetaDirection(0, 1, 0);

    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        /* Animate Background */
        if (timeElapsed > animationDuration) 
        {
            animationStartTime = glfwGetTime();
            r = randGen.getRandomFloat();
            g = randGen.getRandomFloat();
            b = randGen.getRandomFloat();
        }
        timeElapsed = glfwGetTime() - animationStartTime;
        
        /* Ready Window To Render Obj */
        glUseProgram(program);
        
        //glClearColor(r, g, b, 1.0f);
        glClearColor(0, 0, 0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Get Cursor Position */
        glfwGetCursorPos(window, &xMousePos, &yMousePos); 
        relativeMouseDeltaX = (xMousePos - lastMousePosX) / windowWidth;  
        relativeMouseDeltaY = (yMousePos - lastMousePosY) / windowHeight;
        lastMousePosX = xMousePos;
        lastMousePosY = yMousePos;
        
        /* Calculate Final MVP Matrix */
        cy::Vec3f meshCenter = (mesh.GetBoundMax() + mesh.GetBoundMin())/2;

        cy::Matrix4f meshScale = cy::Matrix4f::Scale(cy::Vec3f(0.05f, 0.05f, 0.05f));
        cy::Matrix4f meshRotationX = cy::Matrix4f::RotationX(deg2Rad(-90));
        cy::Matrix4f meshToOrigin = cy::Matrix4f::Translation(-meshCenter);
        cy::Matrix4f meshfromOrigin = cy::Matrix4f::Translation(meshCenter);
        
        cy::Matrix4f modelMatrix = meshRotationX * meshScale * meshToOrigin;

        // View Matrix
        float angleThetaMaxDeg = 180.0f;
        float anglePhiMaxDeg = 360.0f;
        cy::Vec3f cameraTarget(0, 0, 0);

        if (UserIO::leftMouseHeld) 
        {
            currentAngleTheta += static_cast<float>(relativeMouseDeltaY)*deg2Rad(angleThetaMaxDeg);
            currentAnglePhi += static_cast<float>(relativeMouseDeltaX)*deg2Rad(anglePhiMaxDeg);
        }
        if (UserIO::rightMouseHeld)
        {
            currentOffset += static_cast<float>(relativeMouseDeltaY);
            currentOffset = std::max(0.0f, currentOffset);
        }
        rotationTheta.SetRotation(currentAngleTheta, thetaDirection);
        rotationPhi.SetRotation(currentAnglePhi, phiDirection);
        
        float cameraPositionX = currentOffset * std::sinf(currentAngleTheta)*std::cosf(currentAnglePhi);
        float cameraPositionZ = currentOffset * std::sinf(currentAngleTheta)*std::sinf(currentAnglePhi);
        float cameraPositionY = currentOffset * std::cosf(currentAngleTheta);

        cy::Vec3f cameraPosition(cameraPositionX, cameraPositionY, cameraPositionZ);
        cy::Matrix4f viewMatrix = cy::Matrix4f::View(cameraPosition, cameraTarget, thetaDirection);
        
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

