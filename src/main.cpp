#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "cyGL.h"
#include "cyTriMesh.h"
#include "cyMatrix.h"
#include "cyQuat.h"
#include "lodepng.h"

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
        inline static bool controlPressed = false;
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
            if ( (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_LEFT_CONTROL) && action == GLFW_PRESS) { controlPressed = true; }
            else if ( (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_LEFT_CONTROL) && action == GLFW_RELEASE ) { controlPressed = false; }
        }
};

class OrbitalObject {
    public:
        float radius = 3.0;
        float radialSpeed = 2.0;
        cy::Vec3f worldX = cy::Vec3f(1, 0, 0);
        cy::Vec3f worldY = cy::Vec3f(0, 1, 0);
        cy::Vec3f worldZ = cy::Vec3f(0, 0, 1);
        cy::Quatf currentOrientation = cy::Quatf(1, 0, 0, 0);
        cy::Vec3f currentPosition = cy::Vec3f(0, 0, radius);
    
        void update(float phiDelta, float thetaDelta, float radiusDelta)
        {
            cy::Vec3f thetaDirection = (currentOrientation.ToMatrix3() * worldX).GetNormalized();
            cy::Vec3f phiDirection = worldY;
            
            cy::Quatf rotationTheta;
            cy::Quatf rotationPhi;
            rotationTheta.SetRotation(thetaDelta, thetaDirection);
            rotationPhi.SetRotation(phiDelta, phiDirection);
            currentOrientation = rotationPhi * rotationTheta * currentOrientation;
            currentOrientation.Normalize();

            cy::Vec3f radiusDirection = (currentOrientation.ToMatrix3() * worldZ).GetNormalized();
            radius = cy::Max(0.0f, radialSpeed*radiusDelta + radius);
            currentPosition = radius*radiusDirection;
        }
        cy::Matrix4f getViewMatrix()
        {
            cy::Matrix4f positionTranslation = cy::Matrix4f::Translation(currentPosition);
            cy::Matrix4f orientationRotation = currentOrientation.ToMatrix4();
            return (positionTranslation * orientationRotation).GetInverse();
        }
        cy::Matrix4f getModelMatrix()
        {
            cy::Matrix4f positionTranslation = cy::Matrix4f::Translation(currentPosition);
            cy::Matrix4f orientationRotation = currentOrientation.ToMatrix4();
            return (positionTranslation * orientationRotation);
        }
};


