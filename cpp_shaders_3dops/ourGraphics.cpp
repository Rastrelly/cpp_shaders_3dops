#include "ourGraphics.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool fileWrote = false;
bool surfVertNeedUpdate = false;
flarr surfVerts = {};
intarr surfInds = {};

vector<glm::vec3> colourtable =
{
glm::vec3(163.0f / 255.0f,  73.0f / 255.0f, 163.0f / 255.0f), //purple
glm::vec3(63.0f / 255.0f,  72.0f / 255.0f, 204.0f / 255.0f), //blue
glm::vec3(0.0f / 255.0f, 162.0f / 255.0f, 232.0f / 255.0f), //light blue
glm::vec3(34.0f / 255.0f, 177.0f / 255.0f,  76.0f / 255.0f), //green
glm::vec3(255.0f / 255.0f, 243.0f / 255.0f,   0.0f / 255.0f), //yellow
glm::vec3(255.0f / 255.0f, 127.0f / 255.0f,  39.0f / 255.0f), //orange
glm::vec3(237.0f / 255.0f,  28.0f / 255.0f,  36.0f / 255.0f)  //red
};

OGLManager::OGLManager(int pwx, int pwy, GLFWframebuffersizefun callback)
{
	initOGL(pwx, pwy, callback);
	setDefaultProjections();
}

void OGLManager::addShader(const char* vertexPath, const char* fragmentPath)
{
	shaders.push_back(new Shader(vertexPath, fragmentPath));
}

void OGLManager::useShader(int id)
{
	if (id<shaders.size()) shaders[id]->use();
	else std::cout << "ERROR: No such shader id!\n";
}

Shader * OGLManager::getShader(int id)
{
	if (id < shaders.size())
	{
		return shaders[id];
	}
	else
	{
		std::cout << "ERROR: No such shader id!\n";
		return NULL;
	}
}

unsigned int makeTexture(string fileName)
{
	unsigned int tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int iw, ih, nChan;
	unsigned char *data = stbi_load(fileName.c_str(), &iw, &ih, &nChan, 0);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iw, ih, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Texture load failed " << fileName << endl;
	}

	stbi_image_free(data);

	return tex;
}

bool OGLManager::initOGL(int pwx, int pwy, GLFWframebuffersizefun callback)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(pwx, pwy, "Main Window", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);

	gladLoaded = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	if (!gladLoaded)
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glfwSetFramebufferSizeCallback(window, callback);

	return true;
}

void OGLManager::endDraw()
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void OGLManager::updateProjectionForShader(int id)
{
	if (id < shaders.size())
	{
		shaders[id]->use();
		shaders[id]->setMatrix4f("projection", getProjection());
		shaders[id]->setMatrix4f("view", getView());
		shaders[id]->setMatrix4f("model", getModel());
	}
	else
	{
		std::cout << "ERROR: No such shader id!\n";
	}	
}

void OGLManager::translateModel(glm::vec3 dir)
{
	setModel(
		glm::translate(
			getModel(),
			dir
		)
	);
}

void OGLManager::rotateModel(float ang, glm::vec3 axis)
{
	setModel(
		glm::rotate(
			getModel(),
			glm::radians(ang),
			axis
		)
	);
}

