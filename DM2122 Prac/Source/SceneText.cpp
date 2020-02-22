﻿
#include "SceneText.h"
#include "GL\glew.h"
#include "Application.h"
#include <Mtx44.h>
#include "shader.hpp"
#include "MeshBuilder.h"
#include "Utility.h"
#include "LoadTGA.h"

using namespace std;

#define ROT_LIMIT 45.f;
#define SCALE_LIMIT 5.f;
#define LSPEED 10.f




SceneText::SceneText()
{
	
	for (int i = 0; i < NUM_GEOMETRY; ++i)
	{
		meshList[i] = NULL;
	}
	int currentindex = 0;
	for (auto& p : std::experimental::filesystem::directory_iterator("OBJ"))
	{
		if (p.path().extension() == ".obj")
		{
			for (int i = 0; i < size(objectlist); ++i)
			{
				if (objectlist[i].GetMesh() == NULL)
				{
					string location = p.path().filename().string();
					for (int i = 0; i < location.length(); i++)
					{
						if (location[i] == '.')
						{
							location = location.substr(0, i);
							break;
						}
					}
					objectlist[i].SetMesh(location, p.path().string());
					if (std::experimental::filesystem::exists("Image//" + location + ".tga"))
					{
						objectlist[i].GetMesh()->textureID = LoadTGA(("Image//" + location + ".tga").c_str());
					}
					objectlist[i].GetMesh()->material.kAmbient.Set(0.6f, 0.6f, 0.6f);
					objectlist[i].GetMesh()->material.kDiffuse.Set(0.2f, 0.2f, 0.2f);
					objectlist[i].GetMesh()->material.kSpecular.Set(1.f, 1.f, 1.f);
					objectlist[i].GetMesh()->material.kShininess = 1.f;
					numberofobjects++;
					currentindex++;
					break;
				}

			}

		}
	}
}

