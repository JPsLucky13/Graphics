#include <GL/freeglut.h>

//Clear color values
float red = 0.25f;
float green = 0.1f;
float blue = 0.2f;
float alpha = 0.0f;
float colorChangeSpeed = 0.005f;

void Display()
{
	glClearColor(GLclampf(red), GLclampf(green), GLclampf(blue), GLclampf(alpha));
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
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(512, 512);
	glutInitWindowPosition(0,0);
	glutCreateWindow("Hello World");
	glutDisplayFunc(Display);
	glutKeyboardFunc(Keyboard);
	glutIdleFunc(Idle);
	glutMainLoop();

	return 0;
}