void drawOurVBO(flarr verts, int verts_block_size, GLenum objType, int vertAttrSize)
{
	//verts_block_size defines how many layouts
	//are transmited:

	/*
	 3- XYZ
	 6- XYZRGB
	 8- XYZRGBUV
	*/


	//buffers
	unsigned int VBO; //vertex buffer
	unsigned int VAO; //vertex array

	glGenBuffers(1, &VBO); //generate vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO); //assign data type to buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*verts.size(), verts.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	if (verts_block_size >= 3)
	{
		glVertexAttribPointer(0, vertAttrSize, GL_FLOAT, GL_FALSE, verts_block_size * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	if (verts_block_size >= 6)
	{
		glVertexAttribPointer(1, vertAttrSize, GL_FLOAT, GL_FALSE, verts_block_size * sizeof(float), (void*)(vertAttrSize * sizeof(float)));
		glEnableVertexAttribArray(1);
	}

	if (verts_block_size >= 8)
	{
		glVertexAttribPointer(2, vertAttrSize, GL_FLOAT, GL_FALSE, verts_block_size * sizeof(float), (void*)(2 * vertAttrSize * sizeof(float)));
		glEnableVertexAttribArray(2);
	}
	glDrawArrays(objType, 0, verts.size() / verts_block_size);

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void drawOurEBO(flarr verts, intarr inds, unsigned int tex, int verts_block_size, bool usetex)
{

	//buffers
	unsigned int VBO; //vertex buffer
	unsigned int EBO; //elemnt buffer
	unsigned int VAO; //vertex array

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO); //generate vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO); //assign data type to buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*verts.size(), verts.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &EBO); //generate element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); //assgn data type
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*inds.size(), inds.data(), GL_STATIC_DRAW);

	glBindVertexArray(VAO);

	if (verts_block_size == 3)
	{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, verts_block_size * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
	}

	if (verts_block_size == 6)
	{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, verts_block_size * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, verts_block_size * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
	}

	if (verts_block_size == 9)
	{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, verts_block_size * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, verts_block_size * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, verts_block_size * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
	}

	if (verts_block_size == 11)
	{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, verts_block_size * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, verts_block_size * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, verts_block_size * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, verts_block_size * sizeof(float), (void*)(9 * sizeof(float)));
		glEnableVertexAttribArray(3);
	}

	if (usetex)
		glBindTexture(GL_TEXTURE_2D, tex);
	else
		glBindTexture(GL_TEXTURE_2D, 0);

	glDrawElements(GL_TRIANGLES, inds.size(), GL_UNSIGNED_INT, 0);

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &EBO);
}

int getSymbolId(char smb)
{
	char smbToFind = tolower(smb);
	vector<char> symbols = symbolsList();
	int fid = -1;
	for (int i = 0; i < symbols.size(); i++)
	{
		if (smbToFind == symbols[i]) { fid = i; return fid; }
	}
	return fid;
}

void getSymbolCoords(int rowWidth, int smbId, float &sx, float &sy, float &sw)
{
	int c=0, r=0, cId=0;
	while (cId < smbId)
	{
		cId++;
		c++;
		if (c > rowWidth-1) { c = 0; r++; }
	}

	float pw = (1.0f / (float)rowWidth);

	sx = (float)c * pw;
	sy = (float)r * pw;
	sw = pw;
}

smbarr symbolsList()
{
  return { 'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t', 'u','v','w','x','y','z','.',',','-','+','=','/','?','\'','$','#','\"','(',')','_','!',' ','1','2','3','4','5','6','7','8','9','0'};
}

void printBitmapText(Shader * sh, float tx, float ty, float size, string txt, unsigned int fontTex)
{
	sh->use();

	int l = txt.length();
	for (int i = 0; i < l; i++)
	{
		char cs = txt[i];

		float symX = 0, symY = 0, symW = 0;

		getSymbolCoords(8,getSymbolId(cs),symX,symY,symW);

		flarr letter_coords = {};
		intarr letter_inds = {};

		//vertex array
		//1
		letter_coords.push_back(tx + size * i); letter_coords.push_back(ty); letter_coords.push_back(0.0f); //vert coords
		letter_coords.push_back(1.0f); letter_coords.push_back(1.0f); letter_coords.push_back(1.0f); //vert clr
		letter_coords.push_back(symX); letter_coords.push_back(symY+symW); //tex coords
		//2
		letter_coords.push_back(tx + size + size * i); letter_coords.push_back(ty); letter_coords.push_back(0.0f); //vert coords
		letter_coords.push_back(1.0f); letter_coords.push_back(1.0f); letter_coords.push_back(1.0f); //vert clr
		letter_coords.push_back(symX + symW); letter_coords.push_back(symY + symW); //tex coords
		//3
		letter_coords.push_back(tx + size * i); letter_coords.push_back(ty + size); letter_coords.push_back(0.0f); //vert coords
		letter_coords.push_back(1.0f); letter_coords.push_back(1.0f); letter_coords.push_back(1.0f); //vert clr
		letter_coords.push_back(symX); letter_coords.push_back(symY); //tex coords
		//4
		letter_coords.push_back(tx + size + size * i); letter_coords.push_back(ty + size); letter_coords.push_back(0.0f); //vert coords
		letter_coords.push_back(1.0f); letter_coords.push_back(1.0f); letter_coords.push_back(1.0f); //vert clr
		letter_coords.push_back(symX+symW); letter_coords.push_back(symY); //tex coords

		//index array
		letter_inds.push_back(0); letter_inds.push_back(1); letter_inds.push_back(2);
		letter_inds.push_back(1); letter_inds.push_back(2); letter_inds.push_back(3);

		glBindTexture(GL_TEXTURE_2D, fontTex);

		drawOurEBO(letter_coords, letter_inds, fontTex, 8, true);

	}
}

