#include <GL/glew.h>
#include <GL/freeglut.h>
#include <../../Utilities/cyTriMesh.h>
#include <../../Utilities/cyGL.h>
#include <../../Utilities/cyMatrix.h>

//Clear color values
const int screenWidth = 512;
const int screenHeight = 512;

float red = 0.25f;
float green = 0.1f;
float blue = 0.2f;
float alpha = 0.0f;
float colorChangeSpeed = 0.005f;

GLuint VBO;
GLuint VAO;
cy::TriMesh mesh;
cy::GLSLProgram shaderProgram;

//Mouse states
bool leftMouseDown = false;
bool rightMouseDown = false;

//Mouse Positions
cy::Point3f currentMousePosition;
cy::Point3f lastMousePosition;
cy::Point3f mouseDirection;

//Mouse Rotations
cy::Point3f currentMouseYRotation;
cy::Point3f lastMouseYRotation;
cy::Point3f mouseYRotationDirection;

//Matrices
cy::Matrix4<float> cameraPositionMatrix = cy::Matrix4<float>::MatrixTrans(cy::Point3f(0.0f,0.0f,-50.0f));
cy::Matrix4<float> cameraRotationMatrix = cy::Matrix4<float>::MatrixRotationX(0.0f);
cy::Matrix4<float> viewMatrix;
cy::Matrix4<float> projectionMatrix = cy::Matrix4<float>::MatrixPerspective(0.785398f, screenWidth / screenHeight, 0.1f, 100);
cy::Matrix4<float> mvp;

void CreateMesh()
{
	bool result = mesh.LoadFromFileObj("Models/teapot.obj");
}

void Display()
{
	//glClearColor(GLclampf(red), GLclampf(green), GLclampf(blue), GLclampf(alpha));
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	viewMatrix = cameraPositionMatrix * cameraRotationMatrix;
	mvp = projectionMatrix * viewMatrix;
	shaderProgram.Bind();
	shaderProgram.SetUniformMatrix4("mvp",mvp.data);
	glDrawArrays(GL_POINTS, 0, mesh.NV());
	glutSwapBuffers();
}

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		//The escape key is pressed to close the window
		case 27:
			glutLeaveMainLoop();
	}
}

void Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		lastMouseYRotation = cy::Point3f(0.0f, static_cast<float>(x), 0.0f);
		leftMouseDown = true;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		leftMouseDown = false;
	}

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		lastMousePosition = cy::Point3f(0.0f, 0.0f, static_cast<float>(y));
		rightMouseDown = true;
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
	{
		rightMouseDown = false;
	}



}

void MouseMovement(int x, int y)
{
	//Update the mouse

	//Camera Rotation
	if (leftMouseDown)
	{
		currentMouseYRotation = cy::Point3f(0.0f, static_cast<float>(x), 0.0f);

		mouseYRotationDirection = currentMouseYRotation - lastMouseYRotation;

		lastMouseYRotation = cy::Point3f(0.0f, static_cast<float>(x), 0.0f);


		cy::Matrix4<float> oldMouseYRotation = cameraRotationMatrix;
		cameraRotationMatrix.SetRotationY(mouseYRotationDirection.y);

	}

	//Camera Translation
	if (rightMouseDown)
	{
		currentMousePosition = cy::Point3f(0.0f,0.0f, static_cast<float>(y));

		mouseDirection = currentMousePosition - lastMousePosition;
		
		lastMousePosition = cy::Point3f(0.0f, 0.0f, static_cast<float>(y));

		cameraPositionMatrix.AddTrans(mouseDirection);
	}
}


void Idle()
{
	red += colorChangeSpeed;
	green += colorChangeSpeed;
	blue += colorChangeSpeed;

	if (red >= 1.0f)
	{
		red = 0.0f;
	}

	if (green >= 1.0f)
	{
		green = 0.0f;
	}

	if (blue >= 1.0f)
	{
		blue = 0.0f;
	}
	glutPostRedisplay();
}

void CreateShaders()
{
	//The Shader Program
	cy::GLSLShader vertexShader;
	vertexShader.CompileFile("Shaders/Vertex/mesh.cgfx",GL_VERTEX_SHADER);
	cy::GLSLShader fragmentShader;
	fragmentShader.CompileFile("Shaders/Fragment/mesh.cgfx", GL_FRAGMENT_SHADER);
	shaderProgram.Build(&vertexShader,&fragmentShader);
	
}

int main(int argc, char* argv)
{
	//Initialize GLUT
	glutInit(&argc, &argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(screenWidth, screenHeight);
	glutInitWindowPosition(0,0);
	glutCreateWindow("Hello Teapot!");
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(MouseMovement);
	glutDisplayFunc(Display);
	glutIdleFunc(Idle);
	
	//Initialize GLEW
	glewInit();
	CreateMesh();
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, mesh.NV() * sizeof(cy::Point3f), &mesh.V(0), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	
	
	CreateShaders();

	glutMainLoop();

	return 0;
}