SceneText::~SceneText()
{
}
GLint GetUniformLocation(GLuint program​, const std::string& name)
{
	return glGetUniformLocation(program​, name.c_str());
}
void SceneText::Init()
{
	initialized = true;
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Generate a default VAO for now
	glGenVertexArrays(1, &m_vertexArrayID);
	glBindVertexArray(m_vertexArrayID);
	glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	camera.Init(Vector3(0, 12, 10), Vector3(0, 5, 0), Vector3(0, 1, 0));

	Mtx44 projection;
	projection.SetToPerspective(45.f, 4.f / 3.f, 0.1f, 50000.f);
	projectionStack.LoadMatrix(projection);

	m_programID = LoadShaders("Shader//Texture.vertexshader", "Shader//Text.fragmentshader");

	//m_programID = LoadShaders("Shader//Texture.vertexshader", "Shader//Texture.fragmentshader");

	m_parameters[U_MVP] = glGetUniformLocation(m_programID, "MVP");
	m_parameters[U_MODELVIEW] = glGetUniformLocation(m_programID, "MV");
	m_parameters[U_MODELVIEW_INVERSE_TRANSPOSE] = glGetUniformLocation(m_programID, "MV_inverse_transpose");
	m_parameters[U_MATERIAL_AMBIENT] = glGetUniformLocation(m_programID, "material.kAmbient");
	m_parameters[U_MATERIAL_DIFFUSE] = glGetUniformLocation(m_programID, "material.kDiffuse");
	m_parameters[U_MATERIAL_SPECULAR] = glGetUniformLocation(m_programID, "material.kSpecular");
	m_parameters[U_MATERIAL_SHININESS] = glGetUniformLocation(m_programID, "material.kShininess");
	m_parameters[U_LIGHTENABLED] = glGetUniformLocation(m_programID, "lightEnabled");
	m_parameters[U_NUMLIGHTS] = glGetUniformLocation(m_programID, "numLights");
	//For First Light
	for (int i = 0; i < numlights; i++)
	{
		m_parameters[8 + i * 11] = GetUniformLocation(m_programID, "lights[" + to_string(i) + "].position_cameraspace");
		m_parameters[9 + i * 11] = GetUniformLocation(m_programID, "lights[" + to_string(i) + "].color");
		m_parameters[10 + i * 11] = GetUniformLocation(m_programID, "lights[" + to_string(i) + "].power");
		m_parameters[11 + i * 11] = GetUniformLocation(m_programID, "lights[" + to_string(i) + "].kC");
		m_parameters[12 + i * 11] = GetUniformLocation(m_programID, "lights[" + to_string(i) + "].kL");
		m_parameters[13 + i * 11] = GetUniformLocation(m_programID, "lights[" + to_string(i) + "].kQ");
		m_parameters[14 + i * 11] = GetUniformLocation(m_programID, "lights[" + to_string(i) + "].type");
		m_parameters[15 + i * 11] = GetUniformLocation(m_programID, "lights[" + to_string(i) + "].spotDirection");
		m_parameters[16 + i * 11] = GetUniformLocation(m_programID, "lights[" + to_string(i) + "].cosCutoff");
		m_parameters[17 + i * 11] = GetUniformLocation(m_programID, "lights[" + to_string(i) + "].cosInner");
		m_parameters[18 + i * 11] = GetUniformLocation(m_programID, "lights[" + to_string(i) + "].exponent");
	}
	//For Second Light

	m_parameters[U_TEXT_ENABLED] = glGetUniformLocation(m_programID, "textEnabled");
	m_parameters[U_TEXT_COLOR] = glGetUniformLocation(m_programID, "textColor");

	//Get a handle for our "colorTexture" uniform
	m_parameters[U_COLOR_TEXTURE_ENABLED] = glGetUniformLocation(m_programID, "colorTextureEnabled");
	m_parameters[U_COLOR_TEXTURE] = glGetUniformLocation(m_programID, "colorTexture");

	glUseProgram(m_programID);
	// Enable depth test
	glEnable(GL_DEPTH_TEST);

	//Initialize First light
	for (int i = 0; i < numlights; i++)
	{
		light[i].type = Light::LIGHT_SPOT;
		light[i].position.Set(0, 5, 0);
		light[i].color.Set(0.5f, 0.5f, 0.5f);
		light[i].power = 100;
		light[i].kC = 1.f;
		light[i].kL = 0.01f;
		light[i].kQ = 0.001f;
		light[i].cosCutoff = cos(Math::DegreeToRadian(45));
		light[i].cosInner = cos(Math::DegreeToRadian(30));
		light[i].exponent = 3.f;
		light[i].spotDirection.Set(0.f, 1.f, 0.f);
		glUniform3fv(m_parameters[9 + i * 11], 1, &light[i].color.r);
		glUniform1f(m_parameters[10 + i * 11], light[i].power);
		glUniform1f(m_parameters[11 + i * 11], light[i].kC);
		glUniform1f(m_parameters[12 + i * 11], light[i].kL);
		glUniform1f(m_parameters[13 + i * 11], light[i].kQ);
		glUniform1i(m_parameters[14 + i * 11], light[i].type);
		glUniform3fv(m_parameters[15 + i * 11], 1, &light[i].spotDirection.x);
		glUniform1f(m_parameters[16 + i * 11], light[i].cosCutoff);
		glUniform1f(m_parameters[17 + i * 11], light[i].cosInner);
		glUniform1f(m_parameters[18 + i * 11], light[i].exponent);
	}
	glUniform1i(m_parameters[U_NUMLIGHTS], numlights);

	//skybox outdoor
	meshList[GEO_LEFT] = MeshBuilder::GenerateQuad("left", Color(1, 1, 1), 1.f, 1.f);
	meshList[GEO_LEFT]->textureID = LoadTGA("Image//hills_lf.tga");

	meshList[GEO_RIGHT] = MeshBuilder::GenerateQuad("right", Color(1, 1, 1), 1.f, 1.f);
	meshList[GEO_RIGHT]->textureID = LoadTGA("Image//hills_rt.tga");

	meshList[GEO_TOP] = MeshBuilder::GenerateQuad("top", Color(1, 1, 1), 1.f, 1.f);
	meshList[GEO_TOP]->textureID = LoadTGA("Image//hills_up.tga");

	meshList[GEO_BOTTOM] = MeshBuilder::GenerateQuad("grass", Color(0, 104.f / 255.f, 0), 5000.f, 5000.f);
	meshList[GEO_BOTTOM]->material.kAmbient.Set(0.6f, 0.6f, 0.6f);
	meshList[GEO_BOTTOM]->material.kDiffuse.Set(0.2f, 0.2f, 0.2f);
	meshList[GEO_BOTTOM]->material.kSpecular.Set(1.f, 1.f, 1.f);
	meshList[GEO_BOTTOM]->material.kShininess = 1.f;


	meshList[GEO_FRONT] = MeshBuilder::GenerateQuad("front", Color(1, 1, 1), 1.f, 1.f);
	meshList[GEO_FRONT]->textureID = LoadTGA("Image//hills_ft.tga");

	meshList[GEO_BACK] = MeshBuilder::GenerateQuad("back", Color(1, 1, 1), 1.f, 1.f);
	meshList[GEO_BACK]->textureID = LoadTGA("Image//hills_bk.tga");

	//Motorshow
	meshList[GEO_MOTORSHOW_WALL] = MeshBuilder::GenerateQuad("wall", Color(1, 1, 1), 1.f, 1.f);
	meshList[GEO_MOTORSHOW_WALL]->textureID = LoadTGA("Image//wall_texture.tga");

	meshList[GEO_MOTORSHOW_CEILING] = MeshBuilder::GenerateQuad("ceiling", Color(0, 0, 0), 1.f, 1.f);
	meshList[GEO_MOTORSHOW_CEILING]->textureID = LoadTGA("Image//ceilingtexture.tga");

	meshList[GEO_FLATLAND] = MeshBuilder::GenerateQuad("flatland", Color(1, 1, 1), 2000.f, 2000.f);
	meshList[GEO_FLATLAND]->textureID = LoadTGA("Image//floortexture.tga");
	meshList[GEO_FLATLAND]->material.kAmbient.Set(0.6f, 0.6f, 0.6f);
	meshList[GEO_FLATLAND]->material.kDiffuse.Set(0.2f, 0.2f, 0.2f);
	meshList[GEO_FLATLAND]->material.kSpecular.Set(1.f, 1.f, 1.f);
	meshList[GEO_FLATLAND]->material.kShininess = 1.f;

	//texts
	meshList[GEO_TEXT] = MeshBuilder::GenerateText("text", 16, 16);
	meshList[GEO_TEXT]->textureID = LoadTGA("Image//calibri.tga");

	//renders crosshair in the middle of screen
	meshList[GEO_CROSSHAIR] = MeshBuilder::GenerateOBJ("crosshair", "OBJ//crosshair.obj");
	//meshList[GEO_CROSSHAIR]->textureID = LoadTGA("Image//peashooter.tga");


	meshList[GEO_LIGHTSPHERE] = MeshBuilder::GenerateSphere("lightBall", Color(1.f, 1.f, 1.f), 9, 36, 1.f);

	vector<Mesh*> MeshStorage;

	for (auto& q : std::experimental::filesystem::directory_iterator("OBJ//NPC"))
	{
		string location = q.path().filename().string();
		for (auto& p : std::experimental::filesystem::directory_iterator(q))
		{
			Mesh* x = NULL;
			x = MeshBuilder::GenerateOBJ(location, p.path().string());
			x->textureID = LoadTGA(("Image//People Textures//" + location + ".tga").c_str());
			x->material.kAmbient.Set(0.3f, 0.3f, 0.3f);
			x->material.kDiffuse.Set(0.1f, 0.1f, 0.1f);
			x->material.kSpecular.Set(.5f, .5f, .5f);
			x->material.kShininess = 1.f;
			MeshStorage.push_back(x);
		}

	}
	for (int i = 0; i < numberofNPCs; i++)
	{
		int x = rand() % 10000;
		int y = rand() % 10000;

		NPCs[i] = new NPC(x * y);
		for (int k = 0; k < 6; k++)
		{
			NPCs[i]->SetMesh(MeshStorage[NPCs[i]->Gettype() * 6 + k], k);
		}


	}
	srand(time(NULL));

	OutsideMotorShow = false;//set time to day

	for (int i = 0; i < numberofobjects; i++)
	{
		if (objectlist[i].GetMesh()->name == "Platformbottom")
		{
			objectlist[i].SetNumberOfOccurences(cars.GetnumberofCars());
			//cout << objectlist[i].GetMeshList()[0]->collisionboxcreated;
		}
		if (objectlist[i].GetMesh()->name == "Platformtop")
		{
			objectlist[i].SetNumberOfOccurences(cars.GetnumberofCars());
		}
		if (objectlist[i].GetMesh()->name == "LightFrame")
		{
			Vector3 initiallightpos = Vector3(-bordersize + (bordersize / sqrt(numlights)), bordersize - 20, -bordersize + (bordersize / sqrt(numlights)));
			Vector3 finallightpos = Vector3(bordersize - (bordersize / sqrt(numlights)), bordersize - 20, bordersize - (bordersize / sqrt(numlights)));
			Vector3 currentlightpos = initiallightpos;
			objectlist[i].SetNumberOfOccurences(numlights);
			for (int j = 0; j < sqrt(numlights); j++)
			{
				currentlightpos.x = initiallightpos.x;
				for (int k = 0; k < sqrt(numlights); k++)
				{
					int currentlight = j * sqrt(numlights) + k;
					objectlist[i].SetPosition(currentlight, currentlightpos);
					light[currentlight].position.Set(currentlightpos.x, currentlightpos.y-10, currentlightpos.z);
					float differenceX = (finallightpos.x - initiallightpos.x) / (sqrt(numlights) - 1);
					currentlightpos.x += differenceX;
				}
				float differenceZ = (finallightpos.z - initiallightpos.z) / (sqrt(numlights) - 1);
				currentlightpos.z += differenceZ;
			}
		}



	}
}