float valToDevice(float dimension, float value)
{
	float coeff = 2.0f / dimension;
	return (value * coeff) - 1;
}

void drawChartLine(Shader * shad,vec3arr chartPoints, glm::vec3 colour, float horScale, float vertScale, float depthScale)
{
	shad->use();
	drawOurVBO(pointArrToFlArr(chartPoints, colour, horScale, vertScale, depthScale), 6, GL_LINE_STRIP, 3);
}

flarr pointArrToFlArr(vec3arr cdata, glm::vec3 colour, float xscale, float yscale, float zscale)
{
	flarr dat = {};
	for (int i = 0; i < cdata.size(); i++)
	{
		dat.push_back(scaleVal(cdata[i].x, xscale));
		dat.push_back(scaleVal(cdata[i].y, yscale));
		dat.push_back(scaleVal(cdata[i].z, zscale));
		dat.push_back(colour.r);
		dat.push_back(colour.g);
		dat.push_back(colour.b);
	}
	return dat;
}

float scaleVal(float val, float scale)
{
	return val * scale;
}

void drawLine(Shader * shad,glm::vec3 p1, glm::vec3 p2, glm::vec3 colour)
{
	shad->use();
	drawOurVBO(pointArrToFlArr({ p1, p2 }, colour, 1.0f, 1.0f, 1.0f), 6, GL_LINES, 3);
}

void DCappendPoint(glm::vec3 center, glm::vec3 radius, glm::vec3 d, glm::vec3 colour, glm::vec3 normal, glm::vec2 txcoords, flarr &verts)
{
	glm::vec3 cp = center + d * radius;
	verts.push_back(cp.x);
	verts.push_back(cp.y);
	verts.push_back(cp.z);
	verts.push_back(colour.r);
	verts.push_back(colour.g);
	verts.push_back(colour.b);
	verts.push_back(normal.x);
	verts.push_back(normal.y);
	verts.push_back(normal.z);
	verts.push_back(txcoords.x);
	verts.push_back(txcoords.y);
}

