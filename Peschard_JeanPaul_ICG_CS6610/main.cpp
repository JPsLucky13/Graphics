#include <GL/glew.h>
#include <GL/freeglut.h>
#include "cyTriMesh.h"
#include "cyGL.h"
#include "cyMatrix.h"
#include "lodepng.h"

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

//Vertex Buffer and UV buffer for the plane
GLuint VBPlane;
GLuint UVBPlane;
GLuint VAOPlane;

//Normal Buffer
GLuint NBO;

//UV Buffer
GLuint UVBO;

//Texture Objects
GLuint TO;
GLuint TO2;

//Texture Object for the plane
GLuint TOPlane;
cy::GLSLProgram planeShaderProgram;

//The Render Texture Object to draw the viewport on a texture
cy::GLRenderTexture2D renderTexture;

//Vertex Array Buffer Object
GLuint VAO;
cy::TriMesh mesh;
cy::GLSLProgram shaderProgram;

//The texture unit type
GLenum textureUnitType = GL_TEXTURE_2D;

//Keyboard states
bool leftCtrl = false;
bool alt = false;

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
cy::Matrix4<float> cameraRotationMatrix = cy::Matrix4<float>::MatrixRotationY(0.0f) * cy::Matrix4<float>::MatrixRotationX(0.0f);
cy::Matrix3<float> inverseTransposeOfView;
cy::Matrix4<float> viewMatrix;
cy::Matrix4<float> projectionMatrix = cy::Matrix4<float>::MatrixPerspective(0.785398f, screenWidth / screenHeight, 0.1f, 100);
cy::Matrix4<float> mvp;

//Light
cy::Matrix4<float> lightMatrix;
cy::Matrix4<float> lightPositionMatrix = cy::Matrix4<float>::MatrixTrans(cy::Point3f(0.0f, 0.0f, 200.0f));
cy::Matrix4<float> lightRotationMatrix = cy::Matrix4<float>::MatrixRotationY(0.0f) * cy::Matrix4<float>::MatrixRotationX(0.0f);
float currentLightXRotation = 0.0f;
float currentLightYRotation = 0.0f;
float currentLightZRotation = 0.0f;
float oldLightXRotation = 0.0f;
float oldLightYRotation = 0.0f;
float oldLightZRotation = 0.0f;
float rotationLightX = 0.0f;
float rotationLightY = 0.0f;
float rotationLightZ = 0.0f;

//Plane
cy::Matrix4<float> planePositionMatrix = cy::Matrix4<float>::MatrixTrans(cy::Point3f(0.0f, 0.0f, -50.0f));
cy::Matrix4<float> planeRotationMatrix = cy::Matrix4<float>::MatrixRotationY(0.0f) * cy::Matrix4<float>::MatrixRotationX(0.0f);
float currentPlaneXRotation = 0.0f;
float currentPlaneYRotation = 0.0f;
float oldPlaneXRotation = 0.0f;
float oldPlaneYRotation = 0.0f;
float rotationPlaneX = 0.0f;
float rotationPlaneY = 0.0f;

//Viewer
cy::Point4f viewerPosition;

//Camera
float cameraDistance = 0.0f;
float oldCameraXRotation = 0.0f;
float currentCameraXRotation = 0.0f;
float oldCameraYRotation = 0.0f;
float currentCameraYRotation = 0.0f;
float smoothFactor = 0.01f;
float rotationX = 0.0f;
float rotationY = 0.0f;

//Toggle Projection
bool toggleProjection = false;

std::string GetParentFolder(const char * fileName)
{
	char * newFilePath = const_cast<char *>(fileName);

	for (int i = strlen(newFilePath) - 1; i > 0; --i)
	{
		if (newFilePath[i] == '/')
		{
			newFilePath[i+1] = '\0';
			break;
		}
	}

	std::string result = newFilePath;

	return result;
}


void CreateMesh(const char * fileName)
{	
	bool result = mesh.LoadFromFileObj(fileName);
	mesh.ComputeBoundingBox();

}


