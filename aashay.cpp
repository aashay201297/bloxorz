#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;
int initLevel();
struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;
int x = 0;
int do_rot, floor_rel;;
GLuint programID;
int last_update_time, current_time;
glm::vec3 rect_pos, floor_pos;
float rectangle_rotation = 0;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

    // Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

    // Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

    // Compile Vertex Shader
    //    printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

    // Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    //    printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

    // Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    //    fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

    // Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	if(x == 1)
	{
		printf("CONGRATULATIONS! YOU HAVE COMPLETED THE GAME\n");
	}
	else
		printf("\n GAME OVER! BETTER LUCK NEXT TIME :p\n");
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

void initGLEW(void){
	glewExperimental = GL_TRUE;
	if(glewInit()!=GLEW_OK){
		fprintf(stderr,"Glew failed to initialize : %s\n", glewGetErrorString(glewInit()));
	}
	if(!GLEW_VERSION_3_3)
		fprintf(stderr, "3.3 version not available\n");
}



/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
    // Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
    // Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float camera_angle = 0;
float xpos = 0, ypos = 0, blockz =  0;
int orientation = 0, falling = 0, switchOn = 1;
int viewMode = 0;
bool updir = false, downdir = false, leftdir = false, rightdir = false;
bool changeView = false, mouseLeft = false;
double mouse_x, mouse_y, pressx, pressy;
int level =1;
int lastkey = 1;
int moves = 0,win=0;
struct baseStruct
{
	int type;
} base[20][20];

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_UP:
			updir = false;
			break;
			case GLFW_KEY_DOWN:
			downdir = false;
			break;
			case GLFW_KEY_LEFT:
			leftdir = false;
			break;
			case GLFW_KEY_RIGHT:
			rightdir = false;
			break;
			default:
			break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
			quit(window);
			break;
			case GLFW_KEY_UP:
			updir = true;
			lastkey = 1;
			moves++;
			break;
			case GLFW_KEY_DOWN:
			downdir = true;
			lastkey = 2; 
			moves++;
			break;
			case GLFW_KEY_LEFT:
			leftdir = true;
			lastkey = 3;
			moves++;

			break;
			case GLFW_KEY_RIGHT:
			rightdir = true;

			moves++;
			lastkey = 4;
			break;
			case GLFW_KEY_SPACE:
			changeView = true;
			break;
			default:
			break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
		quit(window);
		break;
		default:
		break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
		if (action == GLFW_RELEASE)
		{
			mouseLeft=false;
		}
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &pressx, &pressy);
			mouseLeft=true;
		}
		break;
		default:
		break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = M_PI/2;

    // sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
	Matrices.projection = glm::perspective(fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    //Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *block[3], *cam, *floor_vao, *solidBase, *fragileBase, *switchBase[2], *bridgeBase, *goal;

void createFragileBase()
{
	static const GLfloat vertex_buffer_data [] = {
		0, 0, 0,
		0, 0.95, 0,
		0.95, 0, 0,

		0, 0.95, 0,
		0.95, 0.95, 0,
		0.95, 0, 0,

		0, 0, -0.4,
		0, 0.95, -0.4,
		0.95, 0, -0.4,

		0, 0.95, -0.4,
		0.95, 0.95, -0.4,
		0.95, 0, -0.4,

		0, 0, 0,
		0, 0, -0.4,
		0.95, 0, 0,

		0, 0, -0.4,
		0.95, 0, 0,
		0.95, 0, -0.4,

		0.95, 0, 0,
		0.95, 0, -0.4,
		0.95, 0.95, 0,

		0.95, 0.95, 0,
		0.95, 0.95, -0.4,
		0.95, 0, -0.4,

		0, 0, 0,
		0, 0, -0.4,
		0, 0.95, 0,

		0, 0.95, 0,
		0, 0.95, -0.4,
		0, 0, -0.4,

		0, 0.95, 0,
		0, 0.95, -0.4,
		0.95, 0.95, 0,

		0, 0.95, -0.4,
		0.95, 0.95, 0,
		0.95, 0.95, -0.4
	};

	static const GLfloat color_buffer_data [] = {
		(float)255/255, (float)94/255, (float)100/255,
		(float)255/255, (float)94/255, (float)100/255,
		(float)255/255, (float)94/255, (float)100/255,

		(float)255/255, (float)94/255, (float)100/255,
		(float)255/255, (float)94/255, (float)100/255,
		(float)255/255, (float)94/255, (float)100/255,

		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,

		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,

		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,

		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,

		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,

		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,

		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,

		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,

		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,

		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255,
		(float)158/255, (float)11/255, (float)62/255
	};

	fragileBase = create3DObject(GL_TRIANGLES, 12*3, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createGoal()
{
	static const GLfloat vertex_buffer_data [] = {
		0, 0, 0,
		0, 0.95, 0,
		0.95, 0, 0,

		0, 0.95, 0,
		0.95, 0.95, 0,
		0.95, 0, 0,

		0, 0, -0.4,
		0, 0.95, -0.4,
		0.95, 0, -0.4,

		0, 0.95, -0.4,
		0.95, 0.95, -0.4,
		0.95, 0, -0.4,

		0, 0, 0,
		0, 0, -0.4,
		0.95, 0, 0,

		0, 0, -0.4,
		0.95, 0, 0,
		0.95, 0, -0.4,

		0.95, 0, 0,
		0.95, 0, -0.4,
		0.95, 0.95, 0,

		0.95, 0.95, 0,
		0.95, 0.95, -0.4,
		0.95, 0, -0.4,

		0, 0, 0,
		0, 0, -0.4,
		0, 0.95, 0,

		0, 0.95, 0,
		0, 0.95, -0.4,
		0, 0, -0.4,

		0, 0.95, 0,
		0, 0.95, -0.4,
		0.95, 0.95, 0,

		0, 0.95, -0.4,
		0.95, 0.95, 0,
		0.95, 0.95, -0.4
	};

	static const GLfloat color_buffer_data [] = {
		(float)101/255, (float)94/255, (float)100/255,
		(float)101/255, (float)94/255, (float)100/255,
		(float)101/255, (float)94/255, (float)100/255,

		(float)101/255, (float)94/255, (float)100/255,
		(float)101/255, (float)94/255, (float)100/255,
		(float)101/255, (float)94/255, (float)100/255,

		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,

		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,

		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,

		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,

		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,

		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,

		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,

		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,

		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,

		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255,
		(float)15/255, (float)111/255, (float)62/255
	};

	goal = create3DObject(GL_TRIANGLES, 12*3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createSolidBase()
{
	static const GLfloat vertex_buffer_data [] = {
		0, 0, 0,
		0, 0.95, 0,
		0.95, 0, 0,

		0, 0.95, 0,
		0.95, 0.95, 0,
		0.95, 0, 0,

		0, 0, -0.4,
		0, 0.95, -0.4,
		0.95, 0, -0.4,

		0, 0.95, -0.4,
		0.95, 0.95, -0.4,
		0.95, 0, -0.4,

		0, 0, 0,
		0, 0, -0.4,
		0.95, 0, 0,

		0, 0, -0.4,
		0.95, 0, 0,
		0.95, 0, -0.4,

		0.95, 0, 0,
		0.95, 0, -0.4,
		0.95, 0.95, 0,

		0.95, 0.95, 0,
		0.95, 0.95, -0.4,
		0.95, 0, -0.4,

		0, 0, 0,
		0, 0, -0.4,
		0, 0.95, 0,

		0, 0.95, 0,
		0, 0.95, -0.4,
		0, 0, -0.4,

		0, 0.95, 0,
		0, 0.95, -0.4,
		0.95, 0.95, 0,

		0, 0.95, -0.4,
		0.95, 0.95, 0,
		0.95, 0.95, -0.4
	};

	static const GLfloat color_buffer_data [] = {
		(float)255/255, (float)109/255, (float)109/255,
		(float)255/255, (float)109/255, (float)109/255,
		(float)255/255, (float)109/255, (float)109/255,

		(float)255/255, (float)109/255, (float)109/255,
		(float)255/255, (float)109/255, (float)109/255,
		(float)255/255, (float)109/255, (float)109/255,

		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,

		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,

		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,

		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,

		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,

		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,

		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,

		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,

		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,

		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255,
		(float)163/255, (float)143/255, (float)64/255
	};

	solidBase = create3DObject(GL_TRIANGLES, 12*3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createSwitchBase()
{
	static const GLfloat vertex_buffer_data [] = {
		0, 0, 0,
		0, 0.95, 0,
		0.95, 0, 0,

		0, 0.95, 0,
		0.95, 0.95, 0,
		0.95, 0, 0,

		0, 0, -0.4,
		0, 0.95, -0.4,
		0.95, 0, -0.4,

		0, 0.95, -0.4,
		0.95, 0.95, -0.4,
		0.95, 0, -0.4,

		0, 0, 0,
		0, 0, -0.4,
		0.95, 0, 0,

		0, 0, -0.4,
		0.95, 0, 0,
		0.95, 0, -0.4,

		0.95, 0, 0,
		0.95, 0, -0.4,
		0.95, 0.95, 0,

		0.95, 0.95, 0,
		0.95, 0.95, -0.4,
		0.95, 0, -0.4,

		0, 0, 0,
		0, 0, -0.4,
		0, 0.95, 0,

		0, 0.95, 0,
		0, 0.95, -0.4,
		0, 0, -0.4,

		0, 0.95, 0,
		0, 0.95, -0.4,
		0.95, 0.95, 0,

		0, 0.95, -0.4,
		0.95, 0.95, 0,
		0.95, 0.95, -0.4
	};

	static const GLfloat color_buffer_data [] = {
		(float)83/255, (float)14/255, (float)83/255,
		(float)83/255, (float)14/255, (float)83/255,
		(float)83/255, (float)14/255, (float)83/255,

		(float)83/255, (float)14/255, (float)83/255,
		(float)83/255, (float)14/255, (float)83/255,
		(float)83/255, (float)14/255, (float)83/255,

		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,

		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,

		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,

		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,

		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,

		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,

		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,

		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,

		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,

		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255,
		(float)106/255, (float)18/255, (float)61/255 
	};

	switchBase[0] = create3DObject(GL_TRIANGLES, 12*3, vertex_buffer_data, color_buffer_data, GL_FILL);

	static const GLfloat color_buffer_data2 [] = {
		(float)86/255, (float)210/255, (float)67/255,
		(float)86/255, (float)210/255, (float)67/255,
		(float)86/255, (float)210/255, (float)67/255,

		(float)86/255, (float)210/255, (float)67/255,
		(float)86/255, (float)210/255, (float)67/255,
		(float)86/255, (float)210/255, (float)67/255,

		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,

		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,

		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,

		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,

		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,

		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,

		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,

		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,

		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,

		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255,
		(float)39/255, (float)84/255, (float)32/255  
	};

	switchBase[1] = create3DObject(GL_TRIANGLES, 12*3, vertex_buffer_data, color_buffer_data2, GL_FILL);
}

void createBridgeBase()
{
	static const GLfloat vertex_buffer_data [] = {
		0, 0, 0,
		0, 0.95, 0,
		0.95, 0, 0,

		0, 0.95, 0,
		0.95, 0.95, 0,
		0.95, 0, 0,

		0, 0, -0.4,
		0, 0.95, -0.4,
		0.95, 0, -0.4,

		0, 0.95, -0.4,
		0.95, 0.95, -0.4,
		0.95, 0, -0.4,

		0, 0, 0,
		0, 0, -0.4,
		0.95, 0, 0,

		0, 0, -0.4,
		0.95, 0, 0,
		0.95, 0, -0.4,

		0.95, 0, 0,
		0.95, 0, -0.4,
		0.95, 0.95, 0,

		0.95, 0.95, 0,
		0.95, 0.95, -0.4,
		0.95, 0, -0.4,

		0, 0, 0,
		0, 0, -0.4,
		0, 0.95, 0,

		0, 0.95, 0,
		0, 0.95, -0.4,
		0, 0, -0.4,

		0, 0.95, 0,
		0, 0.95, -0.4,
		0.95, 0.95, 0,

		0, 0.95, -0.4,
		0.95, 0.95, 0,
		0.95, 0.95, -0.4
	};

	static const GLfloat color_buffer_data [] = {
		(float)12/255, (float)46/255, (float)146/255,
		(float)12/255, (float)46/255, (float)146/255,
		(float)12/255, (float)46/255, (float)146/255,

		(float)12/255, (float)46/255, (float)146/255,
		(float)12/255, (float)46/255, (float)146/255,
		(float)12/255, (float)46/255, (float)146/255,

		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,

		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,

		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,

		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,

		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,

		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,

		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,

		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,

		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,

		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255,
		(float)251/255, (float)101/255, (float)66/255
	};

	bridgeBase = create3DObject(GL_TRIANGLES, 12*3, vertex_buffer_data, color_buffer_data, GL_FILL);

}

void createXBlock()
{
	static const GLfloat vertex_buffer_data [] = {
		0, 0, 0,
		0, 1, 0,
		2, 0, 0,

		2, 0, 0,
		0, 1, 0,
		2, 1, 0,

		0, 0, 1,
		0, 1, 1,
		2, 0, 1,

		2, 0, 1,
		0, 1, 1,
		2, 1, 1,

		0, 0, 0,
		0, 1, 0,
		0, 1, 1,

		0, 0, 1,
		0, 1, 1,
		0, 0, 0,

		0, 0, 0,
		0, 0, 1,
		2, 0, 1,

		0, 0, 0,
		2, 0, 1,
		2, 0, 0,

		0, 1, 0,
		0, 1, 1,
		2, 1, 1,

		2, 1, 0,
		2, 1, 1,
		0, 1, 0,

		2, 0, 0,
		2, 0, 1,
		2, 1, 0,

		2, 1, 0,
		2, 1, 1,
		2, 0, 1
	};

	static const GLfloat color_buffer_data [] = {
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,

		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,

		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255
	};

	block[1] = create3DObject(GL_TRIANGLES, 12*3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createYBlock()
{
	static const GLfloat vertex_buffer_data [] = {
		0, 0, 0,
		1, 0, 0,
		0, 2, 0,

		0, 2, 0,
		1, 0, 0,
		1, 2, 0,

		0, 0, 1,
		1, 0, 1,
		0, 2, 1,

		0, 2, 1,
		1, 0, 1,
		1, 2, 1,

		0, 0, 0,
		1, 0, 0,
		1, 0, 1,

		0, 0, 1,
		1, 0, 1,
		0, 0, 0,

		0, 0, 0,
		0, 0, 1,
		0, 2, 1,

		0, 0, 0,
		0, 2, 1,
		0, 2, 0,

		1, 0, 0,
		1, 0, 1,
		1, 2, 1,

		1, 2, 0,
		1, 2, 1,
		1, 0, 0,

		0, 2, 0,
		0, 2, 1,
		1, 2, 0,

		1, 2, 0,
		1, 2, 1,
		0, 2, 1
	};

	static const GLfloat color_buffer_data [] = {
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,

		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,

		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255
	};

	block[2] = create3DObject(GL_TRIANGLES, 12*3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createVerBlock()
{
	static const GLfloat vertex_buffer_data [] = {
		0, 0, 0,
		1, 0, 0,
		0, 1, 0,

		1, 0, 0,
		1, 1, 0,
		0, 1, 0,

		0, 0, 2,
		1, 0, 2,
		0, 1, 2,

		1, 0, 2,
		1, 1, 2,
		0, 1, 2,

		0, 0, 0,
		0, 0, 2,
		0, 1, 0,

		0, 0, 2,
		0, 1, 0,
		0, 1, 2,

		0, 1, 0,
		0, 1, 2,
		1, 1, 0,

		0, 1, 2,
		1, 1, 2,
		1, 1, 0,

		1, 0, 0,
		1, 1, 0,
		1, 1, 2,

		1, 1, 2,
		1, 0, 2,
		1, 0, 0,

		0, 0, 0,
		1, 0, 0,
		0, 0, 2,

		0, 0, 2,
		1, 0, 2,
		1, 0, 0
	};

	static const GLfloat color_buffer_data [] = {
		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,

		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,

		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,

		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,
		(float)242/255, (float)255/255, (float)41/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,

		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255,
		(float)55/255, (float)94/255, (float)151/255
	};

	block[0] = create3DObject(GL_TRIANGLES, 12*3, vertex_buffer_data, color_buffer_data, GL_FILL);
}
int checkBase(int x, int y)
{
	if ( x > 4.0 || y > 4.0)
	{
		falling = 1;

		return 0;
	}
	if (base[(int)(4-y)][(int)(4-x)].type == 0)
	{
		falling = 1;

		return 0;
	}
	if (base[(int)(4-y)][(int)(4-x)].type == 4 && switchOn == 0)
	{
		falling = 1;

		return 0;
	}
	if (base[(int)(4-y)][(int)(4-x)].type == 3)
		return 1;
	return 0;
}
void checkBlock()
{
	if (falling)
		return;
	int switchCheck = 0;
	if (orientation == 0)
	{
		if (base[(int)(4-ypos)][(int)(4-xpos)].type == 2)
		{
			falling = 1;

			return;
		}
		switchCheck = checkBase(xpos, ypos);
	}
	if(orientation ==0 && base[(int)(4-ypos)][(int)(4-xpos)].type == 5)
	{
		falling = 1;
		win = 1;
	}

	else if (orientation == 1)
	{
		switchCheck += checkBase(xpos, ypos);
		switchCheck += checkBase(xpos+1, ypos);
	}
	else if (orientation == 2)
	{
		switchCheck += checkBase(xpos, ypos);
		switchCheck += checkBase(xpos, ypos+1);
	}

	if (switchCheck > 0)
	{
		if(switchOn)
			switchOn=0;
		else
			switchOn=1;
	}
}
int initLevel()
{
	moves=0;
	win=0;
	falling = 0;
	FILE * file;
	char c;
	switch (level) {
		case 1:
		file = fopen("level01.txt", "r");
		break;
		case 2:
		file = fopen("level04.txt", "r");
		break;
		case 3:
		file = fopen("level10.txt", "r");
		break;
		case 4:
		file = fopen("level03.txt", "r");
		break;
		default:
		file = fopen("level09.txt", "r");
		break;
	}
	int flag=0;
	for(int i=0;i<10;i++)
	{
		for(int j=0;j<20;j++)
		{
			c = getc(file);
			if(c=='-')
				base[i][j].type = 0;
			else if(c=='o')
				base[i][j].type = 1;
			else if(c=='S')
			{
				base[i][j].type = 1;
				ypos=4-i;
				xpos=4-j;
				checkBlock();
			}
			else if(c=='T')
				base[i][j].type = 5;
			else if(c=='.')
				base[i][j].type = 2;
			else if(c=='h' || c=='s')
			{
				base[i][j].type = 3;
			}
			else if(c=='H' || c=='B')
				base[i][j].type = 4;
			else if(c == '\n' || c == EOF)
			{
				if(c==EOF) flag=1; break;
			}
		}
		if(flag) break;
	}
}




void moveBlock()
{
	if (falling)
	{
		blockz -= 0.023;
		if(blockz < -2)
		{
			level++;
			if(level>5)
			{
				x=1;
			}
			blockz=0;
			orientation=0;
			if(win)
				initLevel();
			else
			{
				cout << endl<<"-------------------------------------"<<endl;
				if(x == 1)
				{
					printf("CONGRATULATIONS! YOU HAVE COMPLETED THE GAME\n");
				}
				else
					printf("\n GAME OVER! BETTER LUCK NEXT TIME :p\n");
				exit(1);
			}
		}
		return;
	}
	int moved = 0;
	if(orientation==0)
	{
		if(updir)
		{
			ypos += 1;
			orientation = 2;
			moved=1;
			updir = false;
		}
		else if(downdir)
		{
			ypos -= 2;
			orientation = 2;
			moved=1;
			downdir = false;
		}
		else if(leftdir)
		{
			xpos -= 2;
			orientation = 1;
			moved=1;
			leftdir = false;
		}
		else if(rightdir)
		{
			xpos += 1;
			orientation = 1;
			moved=1;
			rightdir = false;
		}
		if (moved)
			checkBlock();

	}
	else if(orientation==1)
	{
		if(updir)
		{
			ypos += 1;
			orientation = 1;
			moved=1;
			updir = false;
		}
		else if(downdir)
		{
			ypos -= 1;
			orientation = 1;
			moved=1;
			downdir = false;
		}
		else if(leftdir)
		{
			xpos -= 1;
			orientation = 0;
			moved=1;
			leftdir = false;
		}
		else if(rightdir)
		{
			xpos += 2;
			orientation = 0;
			moved=1;
			rightdir = false;
		}
		if (moved)
			checkBlock();

	}
	else if(orientation==2)
	{
		if(updir)
		{
			ypos += 2;
			orientation = 0;
			moved=1;
			updir = false;
		}
		else if(downdir)
		{
			ypos -= 1;
			orientation = 0;
			moved=1;
			downdir = false;
		}
		else if(leftdir)
		{
			xpos -= 1;
			orientation = 2;
			moved=1;
			leftdir = false;
		}
		else if(rightdir)
		{
			xpos += 1;
			orientation = 2;
			moved=1;
			rightdir = false;
		}
		if (moved)
			checkBlock();
	}
}

void defineBase()
{
	string str="11111111112222221111131111111111114444411111111111111111115555";
	int si=str.size(),i;

	for (i=0; i<si; i++)
		base[i/10][i%10].type = (int)(str[i]-'0');

	for(i; i<400; i++)
		base[i/10][1%10].type = 0;
}

void chooseView()
{
	if (changeView)
		viewMode = (viewMode + 1)%5;
	changeView = false;
}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window, float x, float y, float w, float h) //, int doM, int doV, int doP)
{
	int fbwidth, fbheight;
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);
	glViewport((int)(x*fbwidth), (int)(y*fbheight), (int)(w*fbwidth), (int)(h*fbheight));


    // use the loaded shader program
    // Don't change unless you know what you are doing
	glUseProgram(programID);

	glm::vec3 eye, target, up;
	chooseView();
	if(level > 5)
		quit(window);
	if (viewMode == 0)
	{
        //tower 
		eye = glm::vec3(0, -9*cos(60*M_PI/180.0f), 9*sin(60*M_PI/180.0f) );
		target = glm::vec3(0, 0, 0);
		up = glm::vec3(0, 0, 1);
	}
	else if (viewMode == 1)
	{
		if (mouseLeft)
		{
			glfwGetCursorPos(window, &mouse_x, &mouse_y);
			
			if(pressy > 300)
			{
				camera_angle -= (mouse_x - pressx)/8;
			}
			else 
			{
				camera_angle += (mouse_x - pressx)/8;			
			}
			pressx = mouse_x;
			pressy = mouse_y;
		}
		eye = glm::vec3(12*sin(camera_angle*M_PI/180.0f), -12*cos(camera_angle*M_PI/180.0f), 9);
		target = glm::vec3(0, 0, 0);
		up = glm::vec3(0, 0, 1);
	}
	else if (viewMode == 2)
	{
        //top
		eye = glm::vec3(0, 0, 14 );
		target = glm::vec3(0, 0, 0);
		up = glm::vec3(0, 1, 0);
	}
	else if (viewMode == 3)
	{
        //BLOCK VIEW
		if (orientation == 0)
		{
			if (lastkey == 1)
			{
				eye = glm::vec3(xpos + 0.5, ypos+1, 1);
				target = glm::vec3(0, 1000, -500);
				up = glm::vec3(0, 1, 1000);
			}
			else if (lastkey == 2)
			{
				eye = glm::vec3(xpos + 0.5, ypos, 1);
				target = glm::vec3(0, -1000, -500);
				up = glm::vec3(0, 1, 1000);
			}
			else if (lastkey == 3)
			{
				eye = glm::vec3(xpos, ypos+0.5, 1);
				target = glm::vec3(-1000, 0, -500);
				up = glm::vec3(1, 0, 1000);
			}
			else if (lastkey == 4)
			{
				eye = glm::vec3(xpos + 1, ypos + 0.5, 1);
				target = glm::vec3(1000, 0, -500);
				up = glm::vec3(1, 0, 1000);
			}
		}
		else if (orientation == 1)
		{
			if (lastkey == 1)
			{
				eye = glm::vec3(xpos + 1, ypos + 1, 3);
				target = glm::vec3(0, 1000, -500);
				up = glm::vec3(0, 1, 1000);
			}
			else if (lastkey == 2)
			{
				eye = glm::vec3(xpos + 1, ypos, 1);
				target = glm::vec3(0, -1000, -500);
				up = glm::vec3(0, 1, 1000);
			}
			else if (lastkey == 3)
			{
				eye = glm::vec3(xpos, ypos + 0.5, 1);
				target = glm::vec3(-1000, 0, -500);
				up = glm::vec3(1, 0, 1000);
			}
			else if (lastkey == 4)
			{
				eye = glm::vec3(xpos + 2, ypos + 0.5, 1);
				target = glm::vec3(1000, 0, -500);
				up = glm::vec3(1, 0, 1000);
			}
		}
		else if (orientation == 2)
		{
			if (lastkey == 1)
			{
				eye = glm::vec3(xpos + 0.5, ypos + 2, 3);
				target = glm::vec3(0, 1000, -500);
				up = glm::vec3(0, 1, 1000);
			}
			else if (lastkey == 2)
			{
				eye = glm::vec3(xpos + 0.5, ypos, 3);
				target = glm::vec3(0, -1000, -500);
				up = glm::vec3(0, 1, 1000);
			}
			else if (lastkey == 3)
			{
				eye = glm::vec3(xpos, ypos + 1, 3);
				target = glm::vec3(-1000, 0, -500);
				up = glm::vec3(1, 0, 1000);
			}
			else if (lastkey == 4)
			{
				eye = glm::vec3(xpos + 1, ypos + 1, 3);
				target = glm::vec3(1000, 0, -500);
				up = glm::vec3(1, 0, 1000);
			}
			up = glm:: vec3(0, 1, 100);
		}
	}
	else if (viewMode == 4)
	{
        //flow
		eye = glm::vec3(xpos+5, ypos, 2.5);
		target = glm::vec3(-1000, 0, 0);
		up = glm:: vec3(0, 1, 100);
	}
	Matrices.view = glm::lookAt(eye, target, up); // Fixed camera for 2D (ortho) in XY plane

	glm::mat4 VP;
	VP = Matrices.projection * Matrices.view;

    glm::mat4 MVP;	// MVP = Projection * View * Model

    moveBlock();
    Matrices.model = glm::mat4(1.0f);
    Matrices.model *= (glm::translate (glm::vec3(xpos, ypos, blockz)));
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(block[orientation]);

    for (int i=0; i<10; i++)
    {
    	for (int j=0; j<20; j++)
    	{
    		if (base[i][j].type > 0)
    		{
    			Matrices.model = glm::mat4(1.0f);
    			Matrices.model *= (glm::translate (glm::vec3(4-j,4-i,0)));
    			MVP = VP * Matrices.model;
    			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    		}
    		if (base[i][j].type == 1)
    			draw3DObject(solidBase);
    		else if (base[i][j].type == 2)
    			draw3DObject(fragileBase);
    		else if (base[i][j].type == 3)
    			draw3DObject(switchBase[switchOn]);
    		else if (base[i][j].type == 4 && switchOn)
    			draw3DObject(bridgeBase);
    		else if(base[i][j].type == 5 )
    		{
    			draw3DObject(goal);
    		}
    	}
    }
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height){
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
    	exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
    	exit(EXIT_FAILURE);
    	glfwTerminate();
    }

    glfwMakeContextCurrent(window);
    //    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);
    glfwSetWindowCloseCallback(window, quit);
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
    // Create the models
	createFragileBase();
	createSolidBase();
	// defineBase();
	initLevel();
	createVerBlock();
	createXBlock();
	createYBlock();
	createSwitchBase();
	createBridgeBase();
	createGoal();

    // Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
}


int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;
	rect_pos = glm::vec3(0, 0, 0);
	floor_pos = glm::vec3(0, 0, 0);
	do_rot = 0;
	floor_rel = 1;

	GLFWwindow* window = initGLFW(width, height);
	initGLEW();
	initGL (window, width, height);

	last_update_time = glfwGetTime();
    /* Draw in loop */
	cout << "_____________________________________"<<endl;
	while (!glfwWindowShouldClose(window)) {

	// clear the color and depth in the frame buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // OpenGL Draw commands
		current_time = glfwGetTime();
		if(camera_angle > 720)
			camera_angle -= 720;
		last_update_time = current_time;
		draw(window, 0, 0, 1, 1);
        // Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
		glfwPollEvents();
		std::cout << '\r'
		<< "||" << "TIME = "<< current_time  << "||" << "  "
		<<"||"<< "NUMBER OF MOVES = " << moves <<"||"<< std::flush;
	}

	glfwTerminate();
}
