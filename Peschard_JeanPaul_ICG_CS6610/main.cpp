#include <GL/glew.h>
#include <GL/freeglut.h>
#include "cyTriMesh.h"
#include "cyGL.h"
#include "cyMatrix.h"
#include "lodepng.h"

//Screen values
const float screenWidth = 1024;
const float screenHeight = 512;

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
GLuint NBPlane;
GLuint UVBPlane;
GLuint VAOPlane;

//Vertex Buffer and UV buffer for the cube
GLuint VBCube;
GLuint VAOCube;

//Vertex buffers for the sphere
cy::TriMesh sphereMesh;
GLuint VBSphere;
GLuint NBSphere;
GLuint UVBSphere;
GLuint VAOSphere;

//Vertex buffers for the light
cy::TriMesh lightMesh;
GLuint VBLight;
GLuint NBLight;
GLuint VAOLight;

//Normal Buffer
GLuint NBO;

//UV Buffer
GLuint UVBO;

//Texture Objects
GLuint TO;
GLuint TO2;

//Cube Mesh
cy::TriMesh cubeMesh;

//Texture Object for the plane
GLuint TOPlane;
cy::GLSLProgram planeShaderProgram;

//Shader for the cube
cy::GLSLProgram cubeShaderProgram;

//Shader for the sphere
cy::GLSLProgram sphereShaderProgram;

//The Render Texture Object to draw the viewport on a texture
cy::GLRenderTexture2D renderTexture;

//Vertex Array Buffer Object
GLuint VAO;
cy::TriMesh mesh;
cy::GLSLProgram shaderProgram;

//Shadow shader
cy::GLSLProgram shadowMapShaderProgram;
cy::GLSLProgram shadowShaderProgram;

//Simple shader
cy::GLSLProgram simpleShaderProgram;

//The texture unit type
GLenum textureUnitType = GL_TEXTURE_2D;

//The texture unit for the cube map
GLenum textureUnitCubeMap = GL_TEXTURE_CUBE_MAP;

//The cube map texture
cy::GLTextureCubeMap cubeMapTexture;

//The depth map
cy::GLRenderDepth2D depthMap;

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
cy::Matrix4<float> cameraPositionMatrix = cy::Matrix4<float>::MatrixTrans(cy::Point3f(0.0f,8.0f,-70.0f));
cy::Matrix4<float> cameraRotationMatrix = cy::Matrix4<float>::MatrixRotationY(0.0f) * cy::Matrix4<float>::MatrixRotationX(0.0f);
cy::Matrix3<float> inverseTransposeOfView;
cy::Matrix4<float> viewMatrix;
cy::Matrix4<float> projectionMatrix = cy::Matrix4<float>::MatrixPerspective(0.785398f, screenWidth / screenHeight, 0.1f, 1000);
cy::Matrix4<float> mvp;
cy::Matrix4<float> mv;

//Light
cy::Matrix4<float> lightMatrix;
cy::Matrix4<float> lightCameraMatrix;
cy::Matrix4<float> lightPositionMatrix = cy::Matrix4<float>::MatrixTrans(cy::Point3f(0.0f, 0.0f, 100.0f));
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
cy::Matrix4<float> planePositionMatrix = cy::Matrix4<float>::MatrixTrans(cy::Point3f(0.0f, -8.0f, 0.0f));
cy::Matrix4<float> planeRotationMatrix = cy::Matrix4<float>::MatrixRotationY(0.0f) * cy::Matrix4<float>::MatrixRotationX(0.0f);
float currentPlaneXRotation = 0.0f;
float currentPlaneYRotation = 0.0f;
float oldPlaneXRotation = 0.0f;
float oldPlaneYRotation = 0.0f;
float rotationPlaneX = 0.0f;
float rotationPlaneY = 0.0f;

//Cube
cy::Matrix4<float> cubePositionMatrix;
cy::Matrix4<float> cubeRotationMatrix = cy::Matrix4<float>::MatrixRotationY(0.0f) * cy::Matrix4<float>::MatrixRotationX(0.0f);


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
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	bool result = mesh.LoadFromFileObj(fileName);
	mesh.ComputeBoundingBox();

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

	glBufferData(GL_ARRAY_BUFFER, mesh.NF() * 3 * sizeof(cy::Point3f), triangleVertices, GL_STATIC_DRAW);
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
}

