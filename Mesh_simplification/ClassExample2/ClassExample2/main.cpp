#include <iostream>
#include "display.h"
#include "inputManager.h"

Display display(DISPLAY_WIDTH, DISPLAY_HEIGHT, "OpenGL");	
Scene scn(glm::vec3(0.0f, 0.0f, -15.0f), CAM_ANGLE, relation, NEAR, FAR);

int main(int argc, char** argv)
{
	initCallbacks(display);
	//scn.addShape("./res/objs/cube.obj","./res/textures/box0.bmp");
	//scn.addShape("./res/objs/monkey3.obj","./res/texture/box0.bmp");
	int maxfaces = 500;
	//scn.addShape("./res/objs/ball.obj",false,maxfaces);
	scn.addShape("./res/objs/monkey3.obj", true, maxfaces);
	scn.addShape("./res/objs/monkey3.obj", false, maxfaces);
	//scn.addShape("./res/objs/testBoxNoUV.obj",true,2);


	scn.addShader("./res/shaders/basicShader");
	scn.addShader("./res/shaders/pickingShader");

	while(!display.toClose())
	{
		display.Clear(0.7f, 0.7f, 0.7f, 1.0f);
		scn.draw(0,0,true);
		//scn.myRotate(0.04, glm:: vec3(0, 1, 0));
		display.SwapBuffers();
		display.pullEvent();
	}




	return 0;
}