int main(int argc, char** argv) 
{
    /* Parse Command Line Arguements */
    if(argc != 2) 
    {
        std::cerr << "A single argument of a path to a obj file is expected" << std::endl;
        return -1;
    }
    std::filesystem::path objFilePath(argv[1]);

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
    bool meshIsReady = mesh.LoadFromFileObj(objFilePath.string().c_str(), true);
    if (!meshIsReady)
    {
        std::cerr << "Could not load mesh from obj file at path: " << objFilePath << std::endl;
        glfwTerminate();
        return -1;        
    }
    if ( !(mesh.HasNormals() && mesh.HasTextureVertices()) )
    {
        std::cerr << "Could find vertex normal or vertex texture uv's at: " << objFilePath << std::endl;
        glfwTerminate();
        return -1;        
    }
    if ( mesh.NM() == 0)
    {
        std::cerr << "Count not find at least one .mtl file in the same directory as: " << objFilePath << std::endl;
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

    /* Load Textures */
    std::filesystem::path diffuseTextureFilePath = objFilePath.parent_path() / std::filesystem::path(mesh.M(0).map_Kd.data);
    std::vector<unsigned char> diffuseTextureData; 
    unsigned diffuseTextureWidth, diffuseTextureHeight;
    unsigned pngLodeErrorDiffuse = lodepng::decode(diffuseTextureData, diffuseTextureWidth, diffuseTextureHeight, diffuseTextureFilePath.string());
    if(pngLodeErrorDiffuse)
    {
        std::cerr << "Attempted to find diffuse texture at according to .mtl file: " << diffuseTextureFilePath.string() << std::endl;
        std::cerr << "Could not load diffuse texture: " << pngLodeErrorDiffuse << ": " << lodepng_error_text(pngLodeErrorDiffuse) << std::endl;
        glfwTerminate();
        return -1;       
    }
    std::filesystem::path specularTextureFilePath = objFilePath.parent_path() / std::filesystem::path(mesh.M(0).map_Ks.data);
    std::vector<unsigned char> specularTextureData; 
    unsigned specularTextureWidth, specularTextureHeight;
    unsigned pngLodeErrorSpecular = lodepng::decode(specularTextureData, specularTextureWidth, specularTextureHeight, specularTextureFilePath.string());
    if(pngLodeErrorSpecular)
    {
        std::cerr << "Attempted to find specular texture at according to .mtl file: " << specularTextureFilePath.string() << std::endl;
        std::cerr << "Could not load specular texture: " << pngLodeErrorSpecular << ": " << lodepng_error_text(pngLodeErrorSpecular) << std::endl;
        glfwTerminate();
        return -1;       
    }

    /* Write Mesh to GPU */
    std::vector<cy::Vec3f> positions( mesh.NF()*3 );
    std::vector<cy::Vec3f> normals( mesh.NF()*3 );
    std::vector<cy::Vec3f> uvs( mesh.NF()*3 );

    // For each face find the vertices, normals, and uvs and put them in lists ordered by face index
    for (unsigned int faceIndex = 0; faceIndex < mesh.NF(); faceIndex++)
    {
        cy::TriMesh::TriFace facePositions = mesh.F(faceIndex);
        cy::TriMesh::TriFace faceNormals = mesh.FN(faceIndex);
        cy::TriMesh::TriFace faceUvs = mesh.FT(faceIndex);
        for (int indexOnFace = 0; indexOnFace < 3; indexOnFace++)
        {
            unsigned int vertexPositionIndex = facePositions.v[indexOnFace];
            unsigned int vertexNormalIndex = faceNormals.v[indexOnFace];
            unsigned int vertexUvIndex = faceUvs.v[indexOnFace];

            positions.push_back( mesh.V(vertexPositionIndex) );
            normals.push_back( mesh.VN(vertexNormalIndex) );
            uvs.push_back( mesh.VT(vertexUvIndex) );
        }    
    }

    GLuint vao;
    GLuint positionBuffer;
    GLuint normalBuffer;
    GLuint uvBuffer;
    GLuint positionLocation = 0;
    GLuint normalLocation = 1;
    GLuint uvLocation = 2;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &positionBuffer); 
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, positions.size()*sizeof(cy::Vec3f), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(cy::Vec3f), 0);
    glEnableVertexAttribArray(positionLocation);

    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size()*sizeof(cy::Vec3f), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(cy::Vec3f), 0);
    glEnableVertexAttribArray(normalLocation);

    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size()*sizeof(cy::Vec3f), uvs.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(uvLocation, 3, GL_FLOAT, GL_FALSE, sizeof(cy::Vec3f), 0);
    glEnableVertexAttribArray(uvLocation);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);  
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    /* Write Textures to GPU */
    cy::GLTexture2D texDiffuse;
    int texDiffuseUnit = 1;
    texDiffuse.Initialize();
    texDiffuse.SetImage<unsigned char>(&diffuseTextureData[0], 4, diffuseTextureWidth, diffuseTextureHeight);
    texDiffuse.BuildMipmaps();

    cy::GLTexture2D texSpecular;
    int texSpecularUnit = 2;
    texSpecular.Initialize();
    texSpecular.SetImage<unsigned char>(&specularTextureData[0], 4, specularTextureWidth, specularTextureHeight);
    texSpecular.BuildMipmaps();
    
    cy::GLTexture2D texAmbient;
    int texAmbientUnit = 3;
    texAmbient.Initialize();
    texAmbient.SetImage<unsigned char>(&diffuseTextureData[0], 4, diffuseTextureWidth, diffuseTextureHeight);
    texAmbient.BuildMipmaps();

    /* Data on Scene */
    OrbitalObject camera;
    OrbitalObject light;

    float fovRadians = deg2Rad(75);
    float zFar = 1000;
    float zNear = 0.1f;

    float lightIntensity = 1.0f;
    float lightAmbientIntensity = 0.1f;

    /* Execute GLFW Window */
    UserIO::Init(window);
    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {        
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        glViewport(0, 0, windowWidth, windowHeight);
        glClearColor(0, 0, 0, 1.0); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (UserIO::recompileShaders)
        {
            bool shaderIsReady = program.BuildFiles(V_SHADER_PATH, F_SHADER_PATH);
            if (shaderIsReady) { std::cout << "Shaders recompiled successfully" << std::endl; }
            else { std::cerr << "Failed to recompile shaders" << std::endl; glfwSetWindowShouldClose(window, true); }
            UserIO::recompileShaders = false;
        }

        // View Matrix
        UserIO::PollMousePos();
        float thetaDelta = 0.0f;
        float phiDelta = 0.0f;
        float radiusDelta = 0.0f;
        if (UserIO::leftMouseHeld && !UserIO::controlPressed) 
        {
            thetaDelta = static_cast<float>(UserIO::yRelativeMousePosDelta)*(2*cy::Pi<float>());
            phiDelta = static_cast<float>(UserIO::xRelativeMousePosDelta)*(2*cy::Pi<float>());
        }
        if (UserIO::rightMouseHeld && !UserIO::controlPressed)
        {
            radiusDelta = static_cast<float>(UserIO::yRelativeMousePosDelta);
        }        
        camera.update(phiDelta, thetaDelta, radiusDelta);
        cy::Matrix4f viewMatrix = camera.getViewMatrix();
        
        if (UserIO::leftMouseHeld && UserIO::controlPressed) 
        {
            thetaDelta = static_cast<float>(UserIO::yRelativeMousePosDelta)*(2*cy::Pi<float>());
            phiDelta = static_cast<float>(UserIO::xRelativeMousePosDelta)*(2*cy::Pi<float>());
        }
        if (UserIO::rightMouseHeld && UserIO::controlPressed)
        {
            radiusDelta = static_cast<float>(UserIO::yRelativeMousePosDelta);
        }        
        light.update(phiDelta, thetaDelta, radiusDelta);

        // Perspective Matrix
        float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
        cy::Matrix4f perspectiveMatrix = cy::Matrix4f::Perspective(fovRadians, aspect, zNear, zFar);

        // Final MVP Passed as Uniform Value to Shaders
        cy::Matrix4f mvp = perspectiveMatrix * viewMatrix * modelMatrix;
        cy::Matrix4f mv = viewMatrix * modelMatrix;
        cy::Matrix3f mvNormal = cy::Matrix3f(viewMatrix * modelMatrix).GetInverse().GetTranspose();
        cy::Vec3f lightPositionView = cy::Vec3f( viewMatrix * cy::Vec4f(light.currentPosition, 1.0) );

        /* Bind Program and Uniforms then Draw */
        program.Bind();
        
        program.SetUniformMatrix4("mvp", &mvp.cell[0]);
        program.SetUniformMatrix4("mv", &mv.cell[0]);
        program.SetUniformMatrix3("mvNormal", &mvNormal.cell[0]);
        
        program.SetUniform3("lightPosition", &lightPositionView[0]);
        program.SetUniform1("lightIntensity", &lightIntensity);
        program.SetUniform1("lightAmbientIntensity", &lightAmbientIntensity);
        
        texDiffuse.Bind(texDiffuseUnit);
        program.SetUniform("mapKd", texDiffuseUnit);
        texSpecular.Bind(texSpecularUnit);
        program.SetUniform("mapKs", texSpecularUnit);
        texSpecular.Bind(texAmbientUnit);
        program.SetUniform("mapKa", texAmbientUnit);
        
        program.SetUniform1("mtl_Ns", &mesh.M(0).Ns);
        program.SetUniform3("mtl_Ka", &mesh.M(0).Ka[0]);
        program.SetUniform3("mtl_Kd", &mesh.M(0).Kd[0]);
        program.SetUniform3("mtl_Ks", &mesh.M(0).Ks[0]);
        
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(positions.size()) );
        glBindVertexArray(0);

        /* Display Final Render */
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}