void CreateCubeMesh()
{
	//Generate and bind the cube array buffer 
	glGenVertexArrays(1, &VAOCube);
	glBindVertexArray(VAOCube);

	//Generate and bind the cube buffer 
	glGenBuffers(1, &VBCube);
	glBindBuffer(GL_ARRAY_BUFFER, VBCube);

	bool result = cubeMesh.LoadFromFileObj("Models/cube.obj");
	cubeMesh.ComputeBoundingBox();

	//Create the array of vertices from the triangle vertices
	cy::Point3f * cubeVertices = new cy::Point3f[cubeMesh.NF() * 3];

	unsigned int vertexIndex = 0;

	for (size_t i = 0; i < cubeMesh.NF(); i++)
	{
		cubeVertices[vertexIndex] = cubeMesh.V(cubeMesh.F(i).v[0]);
		cubeVertices[vertexIndex + 1] = cubeMesh.V(cubeMesh.F(i).v[1]);
		cubeVertices[vertexIndex + 2] = cubeMesh.V(cubeMesh.F(i).v[2]);
		vertexIndex += 3;
	}

	glBufferData(GL_ARRAY_BUFFER, cubeMesh.NF() * 3 * sizeof(cy::Point3f), cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

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

	planeVertices[0] = cy::Point3f(-dimensions,0.0f, -dimensions);
	planeVertices[1] = cy::Point3f(dimensions,0.0f , -dimensions);
	planeVertices[2] = cy::Point3f(-dimensions, 0.0f, dimensions);

	planeVertices[3] = cy::Point3f(dimensions, 0.0f, -dimensions);
	planeVertices[4] = cy::Point3f(dimensions,0.0f, dimensions);
	planeVertices[5] = cy::Point3f(-dimensions, 0.0f, dimensions);

	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(cy::Point3f), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//Generate and bind the plane normal buffer 
	glGenBuffers(1, &NBPlane);
	glBindBuffer(GL_ARRAY_BUFFER, NBPlane);

	//Create the array of normals from the triangle vertices
	cy::Point3f * planeNormals = new cy::Point3f[6];

	planeNormals[0] = cy::Point3f(0.0f, 1.0f, 0.0f);
	planeNormals[1] = cy::Point3f(0.0f, 1.0f, 0.0f);
	planeNormals[2] = cy::Point3f(0.0f, 1.0f, 0.0f);

	planeNormals[3] = cy::Point3f(0.0f, 1.0f, 0.0f);
	planeNormals[4] = cy::Point3f(0.0f, 1.0f, 0.0f);
	planeNormals[5] = cy::Point3f(0.0f, 1.0f, 0.0f);

	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(cy::Point3f), planeNormals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void CreateSphereMesh()
{
	//Generate and bind the cube array buffer 
	glGenVertexArrays(1, &VAOSphere);
	glBindVertexArray(VAOSphere);

	//Generate and bind the cube buffer 
	glGenBuffers(1, &VBSphere);
	glBindBuffer(GL_ARRAY_BUFFER, VBSphere);

	bool result = sphereMesh.LoadFromFileObj("Models/sphere.obj");
	sphereMesh.ComputeBoundingBox();

	//Create the array of vertices from the triangle vertices
	cy::Point3f * sphereVertices = new cy::Point3f[sphereMesh.NF() * 3];

	unsigned int vertexIndex = 0;

	for (size_t i = 0; i < sphereMesh.NF(); i++)
	{
		sphereVertices[vertexIndex] = sphereMesh.V(sphereMesh.F(i).v[0]);
		sphereVertices[vertexIndex + 1] = sphereMesh.V(sphereMesh.F(i).v[1]);
		sphereVertices[vertexIndex + 2] = sphereMesh.V(sphereMesh.F(i).v[2]);
		vertexIndex += 3;
	}

	glBufferData(GL_ARRAY_BUFFER, sphereMesh.NF() * 3 * sizeof(cy::Point3f), sphereVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &NBSphere);
	glBindBuffer(GL_ARRAY_BUFFER, NBSphere);

	sphereMesh.ComputeNormals();


	//Create the array of vertex normals from the triangle vertex normals
	cy::Point3f * triangleVertexNormals = new cy::Point3f[sphereMesh.NF() * 3];

	unsigned int vertexNormalIndex = 0;

	for (size_t i = 0; i < sphereMesh.NF(); i++)
	{
		triangleVertexNormals[vertexNormalIndex] = sphereMesh.VN(sphereMesh.FN(i).v[0]);
		triangleVertexNormals[vertexNormalIndex + 1] = sphereMesh.VN(sphereMesh.FN(i).v[1]);
		triangleVertexNormals[vertexNormalIndex + 2] = sphereMesh.VN(sphereMesh.FN(i).v[2]);
		vertexNormalIndex += 3;
	}

	glBufferData(GL_ARRAY_BUFFER, sphereMesh.NF() * 3 * sizeof(cy::Point3f), triangleVertexNormals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &UVBSphere);
	glBindBuffer(GL_ARRAY_BUFFER, UVBSphere);

	//Create the array of vertex uvs from the triangle vertex uvs
	cy::Point2f * triangleVertexTextureCoords = new cy::Point2f[sphereMesh.NF() * 3];

	unsigned int vertexTextureIndex = 0;

	for (size_t i = 0; i < sphereMesh.NF(); i++)
	{
		triangleVertexTextureCoords[vertexTextureIndex] = cy::Point2f(sphereMesh.VT(sphereMesh.FT(i).v[0]));
		triangleVertexTextureCoords[vertexTextureIndex + 1] = cy::Point2f(sphereMesh.VT(sphereMesh.FT(i).v[1]));
		triangleVertexTextureCoords[vertexTextureIndex + 2] = cy::Point2f(sphereMesh.VT(sphereMesh.FT(i).v[2]));
		vertexTextureIndex += 3;
	}
	glBufferData(GL_ARRAY_BUFFER, sphereMesh.NF() * 3 * sizeof(cy::Point2f), triangleVertexTextureCoords, GL_STATIC_DRAW);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);


}

void CreateLightMesh()
{
	glGenVertexArrays(1, &VAOLight);
	glBindVertexArray(VAOLight);

	glGenBuffers(1, &VBLight);
	glBindBuffer(GL_ARRAY_BUFFER, VBLight);

	bool result = lightMesh.LoadFromFileObj("Models/light.obj");
	lightMesh.ComputeBoundingBox();

	//Create the array of vertices from the triangle vertices
	cy::Point3f * triangleVertices = new cy::Point3f[lightMesh.NF() * 3];

	unsigned int vertexIndex = 0;

	for (size_t i = 0; i < lightMesh.NF(); i++)
	{
		triangleVertices[vertexIndex] = lightMesh.V(lightMesh.F(i).v[0]);
		triangleVertices[vertexIndex + 1] = lightMesh.V(lightMesh.F(i).v[1]);
		triangleVertices[vertexIndex + 2] = lightMesh.V(lightMesh.F(i).v[2]);
		vertexIndex += 3;
	}

	glBufferData(GL_ARRAY_BUFFER, lightMesh.NF() * 3 * sizeof(cy::Point3f), triangleVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &NBLight);
	glBindBuffer(GL_ARRAY_BUFFER, NBLight);

	lightMesh.ComputeNormals();


	//Create the array of vertex normals from the triangle vertex normals
	cy::Point3f * triangleVertexNormals = new cy::Point3f[lightMesh.NF() * 3];

	unsigned int vertexNormalIndex = 0;

	for (size_t i = 0; i < lightMesh.NF(); i++)
	{
		triangleVertexNormals[vertexNormalIndex] = lightMesh.VN(lightMesh.FN(i).v[0]);
		triangleVertexNormals[vertexNormalIndex + 1] = lightMesh.VN(lightMesh.FN(i).v[1]);
		triangleVertexNormals[vertexNormalIndex + 2] = lightMesh.VN(lightMesh.FN(i).v[2]);
		vertexNormalIndex += 3;
	}

	glBufferData(GL_ARRAY_BUFFER, lightMesh.NF() * 3 * sizeof(cy::Point3f), triangleVertexNormals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
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

void CreateCubeMapTextures()
{
	std::vector<unsigned char> image; //the raw pixels
	unsigned width, height;

	std::vector<std::string> fileNames;
	fileNames.push_back("Textures/CubeTextures/cubemap_posx.png");
	fileNames.push_back("Textures/CubeTextures/cubemap_negx.png");
	fileNames.push_back("Textures/CubeTextures/cubemap_posy.png");
	fileNames.push_back("Textures/CubeTextures/cubemap_negy.png");
	fileNames.push_back("Textures/CubeTextures/cubemap_posz.png");
	fileNames.push_back("Textures/CubeTextures/cubemap_negz.png");

	
	//decode
	unsigned error = lodepng::decode(image, width, height, fileNames[0], LodePNGColorType::LCT_RGB);

	//if there's an error, display it
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	cubeMapTexture.SetImage(cubeMapTexture.POSITIVE_X, image.data(), 3, width, height);
	image.clear();

	//decode
	error = lodepng::decode(image, width, height, fileNames[1], LodePNGColorType::LCT_RGB);

	//if there's an error, display it
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	cubeMapTexture.SetImage(cubeMapTexture.NEGATIVE_X, image.data(), 3, width, height);
	image.clear();

	//decode
	error = lodepng::decode(image, width, height, fileNames[2], LodePNGColorType::LCT_RGB);

	//if there's an error, display it
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	cubeMapTexture.SetImage(cubeMapTexture.POSITIVE_Y, image.data(), 3, width, height);
	image.clear();

	//decode
	error = lodepng::decode(image, width, height, fileNames[3], LodePNGColorType::LCT_RGB);

	//if there's an error, display it
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	cubeMapTexture.SetImage(cubeMapTexture.NEGATIVE_Y, image.data(), 3, width, height);
	image.clear();

	//decode
	error = lodepng::decode(image, width, height, fileNames[4], LodePNGColorType::LCT_RGB);

	//if there's an error, display it
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	cubeMapTexture.SetImage(cubeMapTexture.POSITIVE_Z, image.data(), 3, width, height);
	image.clear();

	//decode
	error = lodepng::decode(image, width, height, fileNames[5], LodePNGColorType::LCT_RGB);

	//if there's an error, display it
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	cubeMapTexture.SetImage(cubeMapTexture.NEGATIVE_Z, image.data(), 3, width, height);
	image.clear();

	cubeMapTexture.SetSeamless();
	cubeMapTexture.SetFilteringMode(GL_LINEAR_MIPMAP_LINEAR, 0);
	cubeMapTexture.SetMaxAnisotropy();
	cubeMapTexture.BuildMipmaps();

}

void CreateDepthMap()
{
	depthMap.Initialize(true, 4096, 4096, GL_DEPTH_COMPONENT24);
	depthMap.SetTextureFilteringMode(GL_LINEAR, GL_LINEAR);
	depthMap.SetTextureWrappingMode(GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	depthMap.SetTextureMaxAnisotropy();
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

void CreateSimpleShader()
{
	//The Shader Program
	cy::GLSLShader vertexShader;
	vertexShader.CompileFile("Shaders/Vertex/simpleShader.cgfx", GL_VERTEX_SHADER);
	cy::GLSLShader fragmentShader;
	fragmentShader.CompileFile("Shaders/Fragment/simpleShader.cgfx", GL_FRAGMENT_SHADER);
	simpleShaderProgram.Build(&vertexShader, &fragmentShader);
}

void CreateShadowMapShader()
{
	//The Shader Program
	cy::GLSLShader vertexShader;
	vertexShader.CompileFile("Shaders/Vertex/shadowMap.cgfx", GL_VERTEX_SHADER);
	cy::GLSLShader fragmentShader;
	fragmentShader.CompileFile("Shaders/Fragment/shadowMap.cgfx", GL_FRAGMENT_SHADER);
	shadowMapShaderProgram.Build(&vertexShader, &fragmentShader);
}

void CreateShadowShader()
{
	//The Shader Program
	cy::GLSLShader vertexShader;
	vertexShader.CompileFile("Shaders/Vertex/shadowShader.cgfx", GL_VERTEX_SHADER);
	cy::GLSLShader fragmentShader;
	fragmentShader.CompileFile("Shaders/Fragment/shadowShader.cgfx", GL_FRAGMENT_SHADER);
	shadowShaderProgram.Build(&vertexShader, &fragmentShader);
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

void CreateCubeShaders()
{
	//The Shader Program
	cy::GLSLShader vertexShader;
	vertexShader.CompileFile("Shaders/Vertex/cube.cgfx", GL_VERTEX_SHADER);
	cy::GLSLShader fragmentShader;
	fragmentShader.CompileFile("Shaders/Fragment/cube.cgfx", GL_FRAGMENT_SHADER);
	cubeShaderProgram.Build(&vertexShader, &fragmentShader);
}

void CreateSphereShaders()
{
	//The Shader Program
	cy::GLSLShader vertexShader;
	vertexShader.CompileFile("Shaders/Vertex/sphere.cgfx", GL_VERTEX_SHADER);
	cy::GLSLShader fragmentShader;
	fragmentShader.CompileFile("Shaders/Fragment/sphere.cgfx", GL_FRAGMENT_SHADER);
	sphereShaderProgram.Build(&vertexShader, &fragmentShader);
}

void Display()
{
	//Create the render target
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//Bind the depth map because we already rendered the scene
	depthMap.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Create the light matrix
	cy::Matrix4f lightProjectionMatrix;
	float right = 50.0f;
	float left = -50.0f;
	float top = 50.0f;
	float bottom = -50.0f;
	float nearPlane = 1.0f;
	float farPlane = 1000.0f;
	lightProjectionMatrix.SetColumn(0,2.0f / (right- left),0.0f,0.0f,0.0f);
	lightProjectionMatrix.SetColumn(1, 0.0f, 2.0f / (top- bottom), 0.0f, 0.0f);
	lightProjectionMatrix.SetColumn(2, 0.0f, 0.0f, -2 / (farPlane - nearPlane), 0.0f);
	lightProjectionMatrix.SetColumn(3, -((right + left)/(right- left)), -((top + bottom) / (top - bottom)), -((farPlane + nearPlane) / (farPlane - nearPlane)), 1.0f);
	cy::Matrix4f lightSpaceMatrix = lightProjectionMatrix * cy::Matrix4f::MatrixView((lightRotationMatrix * lightPositionMatrix).GetTrans(), cy::Point3f(0.0f, 0.0f, 0.0f), cy::Point3f(0.0f, 1.0f, 0.0f));

	//Draw teapot from light's view
	modelMatrix = cy::Matrix4<float>::MatrixTrans(-(mesh.GetBoundMin() + mesh.GetBoundMax())* 0.5f);
	mvp = lightSpaceMatrix * modelMatrix;

	shadowMapShaderProgram.Bind();
	shadowMapShaderProgram.SetUniformMatrix4("mvp", mvp.data);

	//Set the parameters of the renderTexture object
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, mesh.NF() * 3);
	depthMap.Unbind();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Draw the light
	modelMatrix = lightRotationMatrix * lightPositionMatrix;
	viewMatrix = cameraPositionMatrix * cameraRotationMatrix;
	mvp = projectionMatrix * viewMatrix * modelMatrix;

	simpleShaderProgram.Bind();
	simpleShaderProgram.SetUniformMatrix4("mvp", mvp.data);

	//Set the parameters of the renderTexture object
	glBindVertexArray(VAOLight);
	glDrawArrays(GL_TRIANGLES, 0, lightMesh.NF() * 3);

	//Draw teapot
	modelMatrix = cy::Matrix4<float>::MatrixTrans(-(mesh.GetBoundMin() + mesh.GetBoundMax())* 0.5f);
	viewMatrix = cameraPositionMatrix * cameraRotationMatrix;
	lightMatrix = lightRotationMatrix * lightPositionMatrix;
	lightCameraMatrix = viewMatrix * lightRotationMatrix * lightPositionMatrix;
	viewerPosition = viewMatrix * cameraPositionMatrix.GetTrans();

	mv = viewMatrix * modelMatrix;
	mvp = projectionMatrix * viewMatrix * modelMatrix;
	inverseTransposeOfView = (viewMatrix * modelMatrix).GetSubMatrix3();
	inverseTransposeOfView.Invert();
	inverseTransposeOfView.Transpose();
	cameraDistance = cameraPositionMatrix.GetTrans().Length();

	shadowShaderProgram.Bind();
	shadowShaderProgram.SetUniform("diffuse", cy::Point3f(0.5,0.2,0.5));
	shadowShaderProgram.SetUniformMatrix4("mv", mv.data);
	shadowShaderProgram.SetUniformMatrix4("mvp", mvp.data);
	shadowShaderProgram.SetUniformMatrix4("lightSpaceMatrix", lightSpaceMatrix.data);
	shadowShaderProgram.SetUniformMatrix3("inverseCM", inverseTransposeOfView.data);
	shadowShaderProgram.SetUniformMatrix4("modelMatrix", modelMatrix.data);
	shadowShaderProgram.SetUniformMatrix4("viewMatrix", viewMatrix.data);
	shadowShaderProgram.SetUniform("worldLightPosition", lightMatrix.GetTrans());
	shadowShaderProgram.SetUniform("lightPosition", lightCameraMatrix.GetTrans());
	shadowShaderProgram.SetUniform("viewerPosition", viewerPosition);

	//Set the parameters of the renderTexture object
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, mesh.NF() * 3);

	//Draw Plane
	modelMatrix = planeRotationMatrix * planePositionMatrix;
	viewMatrix = cameraPositionMatrix * cameraRotationMatrix;
	mvp = projectionMatrix * viewMatrix * modelMatrix;
	viewerPosition = viewMatrix * cameraPositionMatrix.GetTrans();

	inverseTransposeOfView = (viewMatrix * modelMatrix).GetSubMatrix3();
	inverseTransposeOfView.Invert();
	inverseTransposeOfView.Transpose();
	
	shadowShaderProgram.Bind();
	shadowShaderProgram.SetUniform("diffuse", cy::Point3f(0.5, 0.5, 0.5));
	shadowShaderProgram.SetUniformMatrix4("mvp", mvp.data);
	shadowShaderProgram.SetUniformMatrix4("lightSpaceMatrix", lightSpaceMatrix.data);
	shadowShaderProgram.SetUniformMatrix3("inverseCM", inverseTransposeOfView.data);
	shadowShaderProgram.SetUniformMatrix4("modelMatrix", modelMatrix.data);
	shadowShaderProgram.SetUniformMatrix4("viewMatrix", viewMatrix.data);
	shadowShaderProgram.SetUniform("worldLightPosition", lightMatrix.GetTrans());
	shadowShaderProgram.SetUniform("lightPosition", lightCameraMatrix.GetTrans());
	shadowShaderProgram.SetUniform("viewerPosition", viewerPosition);

	glActiveTexture(GL_TEXTURE0);
	depthMap.BindTexture();

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
	glutInitWindowSize((int)screenWidth,(int)screenHeight);
	glutInitWindowPosition(0,0);
	glutInitContextVersion(3, 3);
	glutCreateWindow("Reflections");
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

	CreateShaders();

	CreateDepthMap();
	CreateShadowMapShader();
	CreateShadowShader();

	//Create the plane Mesh
	CreatePlaneMesh(50.0f);
	CreatePlaneShaders();

	//Create the sphere Mesh
	CreateSphereMesh();
	CreateSphereShaders();

	//Create the light Mesh
	CreateLightMesh();
	CreateSimpleShader();

	glutMainLoop();
	return 0;
}