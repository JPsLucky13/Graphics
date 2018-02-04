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

//Vertex Buffer
GLuint VBO;

//Normal Buffer
GLuint NBO;

GLuint VAO;
cy::TriMesh mesh;
cy::GLSLProgram shaderProgram;

//Keyboard states
bool leftCtrl = false;

//Mouse states
bool leftMouseDown = false;
bool rightMouseDown = false;

//Mouse Positions
cy::Point3f currentMousePosition;
cy::Point3f lastMousePosition;
cy::Point3f mouseDirection;

//Matrices
cy::Matrix4<float> modelMatrix;
cy::Matrix4<float> cameraPositionMatrix = cy::Matrix4<float>::MatrixTrans(cy::Point3f(0.0f,0.0f,-50.0f));
cy::Matrix4<float> cameraRotationMatrix = cy::Matrix4<float>::MatrixRotationY(0.0f) * cy::Matrix4<float>::MatrixRotationX(-1.5708f);
cy::Matrix3<float> inverseTransposeOfView;
cy::Matrix4<float> viewMatrix;
cy::Matrix4<float> projectionMatrix = cy::Matrix4<float>::MatrixPerspective(0.785398f, screenWidth / screenHeight, 0.1f, 100);
cy::Matrix4<float> mvp;

//Light
cy::Matrix4<float> lightMatrix;
cy::Matrix4<float> lightPositionMatrix = cy::Matrix4<float>::MatrixTrans(cy::Point3f(0.0f, 0.0f, 100.0f));
cy::Matrix4<float> lightRotationMatrix = cy::Matrix4<float>::MatrixRotationY(0.0f) * cy::Matrix4<float>::MatrixRotationX(0.0f);
float currentLightXRotation = 0.0f;
float currentLightYRotation = 0.0f;
float oldLightXRotation = 0.0f;
float oldLightYRotation = 0.0f;
float rotationLightX = 0.0f;
float rotationLightY = 0.0f;

//Viewer
cy::Point3f viewerPosition;

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
	modelMatrix = cy::Matrix4<float>::MatrixTrans(-(mesh.GetBoundMin() + mesh.GetBoundMax())* 0.5f);
	viewMatrix = cameraPositionMatrix * cameraRotationMatrix;
	lightMatrix = lightPositionMatrix * lightRotationMatrix;
	viewerPosition = cameraPositionMatrix.GetTrans();

	mvp = projectionMatrix * viewMatrix * modelMatrix;
	inverseTransposeOfView = (viewMatrix * modelMatrix).GetSubMatrix3();
	inverseTransposeOfView.Invert();
	inverseTransposeOfView.Transpose();
	cameraDistance = cameraPositionMatrix.GetTrans().Length();

	shaderProgram.Bind();
	shaderProgram.SetUniformMatrix4("mvp",mvp.data);
	shaderProgram.SetUniformMatrix3("inverseCM", inverseTransposeOfView.data);
	shaderProgram.SetUniformMatrix4("modelMatrix", modelMatrix.data);
	shaderProgram.SetUniform("lightPosition", lightMatrix.GetTrans());
	shaderProgram.SetUniform("viewerPosition", viewerPosition);
	glDrawArrays(GL_TRIANGLES, 0, mesh.NF() * 3);
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
		oldLightXRotation = static_cast<float>(y);
		oldLightYRotation = static_cast<float>(x);
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
	int mod = glutGetModifiers();
	if (mod == GLUT_ACTIVE_CTRL)
	{
		leftCtrl = true;
	}
	else
	{
		leftCtrl = false;
	}

	//Light Rotation
	if (leftCtrl && leftMouseDown)
	{
		currentLightXRotation = static_cast<float>(y);
		currentLightYRotation = static_cast<float>(x);

		float offsetX = currentLightXRotation - oldLightXRotation;
		float offsetY = currentLightYRotation - oldLightYRotation;

		oldLightXRotation = static_cast<float>(y);
		oldLightYRotation = static_cast<float>(x);

		rotationLightX += offsetX * 0.0174533f * .0035f;
		rotationLightY += offsetY * 0.0174533f;

		cy::Matrix4f xRotationMatrix = cy::Matrix4f::MatrixRotationX(rotationLightX);
		cy::Matrix4f yRotationMatrix = cy::Matrix4f::MatrixRotationY(rotationLightY);

		lightRotationMatrix =  xRotationMatrix;
		cy::Point3f lightPositionInModelSpace = modelMatrix.GetSubMatrix3() * lightPositionMatrix.GetTrans();
		cy::Point3f rotatedLightPositionInModelSpace = lightRotationMatrix.GetSubMatrix3() * lightPositionInModelSpace;
		cy::Point3f rotatedLightPositionInWorldSpace = modelMatrix.GetSubMatrix3().GetInverse() * rotatedLightPositionInModelSpace;
		lightPositionMatrix.SetTrans(rotatedLightPositionInWorldSpace);
	}

	//Update the mouse

	//Camera Rotation
	else if (leftMouseDown)
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
	else if (rightMouseDown)
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
	glEnable(GL_DEPTH_TEST);
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
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Create the array of vertices from the triangle vertices
	cy::Point3f * triangleVertices = new cy::Point3f[mesh.NF() * 3];

	unsigned int vertexIndex = 0;

	for (size_t i = 0; i < mesh.NF(); i++)
	{
		triangleVertices[vertexIndex] = mesh.V(mesh.F(i).v[0]);
		triangleVertices[vertexIndex + 1] = mesh.V(mesh.F(i).v[1]);
		triangleVertices[vertexIndex + 2] = mesh.V(mesh.F(i).v[2]);
		vertexIndex += 3;
	}

	

	glBufferData(GL_ARRAY_BUFFER, mesh.NF() * 3 * sizeof(cy::Point3f),triangleVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &NBO);
	glBindBuffer(GL_ARRAY_BUFFER, NBO);


	if (!mesh.HasNormals())
	{
		mesh.ComputeNormals();
	}

	//Create the array of vertex normals from the triangle vertex normals
	cy::Point3f * triangleVertexNormals = new cy::Point3f[mesh.NF() * 3];

	unsigned int vertexNormalIndex = 0;

	for (size_t i = 0; i < mesh.NF(); i++)
	{
		triangleVertexNormals[vertexNormalIndex] = mesh.VN(mesh.FN(i).v[0]);
		triangleVertexNormals[vertexNormalIndex + 1] = mesh.VN(mesh.FN(i).v[1]);
		triangleVertexNormals[vertexNormalIndex + 2] = mesh.VN(mesh.FN(i).v[2]);
		vertexNormalIndex += 3;
	}

	glBufferData(GL_ARRAY_BUFFER, mesh.NF() * 3 * sizeof(cy::Point3f), triangleVertexNormals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	CreateShaders();

	glutMainLoop();

	return 0;
}