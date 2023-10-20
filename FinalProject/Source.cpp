#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <camera.h> // Camera class

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Final Project Jesse Draper"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao[10];         // Handle for the vertex array object
        GLuint vbo[10];      // Handle for the vertex buffer object
        GLuint nVertices[10];    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data seperate for each seperate mesh
    GLMesh gMesh;
    // Texture
    GLuint boxTexture, planeTexture, pyramidTexture;
    glm::vec2 gUVScale(1.0f, 1.0f);
    GLint gTexWrapMode = GL_REPEAT;
    //
    GLuint gBoxProgramId;
    GLuint gLampProgramId;
    bool viewProjection = true;

    // Cube and light color
    //glm::vec3 gObjectColor(0.6f, 0.5f, 0.75f);
    glm::vec3 gObjectColor(1.0f, 0.2f, 0.8f);
    glm::vec3 gLightColor(0.8f, 0.8f, 0.8f);
    glm::vec3 gLightColor2(0.0f, 0.0f, 0.9f);

    // Light position 
    glm::vec3 gLightPosition(10.5f, 20.5f, 10.0f);
    glm::vec3 gLightPosition2(15.5f, 25.5f, 11.0f);

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 10.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // Subject position and scale
    glm::vec3 gBoxPosition(1.0f,-0.5f, -4.0f);
    glm::vec3 gBoxScale(0.5f, 0.5f, 0.5f);

    glm::vec3 gBoxPosition2(0.6f, -0.64f, -4.1f);
    glm::vec3 gBoxScale2(0.25f, 0.25f, 0.25f);
    
    glm::vec3 gLampPosition(5.0f,10.0f, 20.0f);
    glm::vec3 gLampScale(0.25f,0.25f,0.25f);

    glm::vec3 gPlanePosition(-4.0f, -0.8f, 0.0f);
    glm::vec3 gPlaneScale(15.0f,10.0f, 10.0f);

    glm::vec3 gPyramidPosition(-3.0f,-1.0f,-1.0f);
    glm::vec3 gPyramidScale(3.0f,5.0f,2.0f);

    glm::vec3 gPyramidPosition2(-8.0f, -1.0f, -1.0f);
    glm::vec3 gPyramidScale2(3.0f, 5.0f, 2.0f);
        

    glm::vec3 gLastObjectPosition(0.0f, 0.0f, 0.0f);
    glm::vec3 gLastObjectScale(1.0f, 1.0f, 1.0f);

}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

//add shader code here FOR EACH OBJECT light and object

/* box Vertex Shader Source Code*/
const GLchar* boxVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Cube Fragment Shader Source Code*/
const GLchar* boxFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightColor2;
uniform vec3 lightPos;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform vec2 uvScale;

void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

 //Calculate Ambient lighting*/
    float ambientStrength = 0.4f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity = 0.8f; // Set specular light strength
    float highlightSize = 5.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;

    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);


/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

        //Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Lamp Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);



// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

    // Create the shader programs NEED TO ADD ONE FOR EACH SHADER BEING CREATED BOX,Plane,Lamp, Pyramid, lastObj
    if (!UCreateShaderProgram(boxVertexShaderSource, boxFragmentShaderSource, gBoxProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;



    // Load texture one for each object

    const char* boxTextureFile = "../resources/box.png";
    if (!UCreateTexture(boxTextureFile, boxTexture))
    {
        cout << "Failed to load texture " << boxTextureFile << endl;
        return EXIT_FAILURE;
    }
    //wood box
    const char* planeTextureFile = "../resources/metal.png";
    if (!UCreateTexture(planeTextureFile, planeTexture))
    {
        cout << "Failed to load texture " << planeTexture << endl;
       return EXIT_FAILURE;
    }
    //metal plane
    const char* pyramidTextureFile = "../resources/metal.png";
    if (!UCreateTexture(pyramidTextureFile, pyramidTexture))
    {
        cout << "Failed to load texture " << pyramidTexture << endl;
        return EXIT_FAILURE;
   }
    //metal pyramid


    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gBoxProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gBoxProgramId, "uTexture"), 0);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);

    // Release texture
    UDestroyTexture(boxTexture);
    UDestroyTexture(planeTexture);
    UDestroyTexture(pyramidTexture);

    // Release shader programs
    UDestroyShaderProgram(gBoxProgramId);
    UDestroyShaderProgram(gLampProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
     // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.Position.y -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.Position.y += 0.01f;
    //implement view change
    	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		viewProjection = !viewProjection;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset == 1.0f) {
        gCamera.MovementSpeed = gCamera.MovementSpeed * 2;
    }
    if (yoffset == -1.0f) {
        gCamera.MovementSpeed = gCamera.MovementSpeed / 2;
    }
}



