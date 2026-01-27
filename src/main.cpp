#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "cyGL.h"
#include "cyTriMesh.h"
#include "cyMatrix.h"
#include <iostream>
#include <fstream>
#include <string>

constexpr const char* V_SHADER_PATH = "res/shaders/vertShader.vert";
constexpr const char* F_SHADER_PATH = "res/shaders/fragShader.frag";

/* Convert Degrees To Radians */
float deg2Rad(float deg) { return deg * (cy::Pi<float>()/180.0f); }

/* GLFW user input callbacks wrapper */
class UserIO {
    public:
        inline static bool leftMouseHeld = false;
        inline static bool rightMouseHeld = false;
        inline static bool recompileShaders = false;
        inline static double xMousePosDelta = 0.0;
        inline static double yMousePosDelta = 0.0;
        inline static double xRelativeMousePosDelta = 0.0;
        inline static double yRelativeMousePosDelta = 0.0;        
        inline static double xMousePos = 0.0;
        inline static double yMousePos = 0.0;

        static void Init(GLFWwindow* window)
        {
            _window = window;
            glfwGetWindowSize(window, &_windowWidth, &_windowHeight);
            glfwSetKeyCallback(window, getKeyInput);
            glfwSetCursorPosCallback(window, getCursorPos);
            glfwSetMouseButtonCallback(window, getMouseButton);
        }
        static void PollMousePos() 
        {
            glfwGetCursorPos(_window, &xMousePos, &yMousePos); 
            glfwGetWindowSize(_window, &_windowWidth, &_windowHeight);
            xMousePosDelta = xMousePos - _xMousePosLast; 
            yMousePosDelta = yMousePos - _yMousePosLast;
            xRelativeMousePosDelta = xMousePosDelta / _windowWidth;
            yRelativeMousePosDelta = yMousePosDelta / _windowHeight;
            _xMousePosLast = xMousePos;
            _yMousePosLast = yMousePos;   
        }

    private:
        inline static GLFWwindow* _window = nullptr;
        inline static double _xMousePosLast = 0.0;
        inline static double _yMousePosLast = 0.0;
        inline static int _windowHeight;
        inline static int _windowWidth;

        static void getCursorPos(GLFWwindow* window, double xMousePos, double yMousePos) { }

        static void getMouseButton(GLFWwindow *window, int button, int action, int mods)
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) { leftMouseHeld = true; }
            else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) { leftMouseHeld = false; }

            if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) { rightMouseHeld = true; }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) { rightMouseHeld = false; }
        }
        static void getKeyInput(GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) { glfwSetWindowShouldClose(window, true); }
            if (key == GLFW_KEY_F6 && action == GLFW_PRESS) { recompileShaders = true; }
        }
};

class Camera{
    public:
        float radius = 3.0;
        float radialSpeed = 2.0;
        cy::Vec3f worldX = cy::Vec3f(1, 0, 0);
        cy::Vec3f worldY = cy::Vec3f(0, 1, 0);
        cy::Vec3f worldZ = cy::Vec3f(0, 0, 1);
        cy::Matrix4f currentOrientation = cy::Matrix4f::Identity();
        cy::Matrix4f currentPosition = cy::Matrix4f::Identity();
    
        void update(float phiDelta, float thetaDelta, float radiusDelta)
        {
            cy::Vec3f thetaDirection = cy::Matrix3f(currentOrientation) * worldX;
            cy::Vec3f phiDirection = worldY;
            cy::Matrix4f rotationTheta = cy::Matrix4f::Rotation(thetaDirection, thetaDelta);
            cy::Matrix4f rotationPhi = cy::Matrix4f::Rotation(phiDirection, phiDelta);
            currentOrientation = rotationPhi * rotationTheta * currentOrientation;

            radius = cy::Max(0.0f, radialSpeed*radiusDelta + radius);
            cy::Vec3f radiusDirection = cy::Matrix3f(currentOrientation) * worldZ;
            currentPosition = cy::Matrix4f::Translation(radius*radiusDirection);
        }
        cy::Matrix4f getViewMatrix()
        {
            return (currentPosition * currentOrientation).GetInverse();
        }
};

class AnimatedRGB {
    public:
        const double animationDuration = 1.0;
        float r = 1.0f;
        float g = 0.0f;
        float b = 0.0f;

        AnimatedRGB(int seed = 1) { std::srand(seed); }

        // Generate random float between 0 and 1
        float getRandomFloat()
        {
            return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        }
        void pollAnimation()
        {
            if (_timeElapsed > animationDuration) 
            {
                _animationStartTime = glfwGetTime();
                r = getRandomFloat();
                g = getRandomFloat();
                b = getRandomFloat();
            }
            _timeElapsed = glfwGetTime() - _animationStartTime;
        }

    private:
        double _timeElapsed = 0.0;
        double _animationStartTime = 0.0;

};