void drawCube(Shader * shad, glm::vec3 center, glm::vec3 radius, glm::vec3 colour, unsigned int tex, bool usetex)
{
	shad->use();
	flarr cubeVertices = {};
	intarr cubeIndices = {};

	//front (1-3)
	//0
	DCappendPoint(center, radius, glm::vec3(-1, -1, -1), colour, glm::vec3(0, 0, -1), glm::vec2(0, 0), cubeVertices);
	//1
	DCappendPoint(center, radius, glm::vec3(-1, 1, -1), colour, glm::vec3(0, 0, -1), glm::vec2(0, 1), cubeVertices);
	//2
	DCappendPoint(center, radius, glm::vec3(1, 1, -1), colour, glm::vec3(0, 0, -1), glm::vec2(1, 1), cubeVertices);
	//3
	DCappendPoint(center, radius, glm::vec3(1, -1, -1), colour, glm::vec3(0, 0, -1), glm::vec2(1, 0), cubeVertices);

	//right
	//4
	DCappendPoint(center, radius, glm::vec3(1, -1, -1), colour, glm::vec3(1, 0, 0), glm::vec2(0, 0), cubeVertices);
	//5
	DCappendPoint(center, radius, glm::vec3(1, 1, -1), colour, glm::vec3(1, 0, 0), glm::vec2(0, 1), cubeVertices);
	//6
	DCappendPoint(center, radius, glm::vec3(1, 1, 1), colour, glm::vec3(1, 0, 0), glm::vec2(1, 1), cubeVertices);
	//7
	DCappendPoint(center, radius, glm::vec3(1, -1, 1), colour, glm::vec3(1, 0, 0), glm::vec2(1, 0), cubeVertices);
	
	//back	
	//8
	DCappendPoint(center, radius, glm::vec3(1, -1, 1), colour, glm::vec3(0, 0, 1), glm::vec2(0, 0), cubeVertices);
	//9
	DCappendPoint(center, radius, glm::vec3(1, 1, 1), colour, glm::vec3(0, 0, 1), glm::vec2(0, 1), cubeVertices);
	//10
	DCappendPoint(center, radius, glm::vec3(-1, 1, 1), colour, glm::vec3(0, 0, 1), glm::vec2(1, 1), cubeVertices);
	//11
	DCappendPoint(center, radius, glm::vec3(-1, -1, 1), colour, glm::vec3(0, 0, 1), glm::vec2(1, 0), cubeVertices);

	//left
	//12
	DCappendPoint(center, radius, glm::vec3(-1, -1, 1), colour, glm::vec3(-1, 0, 0), glm::vec2(0, 0), cubeVertices);
	//13
	DCappendPoint(center, radius, glm::vec3(-1, 1, 1), colour, glm::vec3(-1, 0, 0), glm::vec2(0, 1), cubeVertices);
	//14
	DCappendPoint(center, radius, glm::vec3(-1, 1, -1), colour, glm::vec3(-1, 0, 0), glm::vec2(1, 1), cubeVertices);
	//15
	DCappendPoint(center, radius, glm::vec3(-1, -1, -1), colour, glm::vec3(-1, 0, 0), glm::vec2(1, 0), cubeVertices);
	
	//top
	//16
	DCappendPoint(center, radius, glm::vec3(-1, 1, -1), colour, glm::vec3(0, 0, 1), glm::vec2(0, 0), cubeVertices);
	//17
	DCappendPoint(center, radius, glm::vec3(-1, 1, 1), colour, glm::vec3(0, 0, 1), glm::vec2(0, 1), cubeVertices);
	//18
	DCappendPoint(center, radius, glm::vec3(1, 1, 1), colour, glm::vec3(0, 0, 1), glm::vec2(1, 1), cubeVertices);
	//19
	DCappendPoint(center, radius, glm::vec3(1, 1, -1), colour, glm::vec3(0, 0, 1), glm::vec2(1, 0), cubeVertices);

	//bottom
	//20
	DCappendPoint(center, radius, glm::vec3(-1, -1, -1), colour, glm::vec3(0, 0, -1), glm::vec2(0, 0), cubeVertices);
	//21
	DCappendPoint(center, radius, glm::vec3(-1, -1, 1), colour, glm::vec3(0, 0, -1), glm::vec2(0, 1), cubeVertices);
	//22
	DCappendPoint(center, radius, glm::vec3(1, -1, 1), colour, glm::vec3(0, 0, -1), glm::vec2(1, 1), cubeVertices);
	//23
	DCappendPoint(center, radius, glm::vec3(1, -1, -1), colour, glm::vec3(0, 0, -1), glm::vec2(1, 0), cubeVertices);
	

	//front
	cubeIndices.push_back(0);
	cubeIndices.push_back(1);
	cubeIndices.push_back(2);
	cubeIndices.push_back(0);
	cubeIndices.push_back(2);
	cubeIndices.push_back(3);
	//r
	cubeIndices.push_back(4);
	cubeIndices.push_back(5);
	cubeIndices.push_back(6);
	cubeIndices.push_back(4);
	cubeIndices.push_back(6);
	cubeIndices.push_back(7);
	//3
	cubeIndices.push_back(8);
	cubeIndices.push_back(9);
	cubeIndices.push_back(10);
	cubeIndices.push_back(8);
	cubeIndices.push_back(10);
	cubeIndices.push_back(11);
	//4
	cubeIndices.push_back(12);
	cubeIndices.push_back(13);
	cubeIndices.push_back(14);
	cubeIndices.push_back(12);
	cubeIndices.push_back(14);
	cubeIndices.push_back(15);
	//5
	cubeIndices.push_back(16);
	cubeIndices.push_back(17);
	cubeIndices.push_back(18);
	cubeIndices.push_back(16);
	cubeIndices.push_back(18);
	cubeIndices.push_back(19);
	//6
	cubeIndices.push_back(20);
	cubeIndices.push_back(21);
	cubeIndices.push_back(22);
	cubeIndices.push_back(20);
	cubeIndices.push_back(22);
	cubeIndices.push_back(23);

	drawOurEBO(cubeVertices, cubeIndices, tex, 11, usetex);
}


