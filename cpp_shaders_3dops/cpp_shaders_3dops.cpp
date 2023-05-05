#include <iostream>
#include <chrono>
#include <filesystem>
#include <algorithm>
#include <vector>
#include "ourGraphics.h"
#include "ourGraphicsFreeType.h"
#include "ourGraphicsMeshes.h"

using namespace std::chrono;

bool drawChart = true;

steady_clock::time_point lastUpdate = steady_clock::now();

int winx=0.0f, winy=0.0f;

float rot_angle = 0.0f;
float y_pos = 0.0f;
float ampl = 30.0f;

float lcr = 0.0f, lcb = 1.0f, lcg = 0.5;
float spr = 0.2f, spb = -0.2f, spg = 0.2f;

//lighting
glm::vec3 ambient_colour = {1.0, 0.2, 0.2};
float ambient_brightness = 0.3f;

glm::vec3 light_colour = { 1.0, 1.0, 1.0 };
glm::vec3 light_pos = { 0, 70, 20 };
glm::vec3 camera_pos = {0,0,-100};
float light_brightness = 1.0f;
float specular_brightness = 0.5f;

std::vector<glm::vec3> chart_points = {};

//make our data for surface generation
void fill_chart(float x1, float x2, float y1, float y2, int density, std::vector<glm::vec3> &cdata)
{
	cdata.clear();
	float dx = (x2 - x1) / (float)density;
	float dy = (y2 - y1) / (float)density;
	for (int j = 0; j < density; j++)
		for (int i = 0; i < density; i++)
		{
			float cx = x1 + dx * (float)i;
			float cy = y1 + dy * (float)j;
			float cz = 5*sin(0.3*cx) + 10 * cos(0.5 * cy); //any function
			cdata.push_back(glm::vec3(cx,cy,cz));
		}
}

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

void updateColorMotion(float &vval, float &vspd, float dt)
{
	vval += vspd * dt;
	if ((vval > 1) || (vval < 0)) { vspd *= -1.0f; clampVal(vval, 0.0f, 1.0f); }
}