void SceneText::Update(double dt)
{
	if (Application::IsKeyPressed(0x31))
	{
		glDisable(GL_CULL_FACE);
	}
	else if (Application::IsKeyPressed(0x32))
	{
		glEnable(GL_CULL_FACE);
	}
	else if (Application::IsKeyPressed(0x33))
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else if (Application::IsKeyPressed(0x34))
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (!OutsideMotorShow)//if night
	{
		//light[0].position.x = 0;//set light to above tile selector
		//light[0].position.y = 5;
		//light[0].position.z = 0;
		//light[0].power = 1;
		//light[1].power = 0.5f;
		//light[1].color.Set(0, 0, 0.5f);//make light slightly blue
		//glUniform3fv(m_parameters[U_LIGHT1_COLOR], 1, &light[1].color.r);
		//glUniform1f(m_parameters[U_LIGHT1_POWER], light[1].power);
		//glUniform1f(m_parameters[U_LIGHT0_POWER], light[0].power);
	}
	else//if day
	{
		/*light[0].power = 0;
		light[1].power = 1.5f;
		light[1].color.Set(0.5f, 0.5f, 0.5f);
		glUniform3fv(m_parameters[U_LIGHT1_COLOR], 1, &light[1].color.r);
		glUniform1f(m_parameters[U_LIGHT1_POWER], light[1].power);
		glUniform1f(m_parameters[U_LIGHT0_POWER], light[0].power);*/
	}



	float xoffset = -14.5f;//x offset of top left hand corner
	float zoffset = -24;//z offset of top left hand corner
	Vector3 Target2 = camera.position;
	int maxdistance = 1000;
	for (int i = 0; i < maxdistance; i++)
	{
		Target2 += camera.view;//raycast to 20 times player direction
		if (Target2.y <= 0)
		{
			break;
		}
	}


	starepoint = Target2;
	float speed = 2;
	if (Application::IsKeyPressed('Z') && TimeChangeDelay < GetTickCount64())//changes night and day
	{
		OutsideMotorShow = !OutsideMotorShow;//day is not day
		TimeChangeDelay = GetTickCount64() + 250;//delay so day wont be spammed
	}
	if (Application::IsKeyPressed('W'))
	{

		camera.position = camera.position + camera.view * speed;
		camera.position.y = camera.playerheight;
		CheckSquareCollision();
		camera.target = camera.position + camera.view;

	}
	if (Application::IsKeyPressed('S'))
	{

		camera.position = camera.position - camera.view * speed;
		camera.position.y = camera.playerheight;
		CheckSquareCollision();
		camera.target = camera.position + camera.view;

	}
	if (Application::IsKeyPressed('A'))
	{
		camera.position = camera.position - camera.right * speed;
		CheckSquareCollision();
		camera.target = camera.position + camera.view;
	}
	if (Application::IsKeyPressed('D'))
	{
		camera.position = camera.position + camera.right * speed;
		CheckSquareCollision();
		camera.target = camera.position + camera.view;
	}
	CheckSquareCollision();
	camera.Update(dt);
	for (int i = 0; i < numberofNPCs; i++)
	{
		NPCs[i]->move();
	}
	CCar* current = cars.GetStart();
	for (int i = 0; i < cars.GetnumberofCars(); i++)
	{
		cars.GetCar(i)->Spin();

	}


}