void drawPlane(Shader * shad, glm::vec3 p1, glm::vec3 radius, glm::vec3 colour, unsigned int tex, bool usetex)
{
	shad->use();

	flarr verts = {};
	intarr inds = {0, 1, 3, 1, 3, 2};

	//0
	verts.push_back(p1.x - radius.x);
	verts.push_back(p1.y - radius.y);
	verts.push_back(p1.z);

	verts.push_back(colour.x);
	verts.push_back(colour.y);
	verts.push_back(colour.z);

	verts.push_back(0);
	verts.push_back(0);
	verts.push_back(1);

	verts.push_back(0.0f);
	verts.push_back(0.0f);

	//1
	verts.push_back(p1.x - radius.x);
	verts.push_back(p1.y + radius.y);
	verts.push_back(p1.z);

	verts.push_back(colour.x);
	verts.push_back(colour.y);
	verts.push_back(colour.z);

	verts.push_back(0);
	verts.push_back(0);
	verts.push_back(1);

	verts.push_back(0.0f);
	verts.push_back(1.0f);

	//2
	verts.push_back(p1.x + radius.x);
	verts.push_back(p1.y + radius.y);
	verts.push_back(p1.z);

	verts.push_back(colour.x);
	verts.push_back(colour.y);
	verts.push_back(colour.z);

	verts.push_back(0);
	verts.push_back(0);
	verts.push_back(1);

	verts.push_back(1.0f);
	verts.push_back(1.0f);

	//3
	verts.push_back(p1.x + radius.x);
	verts.push_back(p1.y - radius.y);
	verts.push_back(p1.z);

	verts.push_back(colour.x);
	verts.push_back(colour.y);
	verts.push_back(colour.z);

	verts.push_back(0);
	verts.push_back(0);
	verts.push_back(1);

	verts.push_back(1.0f);
	verts.push_back(0.0f);

	drawOurEBO(verts,inds,tex,11, usetex);
}

void coordsbyid(int w, int id, int size, int &x, int &y, bool hardway)
{
	if (!hardway)
	{
		y = trunc(id / w);
		x = id - w * y;
	}
	
	if (hardway)
	{
		int i = -1;
		int cx = -1; int cy = 0;
		int cnt = 0;
		while (cnt < id)
		{
			cnt++;
			cx++;
			if (cx > w) {
				cx = 0; cy++;
			}
		}
		x = cx; y = cy;
	}
}

float linterp(float x0, float x1, float y0, float y1, float x)
{
	if ((x1 != x0) && (y1!=y0))
		return y0 + (x - x0)*((y1 - y0) / (x1 - x0));
	else
		return y0;
}


void calcColour(float val, float min, float max, glm::vec3 &colour)
{
	float prop = (val - min) / (max - min);

	clampVal(prop,0.0f,1.0f);

	float dc = 1/(float)colourtable.size();

	float cdc = -dc;
	int cid = -1;

	while (cdc < prop)
	{
		cdc += dc;
		if (cdc < prop)	cid++;
	}

	if (cid >= colourtable.size()-1) cid = colourtable.size() - 2;
	if (cid <= 0) cid = 0;

	float p1 = 0.0f;
	float p2 = 0.0f;
	glm::vec3 c1 = glm::vec3(1.0f);
	glm::vec3 c2 = glm::vec3(1.0f);

    c1 = colourtable[cid];
    c2 = colourtable[cid + 1];
	p1 = dc * cid;
	p2 = dc * (cid + 1);
	
	colour.r = linterp(p1, p2, c1.r, c2.r, prop);
	colour.g = linterp(p1, p2, c1.g, c2.g, prop);
	colour.b = linterp(p1, p2, c1.b, c2.b, prop);
}