// Functioned called to render a frame
void URender()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the shader to be used
    glUseProgram(gBoxProgramId);

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();


    //draw box
    glUseProgram(gBoxProgramId);

    glm::mat4 projection;
	if (viewProjection) {
		projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	}
	else {
		float scale = 40;
		projection = glm::ortho((800.0f / scale), -(800.0f / scale), -(600.0f / scale), (600.0f / scale), -2.5f, 6.5f);
	}


#pragma region BoxConstruction

    glm::mat4 model = glm::translate(gBoxPosition) * glm::scale(gBoxScale);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gBoxProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gBoxProgramId, "view");
    GLint projLoc = glGetUniformLocation(gBoxProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Uniform / Global variables for object color, light color, light position, and camera/view position
    GLint  objectColorLoc = glGetUniformLocation(gBoxProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gBoxProgramId, "lightColor");
    GLint lightPosLoc = glGetUniformLocation(gBoxProgramId, "lighPos");
    GLint viewPositionLoc = glGetUniformLocation(gBoxProgramId, "viewPosition");
  
    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPosLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gBoxProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[0]);
    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, boxTexture);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[0]);


    model = glm::translate(gBoxPosition2) * glm::scale(gBoxScale2);
    modelLoc = glGetUniformLocation(gBoxProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[0]);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[0]);
#pragma endregion 

#pragma region PyramidConstruction

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pyramidTexture);
    model = glm::translate(gPyramidPosition) * glm::scale(gPyramidScale);
    modelLoc = glGetUniformLocation(gBoxProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[1]);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pyramidTexture);
    model = glm::translate(gPyramidPosition2) * glm::scale(gPyramidScale2);
    modelLoc = glGetUniformLocation(gBoxProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[1]);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[1]);

#pragma endregion

#pragma region PlaneConstruction

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planeTexture);
    model = glm::translate(gPlanePosition) * glm::scale(gPlaneScale);
    modelLoc = glGetUniformLocation(gBoxProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[2]);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[2]);

#pragma endregion

#pragma region LightConstruction

    glUseProgram(gLampProgramId);
    model = glm::translate(gLightPosition) * glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glBindVertexArray(gMesh.vao[0]);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[0]);


    glUseProgram(gLampProgramId);
    model = glm::translate(gLightPosition2) * glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glBindVertexArray(gMesh.vao[0]);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[0]);