void CreatePlaneMesh(float dimensions)
{
	//Generate and bind the plane array buffer 
	glGenVertexArrays(1, &VAOPlane);
	glBindVertexArray(VAOPlane);

	//Generate and bind the plane buffer 
	glGenBuffers(1, &VBPlane);
	glBindBuffer(GL_ARRAY_BUFFER, VBPlane);

	//Create the array of vertices from the triangle vertices
	cy::Point3f * planeVertices = new cy::Point3f[6];

	planeVertices[0] = cy::Point3f(-dimensions,-dimensions,0.0f);
	planeVertices[1] = cy::Point3f(dimensions, -dimensions, 0.0f);
	planeVertices[2] = cy::Point3f(-dimensions, dimensions, 0.0f);
	planeVertices[3] = cy::Point3f(dimensions, -dimensions, 0.0f);
	planeVertices[4] = cy::Point3f(dimensions,dimensions, 0.0f);
	planeVertices[5] = cy::Point3f(-dimensions, dimensions, 0.0f);

	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(cy::Point3f), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//Generate and bind the plane buffer 
	glGenBuffers(1, &UVBPlane);
	glBindBuffer(GL_ARRAY_BUFFER, UVBPlane);

	//Create the array of vertices from the triangle vertices
	cy::Point2f * planeUVVertices = new cy::Point2f[6];

	planeUVVertices[0] = cy::Point2f(0.0f,0.0f);
	planeUVVertices[1] = cy::Point2f(1.0f,0.0f);
	planeUVVertices[2] = cy::Point2f(0.0f,1.0f);
	planeUVVertices[3] = cy::Point2f(1.0f,0.0f);
	planeUVVertices[4] = cy::Point2f(1.0f,1.0f);
	planeUVVertices[5] = cy::Point2f(0.0f,1.0f);

	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(cy::Point2f), planeUVVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
}