void SceneText::Render()
{

	//Clear color & depth buffer every frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	viewStack.LoadIdentity();
	viewStack.LookAt(camera.position.x, camera.position.y, camera.position.z, camera.target.x, camera.target.y, camera.target.z, camera.up.x, camera.up.y, camera.up.z);
	modelStack.LoadIdentity();

	// passing the light direction if it is a direction light	
	for (size_t i = 0; i < numlights; i++)
	{
		if (light[i].type == Light::LIGHT_DIRECTIONAL)
		{
			Vector3 lightDir(light[i].position.x, light[i].position.y, light[i].position.z);
			Vector3 lightDirection_cameraspace = viewStack.Top() * lightDir;
			glUniform3fv(m_parameters[8 + i * 11], 1, &lightDirection_cameraspace.x);
		}
		// if it is spot light, pass in position and direction 
		else if (light[i].type == Light::LIGHT_SPOT)
		{
			Position lightPosition_cameraspace = viewStack.Top() * light[i].position;
			glUniform3fv(m_parameters[8 + i * 11], 1, &lightPosition_cameraspace.x);
			Vector3 spotDirection_cameraspace = viewStack.Top() * light[i].spotDirection;
			glUniform3fv(m_parameters[15 + i * 11], 1, &spotDirection_cameraspace.x);
		}
		else if (light[i].type == Light::LIGHT_POINT)
		{
			// default is point light (only position since point light is 360 degrees)
			Position lightPosition_cameraspace = viewStack.Top() * light[i].position;
			glUniform3fv(m_parameters[8 + i * 11], 1, &lightPosition_cameraspace.x);
		}
	}



	RenderSkybox();

	modelStack.PushMatrix();
	modelStack.Translate(starepoint.x, starepoint.y, starepoint.z);
	RenderMesh(meshList[GEO_LIGHTSPHERE], false, false);
	modelStack.PopMatrix();

	if (!OutsideMotorShow)//Renders Motorshow stuff
	{
		for (int i = 0; i < numberofNPCs; i++)
		{
			modelStack.PushMatrix();
			modelStack.Translate(NPCs[i]->GetPosition().x, 10, NPCs[i]->GetPosition().z);
			modelStack.Rotate(NPCs[i]->getNPCRotation(), 0.f, 1.f, 0.f);
			RenderMesh(NPCs[i]->GetMesh(0), true, false);

			for (int k = 1; k < 6; k++)
			{
				modelStack.PushMatrix();
				if (k == 1 && NPCs[i]->chat(camera.position))
				{
					modelStack.PushMatrix();
					Vector3 target = Vector3(camera.position.x - NPCs[i]->GetPosition().x, 0, camera.position.z - NPCs[i]->GetPosition().z).Normalized();
					float chatbubblerotation = atan2(target.x, target.z) * 180 / Math::PI;
					modelStack.Rotate(-NPCs[i]->getNPCRotation() + chatbubblerotation, 0, 1, 0);
					modelStack.Translate(2, 0, 0);

					for (int i = 0; i < numberofobjects; i++)
					{
						if (objectlist[i].GetMesh()->name == "chatbubble")
						{
							RenderMesh(objectlist[i].GetMesh(), false, false);
						}
					}
					modelStack.PushMatrix();
					modelStack.Scale(0.32f, 0.32f, 1);
					modelStack.Translate(2.75f, 15, 0.2f);
					RenderText(meshList[GEO_TEXT], "U are a nice guy", Color(0, 0, 0));
					modelStack.PopMatrix();
					modelStack.PopMatrix();
				}
				if (k == 2 || k == 5)
				{
					if (k == 5)
					{
						modelStack.Translate(0, -4.5f, 0);
					}

					if (NPCs[i]->GetIsMoving())
					{
						modelStack.Rotate(sin(GetTickCount64() / 100) * 50, 1, 0, 0);
					}
				}
				if (k == 3 || k == 4)
				{
					if (k == 3)
					{
						modelStack.Translate(0, -4.5f, 0);
					}

					if (NPCs[i]->GetIsMoving())
					{
						modelStack.Rotate(-sin(GetTickCount64() / 100) * 50, 1, 0, 0);
					}
				}
				RenderMesh(NPCs[i]->GetMesh(k), true, false);
				modelStack.PopMatrix();
			}
			modelStack.PopMatrix();
		}
		if (cars.GetnumberofCars() > 0)
		{
			CCar* current = cars.GetStart();
			for (int i = 0; i < cars.GetnumberofCars(); i++)
			{
				for (int k = 0; k < numberofobjects; k++)
				{
					if (objectlist[k].GetMesh()->name == "Platformbottom")
					{
						modelStack.PushMatrix();
						modelStack.Scale(4, 4, 4);
						modelStack.Translate(cars.GetCar(i)->GetPostition()[0].x, 0, cars.GetCar(i)->GetPostition()[0].z);

						RenderMesh(objectlist[k].GetMeshList()[i], true, true);

					}
					if (objectlist[k].GetMesh()->name == "Platformtop")
					{
						modelStack.PushMatrix();
						modelStack.Rotate(cars.GetCar(i)->GetRotation()[0].y, 0.f, 1.f, 0.f);

						RenderMesh(objectlist[k].GetMesh(), true, false);
					}
				}
				modelStack.PushMatrix();
				modelStack.Scale(0.5, 0.5, 0.5);
				modelStack.Translate(0, 4.5f, 0);
				RenderMesh(cars.GetCar(i)->GetMesh(), true, false);
				modelStack.PopMatrix();
				modelStack.PopMatrix();
				modelStack.PopMatrix();
			}
		}

		for (int i = 0; i < numberofobjects; i++)
		{
			if (objectlist[i].GetMesh()->name == "LightFrame")
			{
				for (int k = 0; k < objectlist[i].GetNumberOfOccurences(); k++)
				{
					modelStack.PushMatrix();
					modelStack.Translate(objectlist[i].GetPostition()[k].x, objectlist[i].GetPostition()[k].y, objectlist[i].GetPostition()[k].z);
					//cout << objectlist[i].GetPostition()[k] << endl;
					RenderMesh(objectlist[i].GetMesh(), false, false);
					modelStack.PopMatrix();
				}
			}

		}
	}
	for (int i = 0; i < numlights; i++)
	{
		modelStack.PushMatrix();

		modelStack.Translate(light[i].position.x, light[i].position.y, light[i].position.z);
		modelStack.Scale(5, 5, 5);
		RenderMesh(meshList[GEO_LIGHTSPHERE], false, false);
		modelStack.PopMatrix();
	}
	RenderMeshOnScreen(meshList[GEO_CROSSHAIR], 40, 30, 2, 2);//render crosshair
	RenderFramerate(meshList[GEO_TEXT], Color(0, 0, 0), 3, 21, 19);
	//RenderTextOnScreen(meshList[GEO_TEXT], (":" + std::to_string(plantlist.sun)), Color(0, 0, 0), 5, 2, 10.5f);//render amount of sun in inventory
}