int main()
{

	fill_chart(-100,100,-100,100,100,chart_points);
	
	OGLManager oMan(800, 600, framebuffer_size_callback);
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glEnable(GL_DEPTH_TEST);

	oMan.addShader("shader_light_vert.gls", "shader_light_frag.gls");
	oMan.addShader("shader_chart_vert.gls", "shader_chart_frag.gls");

	unsigned int tex = makeTexture("test_img.png");
	unsigned int tex2 = makeTexture("test_img2.png");
	unsigned int tex3 = makeTexture("test_img3.png");
	unsigned int tex4 = makeTexture("house.png");

	//Model objMod("shtuka.obj");
	//Model objMod("crank_handle.obj");
	Model objMod("house_obj.obj");

	while (!glfwWindowShouldClose(oMan.window))
	{
		float deltaTime = getDeltaTime();

		glfwGetWindowSize(oMan.window, &winx, &winy);

		glm::mat4 mat_persp = glm::perspectiveFov(
			(float)winx / (float)winy,
			(float)winx, (float)winy, 0.01f, 1000.0f);

		glm::mat4 mat_view = glm::translate(glm::mat4(1.0),
			camera_pos);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//set lighting uniforms for shader
		oMan.useShader(0);
		oMan.getShader(0)->setBool("useColour", true);
		oMan.getShader(0)->setBool("useTexture", true);
		oMan.getShader(0)->setBool("light.use", true);
				
		updateColorMotion(lcr, spr, deltaTime);
		updateColorMotion(lcg, spg, deltaTime);
		updateColorMotion(lcb, spb, deltaTime);
		
		light_colour.r = lcr; light_colour.g = lcg; light_colour.b = lcb;

		float lcx, lcy, lcz;

		lcx = 100.0f * sin(0.25f*rot_angle);
		lcy = 100.0f * cos(0.25f*rot_angle);
		lcz = 50.0f * sin(0.25f*rot_angle);

		light_pos.x = lcx; light_pos.y = lcy; light_pos.z = lcz;

		oMan.getShader(0)->setVector3f("light.ambientColor", ambient_colour.r, ambient_colour.g, ambient_colour.b);
		oMan.getShader(0)->setFloat("light.ambientBrightness", ambient_brightness);

		oMan.getShader(0)->setVector3f("light.pos", light_pos.x, light_pos.y, light_pos.z);
		oMan.getShader(0)->setVector3f("light.diffuseColor", light_colour.r, light_colour.g, light_colour.b);
		oMan.getShader(0)->setFloat("light.diffuseBrightness", light_brightness);

		oMan.getShader(0)->setVector3f("camPos", camera_pos.x, camera_pos.y, camera_pos.z);
		oMan.getShader(0)->setFloat("light.specularBrightness", specular_brightness);

		oMan.setDefaultProjections();

		oMan.setProjection(mat_persp);
		oMan.setView(mat_view);

		if (!drawChart)
		{
			//draw first object

			//orient model as we need
			oMan.rotateModel(180.0f, glm::vec3(0.0f, 0.0f, 1.0f));

			//rotate model in real time

			rot_angle += 10.0f * deltaTime;

			oMan.rotateModel(rot_angle, glm::vec3(0.0f, 1.0f, 0.0f));

			//translate model in real time
			y_pos = ampl * sin((10 * rot_angle)*3.14f / 180.0f);

			oMan.translateModel(glm::vec3(0.0f, y_pos, 0.0f));

			oMan.updateProjectionForShader(0);

			drawPlane(oMan.getShader(0), glm::vec3(0, 0, 0), glm::vec3(50.0f, 50.0f, 0.0f),
				glm::vec3(1.0), tex, true);

			//draw second object

			oMan.resetModel();

			oMan.translateModel(glm::vec3(-60.0f, 0.0f, -20.0f));
			oMan.rotateModel(rot_angle, glm::vec3(0.0f, 1.0f, 0.0f));

			oMan.updateProjectionForShader(0);
			oMan.getShader(0)->setBool("useColour", false);
			oMan.getShader(0)->setBool("useTexture", true);
			oMan.getShader(0)->setBool("light.use", true);
			drawCube(oMan.getShader(0), glm::vec3(0.0f), glm::vec3(25.0f), glm::vec3(1.0f), tex2, true);

			//drawmodel

			oMan.resetModel();

			oMan.translateModel(glm::vec3(60.0f, 0.0f, -20.0f));
			oMan.rotateModel(rot_angle, glm::vec3(0.0f, 1.0f, 0.0f));

			oMan.updateProjectionForShader(0);
			oMan.getShader(0)->setBool("useColour", false);
			oMan.getShader(0)->setBool("useTexture", true);
			oMan.getShader(0)->setBool("light.use", true);

			glBindTexture(GL_TEXTURE_2D, tex4);

			objMod.Draw(oMan.getShader(0));

			//draw big botom plane
			oMan.resetModel();

			oMan.translateModel(glm::vec3(0.0f, -110.0f, 0.0f));
			oMan.rotateModel(-90, glm::vec3(1.0f, 0.0f, 0.0f));

			oMan.updateProjectionForShader(0);
			oMan.getShader(0)->setBool("useColour", true);
			oMan.getShader(0)->setBool("useTexture", false);
			oMan.getShader(0)->setBool("light.use", true);

			drawPlane(oMan.getShader(0), glm::vec3(0, 0, 0), glm::vec3(250.0f, 250.0f, 0.0f),
				glm::vec3(1.0, 0.4, 0.4), 0, false);

			//draw source
			oMan.resetModel();

			oMan.translateModel(light_pos);

			oMan.updateProjectionForShader(1);
			drawCube(oMan.getShader(1), glm::vec3(0.0f), glm::vec3(9.0f, 9.0f, 9.0f), light_colour, 0, false);
		}
		else
		{
			//set modes
			oMan.getShader(0)->setBool("useColour", true);
			oMan.getShader(0)->setBool("useTexture", false);
			oMan.getShader(0)->setBool("light.use", true);

			//orient model as we need
			oMan.rotateModel(-45.0f, glm::vec3(1.0f, 0.0f, 0.0f));

			//rotate model in real time

			rot_angle += 10.0f * deltaTime;

			oMan.rotateModel(rot_angle, glm::vec3(0.0f, 0.0f, 1.0f));
			oMan.updateProjectionForShader(0);

			drawSurface(oMan.getShader(0), chart_points);

		}

		oMan.endDraw();

	}
}