void CreateTexture(std::string fileName)
{
	std::vector<unsigned char> image; //the raw pixels
	unsigned width, height;

	//decode
	unsigned error = lodepng::decode(image, width, height, fileName);

	//if there's an error, display it
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	glTexImage2D(textureUnitType, 0, GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,image.data());
	
	glTexParameterf(textureUnitType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(textureUnitType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(textureUnitType);
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

void CreatePlaneShaders()
{
	//The Shader Program
	cy::GLSLShader vertexShader;
	vertexShader.CompileFile("Shaders/Vertex/plane.cgfx", GL_VERTEX_SHADER);
	cy::GLSLShader fragmentShader;
	fragmentShader.CompileFile("Shaders/Fragment/plane.cgfx", GL_FRAGMENT_SHADER);
	planeShaderProgram.Build(&vertexShader, &fragmentShader);
}

void Display()
{
	//glClearColor(GLclampf(red), GLclampf(green), GLclampf(blue), GLclampf(alpha));
	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	renderTexture.Bind();
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	modelMatrix = cyMatrix4f::MatrixRotationX(-2 * 0.785398f) * cy::Matrix4<float>::MatrixTrans(-(mesh.GetBoundMin() + mesh.GetBoundMax())* 0.5f);
	viewMatrix = cameraPositionMatrix * cameraRotationMatrix;
	lightMatrix = viewMatrix * lightRotationMatrix * lightPositionMatrix;
	viewerPosition = viewMatrix * cameraPositionMatrix.GetTrans();

	mvp = projectionMatrix * viewMatrix * modelMatrix;
	inverseTransposeOfView = (viewMatrix * modelMatrix).GetSubMatrix3();
	inverseTransposeOfView.Invert();
	inverseTransposeOfView.Transpose();
	cameraDistance = cameraPositionMatrix.GetTrans().Length();

	shaderProgram.Bind();
	shaderProgram.SetUniformMatrix4("mvp",mvp.data);
	shaderProgram.SetUniformMatrix3("inverseCM", inverseTransposeOfView.data);
	shaderProgram.SetUniformMatrix4("modelMatrix", modelMatrix.data);
	shaderProgram.SetUniformMatrix4("viewMatrix", viewMatrix.data);
	shaderProgram.SetUniform("lightPosition", lightMatrix.GetTrans());
	shaderProgram.SetUniform("viewerPosition", viewerPosition);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(textureUnitType, TO);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(textureUnitType, TO2);

	//Set the parameters of the renderTexture object
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, mesh.NF() * 3);
	renderTexture.Unbind();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	modelMatrix = planePositionMatrix * planeRotationMatrix;
	mvp = projectionMatrix * modelMatrix;

	planeShaderProgram.Bind();
	planeShaderProgram.SetUniformMatrix4("mvp", mvp.data);

	glActiveTexture(GL_TEXTURE0);
	renderTexture.BindTexture();

	glBindVertexArray(VAOPlane);
	glDrawArrays(GL_TRIANGLES, 0, 6);
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
		oldPlaneXRotation = static_cast<float>(y);
		oldPlaneYRotation = static_cast<float>(x);
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
	
	if (mod == GLUT_ACTIVE_ALT)
	{
		alt = true;
	}
	else
	{
		alt = false;
	}

	//Update the mouse	
	if (leftMouseDown)
	{
		//Light rotation
		if (leftCtrl)
		{
			currentLightXRotation = static_cast<float>(y);
			currentLightYRotation = static_cast<float>(x);

			float offsetX = currentLightXRotation - oldLightXRotation;
			float offsetY = currentLightYRotation - oldLightYRotation;

			oldLightXRotation = static_cast<float>(y);
			oldLightYRotation = static_cast<float>(x);

			rotationLightX += offsetX * 0.0174533f;
			rotationLightY += offsetY * 0.0174533f;

			cy::Matrix4f xRotationMatrix = cy::Matrix4f::MatrixRotationX(rotationLightX);
			cy::Matrix4f yRotationMatrix = cy::Matrix4f::MatrixRotationY(rotationLightY);

			lightRotationMatrix = yRotationMatrix * xRotationMatrix;			
		}

		else if (alt)
		{
			currentPlaneXRotation = static_cast<float>(y);
			currentPlaneYRotation = static_cast<float>(x);

			float offsetX = currentPlaneXRotation - oldPlaneXRotation;
			float offsetY = currentPlaneYRotation - oldPlaneYRotation;

			oldPlaneXRotation = static_cast<float>(y);
			oldPlaneYRotation = static_cast<float>(x);

			rotationPlaneX += offsetX * 0.0174533f;
			rotationPlaneY += offsetY * 0.0174533f;

			cy::Matrix4f xRotationMatrix = cy::Matrix4f::MatrixRotationX(rotationPlaneX);
			cy::Matrix4f yRotationMatrix = cy::Matrix4f::MatrixRotationY(rotationPlaneY);

			planeRotationMatrix = yRotationMatrix * xRotationMatrix;

		}

		//Camera Rotation
		else if(!leftCtrl && !alt){
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
	}

	//Camera Translation
	else if (rightMouseDown)
	{
		currentMousePosition = cy::Point3f(0.0f, 0.0f, static_cast<float>(y));

		mouseDirection = currentMousePosition - lastMousePosition;

		lastMousePosition = cy::Point3f(0.0f, 0.0f, static_cast<float>(y));

		if (alt)
		{
			planePositionMatrix.AddTrans(mouseDirection);
		}

		else
		{
			cameraPositionMatrix.AddTrans(mouseDirection);
		}
		

		
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
	glutInitContextVersion(3, 3);
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
	std::string path = "";
	if (argc > 1) {
		path = argv[1];
	}
	else {
		path = "Models/teapot.obj";
	}

	CreateMesh(path.c_str());

	renderTexture.Initialize(true, 3, screenWidth, screenHeight);
	renderTexture.SetTextureFilteringMode(GL_LINEAR_MIPMAP_LINEAR, 0);
	renderTexture.SetTextureMaxAnisotropy();
	renderTexture.BuildTextureMipmaps();

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

	mesh.ComputeNormals();


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

	glGenBuffers(1, &UVBO);
	glBindBuffer(GL_ARRAY_BUFFER, UVBO);

	//Create the array of vertex uvs from the triangle vertex uvs
	cy::Point2f * triangleVertexTextureCoords = new cy::Point2f[mesh.NF() * 3];

	unsigned int vertexTextureIndex = 0;

	for (size_t i = 0; i < mesh.NF(); i++)
	{
		triangleVertexTextureCoords[vertexTextureIndex] = cy::Point2f(mesh.VT(mesh.FT(i).v[0]));
		triangleVertexTextureCoords[vertexTextureIndex + 1] = cy::Point2f(mesh.VT(mesh.FT(i).v[1]));
		triangleVertexTextureCoords[vertexTextureIndex + 2] = cy::Point2f(mesh.VT(mesh.FT(i).v[2]));
		vertexTextureIndex += 3;
	}
	glBufferData(GL_ARRAY_BUFFER, mesh.NF() * 3 * sizeof(cy::Point2f), triangleVertexTextureCoords, GL_STATIC_DRAW);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	glGenTextures(1, &TO);
	glBindTexture(textureUnitType, TO);

	//Create the diffuse texture
	CreateTexture(GetParentFolder(path.c_str()).append(mesh.M(0).map_Kd.data));

	glGenTextures(1, &TO2);
	glBindTexture(textureUnitType, TO2);

	//Create the specular texture
	CreateTexture(GetParentFolder(path.c_str()).append(mesh.M(0).map_Ks.data));
	CreateShaders();

	//Create the plane Mesh
	CreatePlaneMesh(10.0f);
	CreatePlaneShaders();

	glutMainLoop();
	return 0;
}