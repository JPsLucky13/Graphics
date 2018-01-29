#include <GL/glew.h>
#include <GL/freeglut.h>
#include "cyTriMesh.h"
#include "cyGL.h"
#include "cyMatrix.h"

//Screen values
const int screenWidth = 1024;
const int screenHeight = 512;

//Separate color values
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

//Matrices
cy::Matrix4<float> cameraPositionMatrix = cy::Matrix4<float>::MatrixTrans(cy::Point3f(0.0f,0.0f,-50.0f));
cy::Matrix4<float> cameraRotationMatrix = cy::Matrix4<float>::MatrixRotationY(0.0f) * cy::Matrix4<float>::MatrixRotationX(-1.5708f);
cy::Matrix4<float> viewMatrix;
cy::Matrix4<float> projectionMatrix = cy::Matrix4<float>::MatrixPerspective(0.785398f, screenWidth / screenHeight, 0.1f, 100);
cy::Matrix4<float> mvp;

//Camera
float cameraDistance = 0.0f;
float oldCameraXRotation = 0.0f;
float currentCameraXRotation = 0.0f;
float oldCameraYRotation = 0.0f;
float currentCameraYRotation = 0.0f;
float smoothFactor = 0.01f;
float rotationX = -1.5708f;
float rotationY = 0.0f;

//Toggle Projection
bool toggleProjection = false;

void CreateMesh(char * fileName)
{	
	bool result = mesh.LoadFromFileObj(fileName);
	mesh.ComputeBoundingBox();
}

void CreateShaders()
{
	//The Shader Program
	cy::GLSLShader vertexShader;
	vertexShader.CompileFile("Shaders/Vertex/mesh.cgfx", GL_VERTEX_SHADER);
	cy::GLSLShader fragmentShader;
	fragmentShader.CompileFile("Shaders/Fragment/mesh.cgfx", GL_FRAGMENT_SHADER);
	shaderProgram.Build(&vertexShader, &fragmentShader);

}

void Display()
{
	//glClearColor(GLclampf(red), GLclampf(green), GLclampf(blue), GLclampf(alpha));
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	cy::Matrix4<float> modelMatrix = cy::Matrix4<float>::MatrixTrans(-(mesh.GetBoundMin() + mesh.GetBoundMax())* 0.5f);
	viewMatrix = cameraPositionMatrix * cameraRotationMatrix;
	mvp = projectionMatrix * viewMatrix * modelMatrix;
	cameraDistance = cameraPositionMatrix.GetTrans().Length();
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
		//The P key is pressed to switch to perspective and vice versa
		case 112:
			if (!toggleProjection)
			{
				projectionMatrix.SetScale(cy::Point3f(1.0f/cameraDistance, 1.0f/cameraDistance, 1.0f / cameraDistance));
				toggleProjection = !toggleProjection;
			}
			else
			{
				projectionMatrix = cy::Matrix4<float>::MatrixPerspective(0.785398f, screenWidth / screenHeight, 0.1f, 100);
				toggleProjection = !toggleProjection;
			}

		
	}
}

void SpecialKeyboard(int key, int x, int y)
{
	if(key == GLUT_KEY_F6)
		CreateShaders();
}


void Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		oldCameraXRotation = static_cast<float>(y);
		oldCameraYRotation = static_cast<float>(x);
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
		currentCameraXRotation = static_cast<float>(y);
		currentCameraYRotation = static_cast<float>(x);

		float offsetX = currentCameraXRotation - oldCameraXRotation;
		float offsetY = currentCameraYRotation - oldCameraYRotation;

		oldCameraXRotation = static_cast<float>(y);
		oldCameraYRotation = static_cast<float>(x);

		rotationX += offsetX * 0.0174533f;
		rotationY += offsetY * 0.0174533f;

		cy::Matrix4f xRotationMatrix = cy::Matrix4f::MatrixRotationX(rotationX);
		cy::Matrix4f yRotationMatrix = cy::Matrix4f::MatrixRotationY(rotationY);

		cameraRotationMatrix = yRotationMatrix * xRotationMatrix;

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



int main(int argc, char* argv [])
{
	//Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(screenWidth, screenHeight);
	glutInitWindowPosition(0,0);
	glutCreateWindow("Hello Teapot!");
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(MouseMovement);
	glutDisplayFunc(Display);
	glutIdleFunc(Idle);

	//Initialize GLEW
	glewInit();
	CreateMesh(argv[1]);
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