void drawSurface(Shader * shad, vector<glm::vec3> surf_points)
{
	shad->use();

	glm::vec3 colour = glm::vec3(1.0f,0.0f,0.0f);

	if (surfVerts.size() == 0) surfVertNeedUpdate = true;

	if (surfVertNeedUpdate)
	{
		surfVertNeedUpdate = false;

		vector<glm::vec3> norms = {};

		int l = surf_points.size();
		int s = sqrtf((float)l);

		for (int i = 0; i < surf_points.size(); i++)
		{
			int ci = 0; int cj = 0;
			coordsbyid(s, i, l, ci, cj, false);

			//cout << "Checking coords: " << ci << ";" << cj << endl;

			if ((ci < s - 1) && (cj < s - 1))
			{
				if ((i + s + 1) < l)
				{
					glm::vec3 p1 = surf_points[i];
					glm::vec3 p2 = surf_points[i + 1];
					glm::vec3 p3 = surf_points[i + s];
					glm::vec3 p4 = surf_points[i + s + 1];

					glm::vec3 n1 = glm::vec3(1.0);
					glm::vec3 n2 = glm::vec3(1.0);

					glm::vec3 u = p2 - p1;
					glm::vec3 v = p3 - p2;

					n1.x = u.y*v.z - u.z*v.y;
					n1.y = u.z*v.x - u.x*v.z;
					n1.z = u.x*v.y - u.y*v.x;

					u = p4 - p3;
					v = p4 - p1;

					n2.x = u.y*v.z - u.z*v.y;
					n2.y = u.z*v.x - u.x*v.z;
					n2.z = u.x*v.y - u.y*v.x;

					glm::vec3 n = (n1 + n2) * glm::vec3(0.5f);
					norms.push_back(n);
				}
				else norms.push_back(norms[norms.size() - 1]);
			}
			else
			{
				norms.push_back(norms[norms.size() - 1]);
			}

			if (i % 100 == 0) cout << "Processing verts: " << i << " of " << surf_points.size() << endl;

		}

		flarr verts = {};
		intarr inds = {};

		for (int i = 0; i < surf_points.size(); i++)
		{
			//cout << "Making verts " << i << endl;
			verts.push_back(surf_points[i].x);
			verts.push_back(surf_points[i].y);
			verts.push_back(surf_points[i].z);

			calcColour(surf_points[i].z, -10, 10, colour);

			verts.push_back(colour.r);
			verts.push_back(colour.g);
			verts.push_back(colour.b);
			verts.push_back(norms[i].x);
			verts.push_back(norms[i].y);
			verts.push_back(norms[i].z);
			verts.push_back(0.0f);
			verts.push_back(0.0f);

			if (i % 100 == 0) cout << "Building verts: " << i << " of " << surf_points.size() << endl;
		}

		int counter = 0;
		for (int j = 0; j < s; j++)
			for (int i = 0; i < s; i++)
			{
				if ((i < s - 1) && (j < s - 1))
				{
					inds.push_back(counter);
					inds.push_back(counter + 1);
					inds.push_back(counter + s + 1);
					inds.push_back(counter);
					inds.push_back(counter + s);
					inds.push_back(counter + s + 1);
				}
				counter++;
				if (i % 100 == 0) cout << "Building inds: " << counter << " of " << surf_points.size() << endl;
			}

		if (!fileWrote)
		{
			fileWrote = true;
			std::ofstream resultfile("vertex_gen.txt");
			for (int i = 0; i < verts.size() - 1; i += 11)
			{
				resultfile << verts[i] << " " << verts[i + 1] << " " << verts[i + 2] << " " << verts[i + 3] << " " << verts[i + 4] << " " << verts[i + 5] << " " << verts[i + 6] << " " << verts[i + 7] << " " << verts[i + 8] << " " << verts[i + 9] << " " << verts[i + 10] << endl;
				if (i % 100 == 0) cout << "Writing verts: " << i << " of " << verts.size() << endl;
			}

			for (int i = 0; i < inds.size() - 1; i += 3)
			{
				resultfile << inds[i] << "->" << inds[i + 1] << "->" << inds[i + 2] << endl;
				if (i % 100 == 0) cout << "Writing inds: " << i << " of " << inds.size() << endl;
			}
			resultfile.close();
		}
		surfInds = inds;
		surfVerts = verts;
	}
	drawOurEBO(surfVerts, surfInds, 0, 11, false);
}

void clampVal(float &val, const float min, const float max)
{
	if (val < min) val = min;
	if (val > max) val = max;
}