#include <GL/glew.h>
#include <GL/freeglut.h>
#include <../../Utilities/cyTriMesh.h>

//Clear color values
float red = 0.25f;
float green = 0.1f;
float blue = 0.2f;
float alpha = 0.0f;
float colorChangeSpeed = 0.005f;

GLuint VBO;
GLuint VAO;
cy::TriMesh mesh;

void CreateMesh()
{
	bool result = mesh.LoadFromFileObj("Models/teapot.obj");
}

void Display()
{
	//glClearColor(GLclampf(red), GLclampf(green), GLclampf(blue), GLclampf(alpha));
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
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



int main(int argc, char* argv[])
{
	//Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(512, 512);
	glutInitWindowPosition(0,0);
	glutCreateWindow("Hello World");
	glutKeyboardFunc(Keyboard);
	
	
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
	glDrawArrays(GL_POINTS,0,1);

	glutDisplayFunc(Display);
	glutIdleFunc(Idle);
	glutMainLoop();

	return 0;
}