void SceneText::Exit()
{
	// Cleanup here
	for (int i = 0; i < NUM_GEOMETRY; ++i)
	{
		if (meshList[i] != NULL)
			delete meshList[i];
	}
	// Cleanup VBO here
	glDeleteVertexArrays(1, &m_vertexArrayID);
	glDeleteProgram(m_programID);

}
void SceneText::CheckSquareCollision()
{

	//		//Vector3 A = Vector3(xmin, ymin, zmax);
	//		//Vector3 B = Vector3(xmax, ymin, zmax);
	//		//Vector3 C = Vector3(xmax, ymin, zmin);
	//		//Vector3 D = Vector3(xmin, ymin, zmin);
	//		//Vector3 E = camera.position;

	//		//float Area1 = ((E.x * B.z - B.x * E.z) - (E.x * A.z - A.x * E.z) + (B.x * A.z - A.x * B.z));
	//		//float Area2 = ((E.x * B.z - B.x * E.z) - (E.x * C.z - C.x * E.z) + (B.x * C.z - C.x * B.z));
	//		//float Area3 = ((E.x * D.z - D.x * E.z) - (E.x * C.z - C.x * E.z) + (D.x * C.z - C.x * D.z));
	//		//float Area4 = ((E.x * D.z - D.x * E.z) - (E.x * A.z - A.x * E.z) + (D.x * A.z - A.x * D.z));
	//		//cout << Area1 << " " << Area2 << " " << Area3 << " " << Area4 << endl;



	for (int current = 0; current < size(objectlist); current++)
	{
		for (int i = 0; i < objectlist[current].GetNumberOfOccurences(); i++)
		{
			if (objectlist[current].GetType() == "Object")
			{
				if (objectlist[current].GetMeshList()[i]->collison)
				{
					float xmin = objectlist[current].GetMeshList()[i]->ColisionVector2.x;
					float xmax = objectlist[current].GetMeshList()[i]->ColisionVector1.x;
					float ymin = objectlist[current].GetMeshList()[i]->ColisionVector2.y;
					float ymax = objectlist[current].GetMeshList()[i]->ColisionVector1.y;
					float zmin = objectlist[current].GetMeshList()[i]->ColisionVector2.z;
					float zmax = objectlist[current].GetMeshList()[i]->ColisionVector1.z;


					if (camera.position.x <= xmax && camera.position.z <= zmax && camera.position.z >= zmin && abs(camera.position.x - xmax) <= 2)
					{
						camera.position.x = xmax + 0.1f;
					}
					if (camera.position.x >= xmin && camera.position.z <= zmax && camera.position.z >= zmin && abs(camera.position.x - xmin) <= 2)
					{
						camera.position.x = xmin - 0.1f;
					}
					if (camera.position.z <= zmax && camera.position.x <= xmax && camera.position.x >= xmin && abs(camera.position.z - zmax) <= 2)
					{
						camera.position.z = zmax + 0.1f;
					}
					if (camera.position.z >= zmin && camera.position.x <= xmax && camera.position.x >= xmin && abs(camera.position.z - zmin) <= 2)
					{
						camera.position.z = zmin - 0.1f;
					}

					for (int i = 0; i < numberofNPCs; i++)
					{
						if (NPCs[i]->GetPosition().x <= xmax && NPCs[i]->GetPosition().z <= zmax && NPCs[i]->GetPosition().z >= zmin && abs(NPCs[i]->GetPosition().x - xmax) <= 2)
						{
							NPCs[i]->SetPosition(Vector3(xmax + 0.5f, NPCs[i]->GetPosition().y, NPCs[i]->GetPosition().z));
						}
						if (NPCs[i]->GetPosition().x >= xmin && NPCs[i]->GetPosition().z <= zmax && NPCs[i]->GetPosition().z >= zmin && abs(NPCs[i]->GetPosition().x - xmin) <= 2)
						{
							NPCs[i]->SetPosition(Vector3(xmin - 0.5f, NPCs[i]->GetPosition().y, NPCs[i]->GetPosition().z));
						}
						if (NPCs[i]->GetPosition().z <= zmax && NPCs[i]->GetPosition().x <= xmax && NPCs[i]->GetPosition().x >= xmin && abs(NPCs[i]->GetPosition().z - zmax) <= 2)
						{
							NPCs[i]->SetPosition(Vector3(NPCs[i]->GetPosition().x, NPCs[i]->GetPosition().y, zmax + 0.5f));
						}
						if (NPCs[i]->GetPosition().z >= zmin && NPCs[i]->GetPosition().x <= xmax && NPCs[i]->GetPosition().x >= xmin && abs(NPCs[i]->GetPosition().z - zmin) <= 2)
						{
							NPCs[i]->SetPosition(Vector3(NPCs[i]->GetPosition().x, NPCs[i]->GetPosition().y, zmin - 0.5f));
						}
					}
				}
			}
		}
	}
	/*for (int current = 0; current < numberofNPCs; current++)
	{
		if (NPCs[current]->GetMesh(0)->collison)
		{
			float xmin = NPCs[current]->GetMesh(0)->ColisionVector2.x;
			float xmax = NPCs[current]->GetMesh(0)->ColisionVector1.x;
			float ymin = NPCs[current]->GetMesh(0)->ColisionVector2.y;
			float ymax = NPCs[current]->GetMesh(0)->ColisionVector1.y;
			float zmin = NPCs[current]->GetMesh(0)->ColisionVector2.z;
			float zmax = NPCs[current]->GetMesh(0)->ColisionVector1.z;


			if (camera.position.x <= xmax && camera.position.z <= zmax && camera.position.z >= zmin && abs(camera.position.x - xmax) <= 2)
			{
				camera.position.x = xmax + 0.1f;
			}
			if (camera.position.x >= xmin && camera.position.z <= zmax && camera.position.z >= zmin && abs(camera.position.x - xmin) <= 2)
			{
				camera.position.x = xmin - 0.1f;
			}
			if (camera.position.z <= zmax && camera.position.x <= xmax && camera.position.x >= xmin && abs(camera.position.z - zmax) <= 2)
			{
				camera.position.z = zmax + 0.1f;
			}
			if (camera.position.z >= zmin && camera.position.x <= xmax && camera.position.x >= xmin && abs(camera.position.z - zmin) <= 2)
			{
				camera.position.z = zmin - 0.1f;
			}
		}


	}*/

}