int main(int argc, char** argv) 
{
    /* Parse Command Line Arguements */
    if(argc != 2) 
    {
        std::cerr << "A single argument of a path to a obj file is expected" << std::endl;
        return -1;
    }
    std::string objFilePath(argv[1]);

    /* Initialize a GLFW Window */
    int glfwErrorCode = glfwInit();
    if (GLFW_TRUE != glfwErrorCode)
    {
        std::cerr << "Could not initialize GLFW" << std::endl;
        return -1;
    }

    /* Create a GLFW Window */
    int windowWidth = 1280;
    int windowHeight = 960;
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "InteractiveGraphicsProject", NULL, NULL);
    if (!window) {
        std::cerr << "Could not initialize a GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    /* Initialize GLEW for using OpenGL Functions */
    GLenum glewErrorCode = glewInit();
    if (GLEW_OK != glewErrorCode) {
        std::cerr << "Could not initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    /* Load Shaders */
    cy::GLSLProgram program;
    bool shaderIsReady = program.BuildFiles(V_SHADER_PATH, F_SHADER_PATH);
    if (!shaderIsReady)
    {
        glfwTerminate();
        std::cerr << "Failed to compile shaders" << std::endl;
        return -1;          
    }

    /* Parse Mesh from File and Prepare Model Matrix from Mesh Bounding Box */
    cy::TriMesh mesh;
    bool meshIsReady = mesh.LoadFromFileObj(objFilePath.c_str());
    if (!meshIsReady)
    {
        std::cerr << "Could not load mesh from obj file at path " << objFilePath << std::endl;
        glfwTerminate();
        return -1;        
    }
    mesh.ComputeBoundingBox();
    cy::Vec3f meshCenter = (mesh.GetBoundMax() + mesh.GetBoundMin())/2;
    cy::Matrix4f meshScale = cy::Matrix4f::Scale(cy::Vec3f(0.05f, 0.05f, 0.05f));
    cy::Matrix4f meshRotationX = cy::Matrix4f::RotationX(deg2Rad(-90));
    cy::Matrix4f meshRotationY = cy::Matrix4f::RotationY(deg2Rad(90));
    cy::Matrix4f meshToOrigin = cy::Matrix4f::Translation(-meshCenter);
    cy::Matrix4f modelMatrix = meshRotationY * meshRotationX * meshScale * meshToOrigin;

    /* Write Mesh to GPU */
    GLuint posLocation = 0;
    const void* posOffset = 0;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLenum usage = GL_STATIC_DRAW;
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &vbo); 
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.NV()*sizeof(cy::Vec3f), &mesh.V(0), usage);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.NF()*sizeof(cy::TriMesh::TriFace), &mesh.F(0), usage);
    
    glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, sizeof(cy::Vec3f), posOffset);
    glEnableVertexAttribArray(posLocation);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);  
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    /* Execute GLFW Window */
    float thetaDelta = 0.0f;
    float phiDelta = 0.0f;
    float radiusDelta = 0.0f;

    float fovRadians = deg2Rad(75);
    float zFar = 1000;
    float zNear = 0.1f;

    Camera camera;
    AnimatedRGB animatedRgb;
    UserIO::Init(window);

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {        
        // Animated Background (disabled b/c distracting)        
        //animatedRgb.pollAnimation();
        //glClearColor(animatedRgb.r, animatedRgb.g, animatedRgb.b, 1.0); 

        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        glViewport(0, 0, windowWidth, windowHeight);
        glClearColor(0, 0, 0, 1.0); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (UserIO::recompileShaders)
        {
            // cmpile
            bool shaderIsReady = program.BuildFiles(V_SHADER_PATH, F_SHADER_PATH);
            if (shaderIsReady) { std::cout << "Shaders recompiled successfully" << std::endl; }
            else { std::cerr << "Failed to recompile shaders" << std::endl; glfwSetWindowShouldClose(window, true); }
            UserIO::recompileShaders = false;
        }

        // View Matrix
        UserIO::PollMousePos();
        thetaDelta = 0.0f;
        phiDelta = 0.0f;
        radiusDelta = 0.0f;
        if (UserIO::leftMouseHeld) 
        {
            thetaDelta = static_cast<float>(UserIO::yRelativeMousePosDelta)*(2*cy::Pi<float>());
            phiDelta = static_cast<float>(UserIO::xRelativeMousePosDelta)*(2*cy::Pi<float>());
        }
        if (UserIO::rightMouseHeld)
        {
            radiusDelta = static_cast<float>(UserIO::yRelativeMousePosDelta);
        }        
        camera.update(phiDelta, thetaDelta, radiusDelta);
        cy::Matrix4f viewMatrix = camera.getViewMatrix();
        
        // Perspective Matrix
        float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
        cy::Matrix4f perspectiveMatrix = cy::Matrix4f::Perspective(fovRadians, aspect, zNear, zFar);

        // Final MVP Passed as Uniform Value to Shaders
        cy::Matrix4f mvp = perspectiveMatrix * viewMatrix * modelMatrix;

        /* Bind Program and Draw */
        program.Bind();
        program.SetUniformMatrix4("mvp", &mvp.cell[0]);
        glBindVertexArray(vao);
        //glDrawArrays(GL_POINTS, 0, mesh.NV());
        glDrawElements(GL_POINTS, mesh.NF()*3, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        /* Display Final Render */
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