#pragma endregion

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    // seperate meshes for each object
    GLfloat cubeverts[] = {
        //Positions          //Normals
        // --------------------------------------
        //Back Face          //Negative Z Normals
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,

       //Front Face         //Positive Z Normals
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,

      //Left Face          //Negative X Normals
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

     //Right Face         //Positive X Normals
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

     //Bottom Face        //Negative Y Normals
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,

    //Top Face           //Positive Y Normals
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
    };

    //pyramid points
    GLfloat pyramidverts[] = {
        //Positions          //Normals
        // ------------------------------------------------------
        //Back Face          //Negative Z Normal  Texture Coords.
        -0.5f,0.0f,0.5f,  0.0f,  0.0f, -1.0f,    0.0f, 0.0f,
        0.5f, 0.0f, 0.5f,  0.0f,  0.0f,-1.0f,   1.0f, 0.0f,
        0.0f, 0.5f, 0.0f,  0.0f,  0.0f, -1.0f,    0.5f, 1.0f,


        //Left Face         //Positive Z Normal
       -0.5f,0.0f,0.5f,  0.0f,  0.0f,  1.0f,      1.0f,0.0f,
       -0.5f,0.0f,-0.5f, 0.0f,  0.0f,  1.0f,      0.0f,0.0f,
        0.0f,0.5f,0.0f,   0.0f,  0.0f,  1.0f,      0.5f,1.0f,


        //Back Face          //Negative X Normal
       -0.5f,0.0f,-0.5f, -1.0f,  0.0f,  0.0f,       0.0f,0.0f,
        0.5f,0.0f,-0.5f,  -1.0f,  0.0f,  0.0f,     1.0f, 0.0f,
        0.0f,0.5f,0.0f,   -1.0f,  0.0f,  0.0f,      0.5f, 1.0f,


        //Right Face         //Positive X Normal
        0.5f,0.0f,-0.5f,   1.0f,  0.0f,  0.0f,       0.0f, 0.0f,
        0.5f,0.0f,0.5f,    1.0f,  0.0f,  0.0f,       1.0f, 0.0f,
        0.0f,0.5f,0.0f,    1.0f,  0.0f,  0.0f,       0.5f, 1.0f,


        //Bottom 1 Face        //Negative Y Normal
       0.5f,0.0f,-0.5f,      0.0f, -1.0f,  0.0f,     0.0f, 0.0f,
       0.5f,0.0f,0.5f,       0.0f, -1.0f,  0.0f,     1.0f, 0.0f,
        -0.5f,0.0f,-0.5f,    0.0f, -1.0f,  0.0f,     0.5f, 1.0f,


        //Bottom 2 Face           //positive Y Normal
        -0.5f,0.0f,-0.5f,  0.0f, 1.0f,  0.0f,      0.0f,0.0f,
        -0.5f,0.0f,0.5f,   0.0f, 1.0f,  0.0f,       1.0f, 0.0f,
        0.5f,0.0f,0.5f,    0.0f, 1.0f,  0.0f,      0.5f,1.0f,
    };

    //plane verts
    GLfloat planeverts[] = {
   -0.5f,0.0f, 0.5f,    0.0f,  -1.0f,  0.0f, 0.0f, 0.0f, //plane wip
   -0.5f,0.0f,-0.5f,     0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
   0.5f,0.0f,-0.5f,      0.0f,  0.0f,  -1.0f,1.0f, 0.0f,
   0.5f,0.0f, -0.5f,      0.0f,  0.0f,  1.0f,1.0f, 0.0f,
   0.5f,0.0f,0.5f,      -1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
  -0.5f,0.0f,0.5f,      1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
    };

    GLfloat walls[] = {

        -1.0f,1.0f,0.0f,     0.0f,  -1.0f,  0.0f, 0.0f, 0.0f,
        -1.0f,0.0f,0.0f,     0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
        1.0f,0.0f,0.0f,      0.0f,  0.0f,  -1.0f,1.0f, 0.0f,
        1.0f,0.0f,0.0f,      0.0f,  0.0f,  1.0f,1.0f, 0.0f,
        1.0f,1.0f,0.0f,     -1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
        -1.0f,1.0f,0.0f,     1.0f,  1.0f,  0.0f, 1.0f, 1.0f,

    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);



    mesh.nVertices[0] = sizeof(cubeverts) / (sizeof(cubeverts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    mesh.nVertices[1] = sizeof(pyramidverts) / (sizeof(pyramidverts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    mesh.nVertices[2] = sizeof(planeverts) / (sizeof(planeverts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    mesh.nVertices[3] = sizeof(walls) / (sizeof(walls[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

#pragma region BoxMesh

    glGenVertexArrays(1, &mesh.vao[0]);
    glGenBuffers(1, &mesh.vbo[0]);
    glBindVertexArray(mesh.vao[0]);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeverts), cubeverts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

#pragma endregion

#pragma region PyramidMesh

    glGenVertexArrays(1, &mesh.vao[1]);
    glGenBuffers(1, &mesh.vbo[1]);
    glBindVertexArray(mesh.vao[1]);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[1]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidverts), pyramidverts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

#pragma endregion

#pragma region planeMesh

    glGenVertexArrays(1, &mesh.vao[2]);
    glGenBuffers(1, &mesh.vbo[2]);
    glBindVertexArray(mesh.vao[2]);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[2]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeverts), planeverts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

                                                                                   // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

#pragma endregion

#pragma region wallMesh



#pragma endregion
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao[10]);
    glDeleteBuffers(1, &mesh.vbo[10]);
}


/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}