void SceneText::RenderMesh(Mesh* mesh, bool enableLight, bool hasCollision)
{

	if (hasCollision)
	{
		if (!mesh->collisionboxcreated)
		{
			mesh->ColisionVector1 = modelStack.Top().GetTranspose().Multiply(mesh->ColisionVector1);
			mesh->ColisionVector2 = modelStack.Top().GetTranspose().Multiply(mesh->ColisionVector2);
			mesh->collison = true;
			mesh->collisionboxcreated = true;
		}

		/*Mesh* Collider = MeshBuilder::GenerateCollisonBox("COLLISIONBOX", mesh->p1, mesh->p2, mesh->p3, mesh->p4, mesh->p5, mesh->p6, mesh->p7, mesh->p8);
		modelStack.PushMatrix();
		RenderMesh(Collider, false, false);
		modelStack.PopMatrix();*/

	}
	Mtx44 MVP, modelView, modelView_inverse_transpose;

	MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top();
	glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, &MVP.a[0]);

	modelView = viewStack.Top() * modelStack.Top();
	glUniformMatrix4fv(m_parameters[U_MODELVIEW], 1, GL_FALSE, &modelView.a[0]);


	if (enableLight)
	{
		glUniform1i(m_parameters[U_LIGHTENABLED], 1);
		modelView_inverse_transpose = modelView.GetInverse().GetTranspose();
		glUniformMatrix4fv(m_parameters[U_MODELVIEW_INVERSE_TRANSPOSE], 1, GL_FALSE, &modelView_inverse_transpose.a[0]);

		//load material
		glUniform3fv(m_parameters[U_MATERIAL_AMBIENT], 1, &mesh->material.kAmbient.r);
		glUniform3fv(m_parameters[U_MATERIAL_DIFFUSE], 1, &mesh->material.kDiffuse.r);
		glUniform3fv(m_parameters[U_MATERIAL_SPECULAR], 1, &mesh->material.kSpecular.r);
		glUniform1f(m_parameters[U_MATERIAL_SHININESS], mesh->material.kShininess);
	}
	else
	{
		glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	}

	if (mesh->textureID > 0) {
		glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mesh->textureID);
		glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);
	}
	else {
		glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 0);
	}
	mesh->Render(); //this line should only be called once in the whole function

	if (mesh->textureID > 0) glBindTexture(GL_TEXTURE_2D, 0);
}

