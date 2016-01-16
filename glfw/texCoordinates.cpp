// OpenGL texture convention test
// Use a 4-texel color buffer in memory to show the definition of OpenGL texture coordinates relative to in-memory layout.
// texture coordinates aligned with viewport NDC (xy) directions:
// xy=(-1, -1), st=(0,0)
// xy=(1, -1), st=(1,0)
// xy=(-1, 1), st=(0,1)
// xy=(1, 1), st=(1,1)

// To Build:
// Required packages:
//
// install glfw3 package (in gentoo):
//
// # emerge media-libs/glfw media-libs/glew
//
//
// libs to include: -lglfw -lGLEW -lGLU -lGL
//
// g++ -lglfw -lGLEW -lGLU -lGL

#include <iostream>
#include <cstdlib>
#include <vector>
#include <sstream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>


GLFWwindow* window;

// helper function to load shaders
GLuint loadShaders();

// helper function to compile one shader
//! helper function to compile a shader
//! @params type shader type, could be GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
//! @params a string contain shader source
//! @return shaderID
GLuint compileShader(GLuint type, std::string const& str);

int main()
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "OpenGL Textured Coord", nullptr, nullptr);
    if( window == nullptr ) {
        fprintf( stderr, "Failed to open GLFW window.\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    //glDisable(GL_DEPTH_TEST);

    // Create and compile our GLSL program from the shaders
    GLuint program = loadShaders();

    // Model matrix : an identity matrix (model will be at the origin)

    // Load the texture using any two methods
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    //texture with 4 pixels in RGBA
    char data[]= {
        '\0', '\0', '\0', '\xff',
        '\0', '\xff', '\0', '\xff',
        '\0', '\0', '\xff',  '\xff',
        '\x80', '\x80', '\x80', '\xff'
    };

    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Get a handle for our "uTexSampler" uniform
    GLuint TextureID  = glGetUniformLocation(program, "uTexSampler");

    // vertices:
    //
    // vec3 for vertex coordinates, vec2 for texture coordinates
    static const GLfloat vertex_buffer_data[] = {
        -0.80f,0.80f, 0.0f,      	0.15f, 0.85f,
        -0.80f,-0.80f,0.0f, 		0.15f, 0.15f,
        0.80f,0.80f, 0.0f, 		0.85f, 0.85f,
        0.80f,-0.80f,0.0f, 		0.85f, 0.15f
    };
    //glDisable(GL_CULL_FACE);

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    do {

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Use our shader
        glUseProgram(program);

        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        // Set our "uTexSampler" sampler to user Texture Unit 0
        glUniform1i(TextureID, 0);

        // 1rst attribute buffer : vertice coordinates
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

        GLuint vertexPosition_modelspaceID = glGetAttribLocation(program, "aPosition");
        glEnableVertexAttribArray(vertexPosition_modelspaceID);
        glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE,
                              20, (const GLvoid*)0);
        // texture coordinates
        GLuint vertexUVID = glGetAttribLocation(program, "aTexCoord");
        glEnableVertexAttribArray(vertexUVID);
        glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE,
                              20, (const GLvoid*)12);

        // Draw the triangles !
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // 12*3 indices starting at 0 -> 12 triangles

        glDisableVertexAttribArray(vertexPosition_modelspaceID);
        glDisableVertexAttribArray(vertexUVID);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
            glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteProgram(program);
    glDeleteTextures(1, &textureID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

//! load a trivial shader program
GLuint loadShaders()
{

//Vertex shader
    std::string const vertexShaderSrc {
        R"(#version 120
attribute vec3 aPosition;
attribute vec2 aTexCoord;
varying vec2 tex;
void main() {
gl_Position = vec4(aPosition, 1.);
tex = aTexCoord;
}
)"
};

// Fragment shader
std::string const fragmentShaderSrc {
R"(uniform sampler2D uTexSampler;
varying vec2 tex;
void main() {
gl_FragColor = texture2D(uTexSampler, tex);
})"
    };

    GLint success = GL_FALSE;
    int logSize;

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    if (vertexShader && fragmentShader == 0)
        return 0;

    // Link the program
    printf("Linking program\n");
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check the program
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
    if ( logSize > 0 ) {
        std::vector<char> linkErrorMsg(logSize+1);
        glGetProgramInfoLog(program, logSize, nullptr, &linkErrorMsg[0]);
        printf("%s\n", &linkErrorMsg[0]);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

GLuint compileShader(GLuint type, std::string const& str)
{
    GLuint ret = glCreateShader(type);
    char const * shaderSrc = str.c_str();
    glShaderSource(ret, 1, &shaderSrc , nullptr);
    glCompileShader(ret);

    // Check Fragment Shader
    GLint success = GL_FALSE;
    int logSize;

    glGetShaderiv(ret, GL_COMPILE_STATUS, &success);
    glGetShaderiv(ret, GL_INFO_LOG_LENGTH, &logSize);
    std::cout<<"success: "<<success<<std::endl;
    if (!success && logSize > 0 ) {
        std::stringstream ss(str);
        std::string line;

        if (str.size())
        {
            int i = 1;
            while(std::getline(ss, line, '\n')) {
                std::cout <<"line "<< i++ << "\t: " << line <<std::endl;
            }
        }

        std::string shaderErrorMsg(logSize+1, '\0');
        glGetShaderInfoLog(ret, logSize, nullptr, &shaderErrorMsg[0]);
        printf("type %d\t: %s\n", type, &shaderErrorMsg[0]);
    }
    return ret;
}


