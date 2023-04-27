#include <iostream>
#include <chrono>
#include <filesystem>
#include "ourGraphics.h"
#include "ourGraphicsFreeType.h"
#include "ourGraphicsMeshes.h"

using namespace std::chrono;

steady_clock::time_point lastUpdate = steady_clock::now();

int winx=0.0f, winy=0.0f;

float rot_angle = 0.0f;
float y_pos = 0.0f;
float ampl = 30.0f;

float getDeltaTime()
{
	auto now = std::chrono::steady_clock::now();
	float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - lastUpdate).count() / 1000000.0f;
	lastUpdate = now;
	return deltaTime;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

int main()
{
	OGLManager oMan(800, 600, framebuffer_size_callback);
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glEnable(GL_DEPTH_TEST);

	Shader * shad = new Shader("shader_font_vert.gls", "shader_font_frag.gls");

	unsigned int tex = makeTexture("test_img.png");
	unsigned int tex2 = makeTexture("test_img2.png");
	unsigned int tex3 = makeTexture("test_img3.png");
	unsigned int tex4 = makeTexture("house.png");

	//Model objMod("shtuka.obj");
	//Model objMod("crank_handle.obj");
	Model objMod("house_fbx.fbx");

	while (!glfwWindowShouldClose(oMan.window))
	{

		float deltaTime = getDeltaTime();

		glfwGetWindowSize(oMan.window, &winx, &winy);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//draw first object

		oMan.setDefaultProjections();

		oMan.setProjection(glm::perspectiveFov(
			(float)winx/(float)winy,
			(float)winx, (float)winy, 0.01f, 1000.0f)
		);

		oMan.setView(glm::translate(oMan.getView(),
			glm::vec3(0.0f,0.0f,-100.0f)));

		//orient model as we need
		oMan.setModel(
			glm::rotate(
				oMan.getModel(), 
				glm::radians(180.0f),
				glm::vec3(0.0f, 0.0f, 1.0f)
			)
		);

		//rotate model in real time

		rot_angle += 10.0f * deltaTime;

		oMan.setModel(
			glm::rotate(
				oMan.getModel(),
				glm::radians(rot_angle),
				glm::vec3(0.0f, 1.0f, 0.0f)
			)
		);

		//translate model in real time
		y_pos = ampl * sin((10 * rot_angle)*3.14f / 180.0f);
		oMan.setModel(
			glm::translate(
				oMan.getModel(),
				glm::vec3(0.0f, y_pos, 0.0f)
			)
		);

		shad->use();
		shad->setMatrix4f("model", oMan.getModel());
		shad->setMatrix4f("view", oMan.getView());
		shad->setMatrix4f("projection", oMan.getProjection());

		drawPlane(shad,glm::vec3(0,0,0),glm::vec3(50.0f,50.0f,0.0f),
			glm::vec3(1.0),tex);

		//draw second object

		oMan.setDefaultProjections();

		oMan.setProjection(glm::perspectiveFov(
			(float)winx / (float)winy,
			(float)winx, (float)winy, 0.01f, 1000.0f)
		);

		oMan.setView(glm::translate(oMan.getView(),
			glm::vec3(0.0f, 0.0f, -100.0f)));

		oMan.setModel(
			glm::translate(
				oMan.getModel(),
				glm::vec3(-60.0f, 0.0f, -20.0f)
			)
		);

		oMan.setModel(
			glm::rotate(
				oMan.getModel(),
				glm::radians(rot_angle),
				glm::vec3(0.0f, 1.0f, 0.0f)
			)
		);

		shad->setMatrix4f("model", oMan.getModel());
		shad->setMatrix4f("view", oMan.getView());
		shad->setMatrix4f("projection", oMan.getProjection());

		drawCube(shad, glm::vec3(0.0f), glm::vec3(25.0f), glm::vec3(1.0f), tex2);

		//drawmodel

		oMan.setDefaultProjections();

		oMan.setProjection(glm::perspectiveFov(
			(float)winx / (float)winy,
			(float)winx, (float)winy, 0.01f, 1000.0f)
		);

		oMan.setView(glm::translate(oMan.getView(),
			glm::vec3(0.0f, 0.0f, -100.0f)));

		oMan.setModel(
			glm::translate(
				oMan.getModel(),
				glm::vec3(60.0f, 0.0f, -20.0f)
			)
		);

		oMan.setModel(
			glm::scale(
				oMan.getModel(),
				glm::vec3(20.0f, 20.0f, 20.0f)
			)
		);

		oMan.setModel(
			glm::rotate(
				oMan.getModel(),
				glm::radians(rot_angle),
				glm::vec3(0.0f, 1.0f, 0.0f)
			)
		);

		shad->setMatrix4f("model", oMan.getModel());
		shad->setMatrix4f("view", oMan.getView());
		shad->setMatrix4f("projection", oMan.getProjection());

		glBindTexture(GL_TEXTURE_2D, tex4);

		objMod.Draw(shad);
		
		oMan.endDraw();

	}
}