void SceneText::RenderSkybox()
{
	float size = bordersize;//uniform scaling
	float offset = size / 200;//used to prevent lines appearing
	if (OutsideMotorShow)//render daytime skybox
	{
		modelStack.PushMatrix();
		///scale, translate, rotate
		modelStack.Translate(-size + offset, 0.f, 0.f);
		modelStack.Scale(size * 2, size * 2, size * 2);
		modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
		RenderMesh(meshList[GEO_LEFT], false, false);
		modelStack.PopMatrix();
		modelStack.PushMatrix();
		///scale, translate, rotate 
		modelStack.Translate(size - offset, 0.f, 0.f);
		modelStack.Scale(size * 2, size * 2, size * 2);
		modelStack.Rotate(-90.f, 0.f, 1.f, 0.f);
		RenderMesh(meshList[GEO_RIGHT], false, false);
		modelStack.PopMatrix();
		modelStack.PushMatrix();
		///scale, translate, rotate
		modelStack.Translate(0.f, size - offset, 0.f);
		modelStack.Scale(size * 2, size * 2, size * 2);
		modelStack.Rotate(-90.f, 0.f, 1.f, 0.f);
		modelStack.Rotate(90.f, 1.f, 0.f, 0.f);
		RenderMesh(meshList[GEO_TOP], false, false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Rotate(-90, 1, 0, 0);
		RenderMesh(meshList[GEO_BOTTOM], true, false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		///scale, translate, rotate
		modelStack.Translate(0.f, 0.f, -size + offset);
		modelStack.Scale(size * 2, size * 2, size * 2);
		RenderMesh(meshList[GEO_FRONT], false, false);
		modelStack.PopMatrix();
		modelStack.PushMatrix();
		///scale, translate, rotate 
		modelStack.Translate(0.f, 0.f, size - offset);
		modelStack.Scale(size * 2, size * 2, size * 2);
		modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
		RenderMesh(meshList[GEO_BACK], false, false);
		modelStack.PopMatrix();
	}
	else//render motorshow skybox
	{
		modelStack.PushMatrix();
		///scale, translate, rotate
		modelStack.Translate(-size + offset, 0.f, 0.f);
		modelStack.Scale(size * 2, size * 2, size * 2);
		modelStack.Rotate(90.f, 0.f, 1.f, 0.f);
		RenderMesh(meshList[GEO_MOTORSHOW_WALL], false, true);
		modelStack.PopMatrix();
		modelStack.PushMatrix();
		///scale, translate, rotate 
		modelStack.Translate(size - offset, 0.f, 0.f);
		modelStack.Scale(size * 2, size * 2, size * 2);
		modelStack.Rotate(-90.f, 0.f, 1.f, 0.f);
		RenderMesh(meshList[GEO_MOTORSHOW_WALL], false, true);
		modelStack.PopMatrix();
		modelStack.PushMatrix();
		///scale, translate, rotate
		modelStack.PushMatrix();
		///scale, translate, rotate
		modelStack.Translate(0.f, 0.f, -size + offset);
		modelStack.Scale(size * 2, size * 2, size * 2);
		RenderMesh(meshList[GEO_MOTORSHOW_WALL], false, true);
		modelStack.PopMatrix();
		modelStack.PushMatrix();
		///scale, translate, rotate 
		modelStack.Translate(0.f, 0.f, size - offset);
		modelStack.Scale(size * 2, size * 2, size * 2);
		modelStack.Rotate(180.f, 0.f, 1.f, 0.f);
		RenderMesh(meshList[GEO_MOTORSHOW_WALL], false, true);
		modelStack.PopMatrix();
		modelStack.Translate(0.f, size - offset, 0.f);
		modelStack.Scale(size * 2.75f, size * 2.75f, size * 2.75f);
		modelStack.Rotate(GetTickCount64() * 0.01f, 0.f, 1.f, 0.f);
		modelStack.Rotate(90.f, 1.f, 0.f, 0.f);
		RenderMesh(meshList[GEO_MOTORSHOW_CEILING], false, true);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Rotate(-90, 1, 0, 0);

		RenderMesh(meshList[GEO_FLATLAND], true, true);
		modelStack.PopMatrix();
	}
}

void SceneText::RenderText(Mesh* mesh, std::string text, Color color)
{
	if (!mesh || mesh->textureID <= 0)
		return;
	glDisable(GL_DEPTH_TEST);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 1);
	glUniform3fv(m_parameters[U_TEXT_COLOR], 1, &color.r);
	glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mesh->textureID);
	glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);
	for (unsigned i = 0; i < text.length(); ++i)
	{
		Mtx44 characterSpacing;
		characterSpacing.SetToTranslation(i * 1.0f, 0, 0);
		Mtx44 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top() * characterSpacing;
		glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, &MVP.a[0]);

		mesh->Render((unsigned)text[i] * 6, 6);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 0);
	glEnable(GL_DEPTH_TEST);
}

void SceneText::RenderTextOnScreen(Mesh* mesh, std::string text, Color color, float size, float x, float y)
{
	if (!mesh || mesh->textureID <= 0)
		return;
	glDisable(GL_DEPTH_TEST);
	Mtx44 ortho;
	ortho.SetToOrtho(0, 80, 0, 60, -10, 10);
	projectionStack.PushMatrix();
	projectionStack.LoadMatrix(ortho);
	viewStack.PushMatrix();
	viewStack.LoadIdentity();
	modelStack.PushMatrix();
	modelStack.LoadIdentity();
	modelStack.Scale(size, size, size);
	modelStack.Translate(x, y, 0);

	glUniform1i(m_parameters[U_TEXT_ENABLED], 1);
	glUniform3fv(m_parameters[U_TEXT_COLOR], 1, &color.r);
	glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mesh->textureID);
	glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);
	for (unsigned i = 0; i < text.length(); ++i)
	{
		Mtx44 characterSpacing;
		characterSpacing.SetToTranslation(i * 1.0f, 0, 0);
		Mtx44 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top() * characterSpacing;
		glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, &MVP.a[0]);

		mesh->Render((unsigned)text[i] * 6, 6);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 0);

	projectionStack.PopMatrix();
	viewStack.PopMatrix();
	modelStack.PopMatrix();

	glEnable(GL_DEPTH_TEST);
}
void SceneText::RenderFramerate(Mesh* mesh, Color color, float size, float x, float y)
{

	static float framesPerSecond = 0.0f;
	static int fps;
	static float lastTime = 0.0f;
	float currentTime = GetTickCount64() * 0.001f;
	++framesPerSecond;

	if (currentTime - lastTime > 1.0f)
	{
		lastTime = currentTime;
		fps = (int)framesPerSecond;
		framesPerSecond = 0;
	}
	std::string frames = "FPS:" + std::to_string(fps);
	if (!mesh || mesh->textureID <= 0)
		return;
	glDisable(GL_DEPTH_TEST);
	Mtx44 ortho;
	ortho.SetToOrtho(0, 80, 0, 60, -10, 10);
	projectionStack.PushMatrix();
	projectionStack.LoadMatrix(ortho);
	viewStack.PushMatrix();
	viewStack.LoadIdentity();
	modelStack.PushMatrix();
	modelStack.LoadIdentity();
	modelStack.Scale(size, size, size);
	modelStack.Translate(x, y, 0);

	glUniform1i(m_parameters[U_TEXT_ENABLED], 1);
	glUniform3fv(m_parameters[U_TEXT_COLOR], 1, &color.r);
	glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mesh->textureID);
	glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);
	for (unsigned i = 0; i < frames.length(); ++i)
	{
		Mtx44 characterSpacing;
		characterSpacing.SetToTranslation(i * 1.0f, 0, 0);
		Mtx44 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top() * characterSpacing;
		glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, &MVP.a[0]);

		mesh->Render((unsigned)frames[i] * 6, 6);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 0);

	projectionStack.PopMatrix();
	viewStack.PopMatrix();
	modelStack.PopMatrix();

	glEnable(GL_DEPTH_TEST);
}

void SceneText::RenderMeshOnScreen(Mesh* mesh, int x, int y, int sizex, int sizey)
{
	glDisable(GL_DEPTH_TEST);
	Mtx44 ortho;
	ortho.SetToOrtho(0, 80, 0, 60, -10, 10);
	projectionStack.PushMatrix();
	projectionStack.LoadMatrix(ortho);
	viewStack.PushMatrix();
	viewStack.LoadIdentity();
	modelStack.PushMatrix();
	modelStack.LoadIdentity();
	modelStack.Translate(x, y, 0);
	modelStack.Scale(sizex, sizey, 1);
	RenderMesh(mesh, false, false);
	modelStack.PopMatrix();
	viewStack.PopMatrix();
	projectionStack.PopMatrix();
}