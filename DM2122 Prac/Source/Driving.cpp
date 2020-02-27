﻿
#include "Driving.h"
#include "GL\glew.h"
#include "Application.h"
#include <Mtx44.h>
#include "shader.hpp"
#include "MeshBuilder.h"
#include "Utility.h"
#include "LoadTGA.h"
#include "Physics.h"
#include "Player.h"

using namespace std;

#define ROT_LIMIT 45.f;
#define SCALE_LIMIT 5.f;
#define LSPEED 10.f




DrivingScene::DrivingScene()
{

	for (int i = 0; i < NUM_GEOMETRY; ++i)
	{
		meshList[i] = NULL;
	}
}

DrivingScene::~DrivingScene()
{
}

void DrivingScene::Init()
{
	initialized = true;
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Generate a default VAO for now
	glGenVertexArrays(1, &m_vertexArrayID);
	glBindVertexArray(m_vertexArrayID);
	glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	camera.Init(Vector3(0, 24, 20), Vector3(0, 5, 0), Vector3(0, 1, 0));

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
		m_parameters[8 + i * 11] = glGetUniformLocation(m_programID, ("lights[" + to_string(i) + "].position_cameraspace").c_str());
		m_parameters[9 + i * 11] = glGetUniformLocation(m_programID, ("lights[" + to_string(i) + "].color").c_str());
		m_parameters[10 + i * 11] = glGetUniformLocation(m_programID, ("lights[" + to_string(i) + "].power").c_str());
		m_parameters[11 + i * 11] = glGetUniformLocation(m_programID, ("lights[" + to_string(i) + "].kC").c_str());
		m_parameters[12 + i * 11] = glGetUniformLocation(m_programID, ("lights[" + to_string(i) + "].kL").c_str());
		m_parameters[13 + i * 11] = glGetUniformLocation(m_programID, ("lights[" + to_string(i) + "].kQ").c_str());
		m_parameters[14 + i * 11] = glGetUniformLocation(m_programID, ("lights[" + to_string(i) + "].type").c_str());
		m_parameters[15 + i * 11] = glGetUniformLocation(m_programID, ("lights[" + to_string(i) + "].spotDirection").c_str());
		m_parameters[16 + i * 11] = glGetUniformLocation(m_programID, ("lights[" + to_string(i) + "].cosCutoff").c_str());
		m_parameters[17 + i * 11] = glGetUniformLocation(m_programID, ("lights[" + to_string(i) + "].cosInner").c_str());
		m_parameters[18 + i * 11] = glGetUniformLocation(m_programID, ("lights[" + to_string(i) + "].exponent").c_str());
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

	//texts
	meshList[GEO_TEXT] = MeshBuilder::GenerateText("text", 16, 16);
	meshList[GEO_TEXT]->textureID = LoadTGA("Image//calibri.tga");

	//renders crosshair in the middle of screen
	meshList[GEO_CROSSHAIR] = MeshBuilder::GenerateOBJ("crosshair", "OBJ//crosshair.obj");



	meshList[GEO_SPEEDOMETERBACK] = MeshBuilder::GenerateQuad("speedometerback", Color(0, 0, 0), 1, 1);
	meshList[GEO_SPEEDOMETERBACK]->textureID = LoadTGA("Image//Car Textures//speedometer_back.tga");
	meshList[GEO_SPEEDOMETERFRONT] = MeshBuilder::GenerateQuad("speedometerfront", Color(0, 0, 0), 1, 1);
	meshList[GEO_SPEEDOMETERFRONT]->textureID = LoadTGA("Image//Car Textures//speedometer_front.tga");
	meshList[GEO_LIGHTSPHERE] = MeshBuilder::GenerateSphere("lightBall", Color(1.f, 1.f, 1.f), 9, 36, 1.f);

	vector<Mesh*> MeshStorage;

	objectlist[0].SetMesh("InnerTrack", "OBJ//Tracks//track_inneredge.obj");
	objectlist[0].GetMesh()->textureID = LoadTGA("Image//track_inneredge.tga");
	objectlist[0].SetNumberOfOccurences(20);
	objectlist[1].SetMesh("OuterTrack", "OBJ//Tracks//track_outeredge.obj");
	objectlist[1].GetMesh()->textureID = LoadTGA("Image//track_outeredge.tga");
	objectlist[1].SetNumberOfOccurences(20);
	/*objectlist[2].SetMesh("OuterTrack", "OBJ//Tracks//track_outeredge.obj");
	objectlist[2].GetMesh()->textureID = LoadTGA("Image//track_outeredge.tga");
	objectlist[2].SetNumberOfOccurences(20);*/

	srand(time(NULL));
	Player::instance()->cars.GetCurrentCar()->SetPosition(0, Vector3(275, 0, 0));
	Player::instance()->cars.GetCurrentCar()->SetRotation(0, Vector3(0, 0, 0));
	camera.mouseenabledVertical = false;
	innerradius = 200;
	outerradius = 300;
}


void DrivingScene::Update(double dt)
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

	

	float speed = 2;
	CCar* currentcar = Player::instance()->cars.GetCurrentCar();
	Mtx44 rotation;
	rotation.SetToRotation(-currentcar->GetRotation()[0].y, 0, 1, 0);
	bool check = CheckSquareCollision();
	Vector3 initialpos = currentcar->GetPostition()[0];
	Vector3 offsetPerFrame = Vector3(0, 0, (currentcar->getcurrentSpeed() / 4));
	offsetPerFrame = rotation.Multiply(offsetPerFrame);
	float RotationSpeed = 2;
	Vector3 futurepos;
	if (!check)
	{
		futurepos = initialpos + offsetPerFrame;
		currentcar->SetPosition(0, futurepos);
	}
	
	
	if (Application::IsKeyPressed('W'))
	{
		if (currentcar->getcurrentSpeed() < currentcar->getmaxSpeed())
		{
			currentcar->setcurrentSpeed(currentcar->getcurrentSpeed() + 0.5f);
		}
		if (Application::IsKeyPressed('A'))
		{
			currentcar->SetRotation(0, currentcar->GetRotation()[0] + Vector3(0, RotationSpeed, 0));
		}
		if (Application::IsKeyPressed('D'))
		{
			currentcar->SetRotation(0, currentcar->GetRotation()[0] + Vector3(0, -RotationSpeed, 0));
		}
	}
	else if (Application::IsKeyPressed('S'))
	{
		if (currentcar->getcurrentSpeed() > -currentcar->getmaxSpeed())
		{
			currentcar->setcurrentSpeed(currentcar->getcurrentSpeed() - 0.5f);
		}
		if (Application::IsKeyPressed('A'))
		{
			currentcar->SetRotation(0, currentcar->GetRotation()[0] + Vector3(0, -RotationSpeed, 0));
		}
		if (Application::IsKeyPressed('D'))
		{
			currentcar->SetRotation(0, currentcar->GetRotation()[0] + Vector3(0, RotationSpeed, 0));
		}

	}
	else
	{
		if (currentcar->getcurrentSpeed() > 0)
		{
			currentcar->setcurrentSpeed(currentcar->getcurrentSpeed() - 0.5f);
			if (currentcar->getcurrentSpeed() < 0)
			{
				currentcar->setcurrentSpeed(0);
			}
			if (Application::IsKeyPressed('A'))
			{
				currentcar->SetRotation(0, currentcar->GetRotation()[0] + Vector3(0, RotationSpeed, 0));
			}
			if (Application::IsKeyPressed('D'))
			{
				currentcar->SetRotation(0, currentcar->GetRotation()[0] + Vector3(0, -RotationSpeed, 0));
			}
		}
		if (currentcar->getcurrentSpeed()< 0)
		{
			currentcar->setcurrentSpeed(currentcar->getcurrentSpeed() + 0.5f);
			if (currentcar->getcurrentSpeed() > 0)
			{
				currentcar->setcurrentSpeed(0);
			}
			if (Application::IsKeyPressed('A'))
			{
				currentcar->SetRotation(0, currentcar->GetRotation()[0] + Vector3(0, -RotationSpeed, 0));
			}
			if (Application::IsKeyPressed('D'))
			{
				currentcar->SetRotation(0, currentcar->GetRotation()[0] + Vector3(0, RotationSpeed, 0));
			}

		}
		offsetPerFrame = currentcar->GetPostition()[0] - initialpos;
		
	}
	if (Application::IsKeyPressed('V'))
	{
		Application::state = Application::Mainmenu;
	}
	camera.offset = currentcar->GetPostition()[0];
	camera.position = Vector3(currentcar->GetPostition()[0].x + 40*sin(Math::DegreeToRadian(camera.Rotationfloat)),10, currentcar->GetPostition()[0].z + 40*cos(Math::DegreeToRadian(camera.Rotationfloat)));
	camera.target = Player::instance()->cars.GetCurrentCar()->GetPostition()[0];
	//cout << Player::instance()->cars.GetCurrentCar()->GetMeshList()[0]->ColisionVector1 << " " << Player::instance()->cars.GetCurrentCar()->GetMeshList()[0]->ColisionVector2 << " " << Player::instance()->cars.GetCurrentCar()->GetMeshList()[0]->ColisionVector3 << " " << Player::instance()->cars.GetCurrentCar()->GetMeshList()[0]->ColisionVector4 << endl;
	camera.Update(dt);

}

void DrivingScene::Render()
{

	//Clear color & depth buffer every frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	viewStack.LoadIdentity();
	viewStack.LookAt(camera.position.x, camera.position.y, camera.position.z, camera.target.x, camera.target.y, camera.target.z, camera.up.x, camera.up.y, camera.up.z);
	modelStack.LoadIdentity();

	// passing the light direction if it is a direction light	
	for (int i = 0; i < numlights; i++)
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

	//modelStack.PushMatrix();
	//modelStack.Translate(starepoint.x, starepoint.y, starepoint.z);
	//RenderMesh(meshList[GEO_LIGHTSPHERE], false, false);
	//modelStack.PopMatrix();



	modelStack.PushMatrix();
	modelStack.Translate(Player::instance()->cars.GetCurrentCar()->GetPostition()[0].x, Player::instance()->cars.GetCurrentCar()->GetPostition()[0].y, Player::instance()->cars.GetCurrentCar()->GetPostition()[0].z);
	modelStack.Rotate(Player::instance()->cars.GetCurrentCar()->GetRotation()[0].y, 0, 1, 0);
	RenderMesh(Player::instance()->cars.GetCurrentCar()->GetMeshList()[0], false, true);
	modelStack.PopMatrix();
		
	
	for (int j = 0; j < objectlist[0].GetNumberOfOccurences(); j++)
	{
		modelStack.PushMatrix();
		modelStack.Rotate((j*(360/objectlist[0].GetNumberOfOccurences())),0, 1, 0);
		modelStack.Translate(0, 0, innerradius);
		RenderMesh(objectlist[0].GetMeshList()[j], false, true);
		modelStack.PopMatrix();
	}
	
	for (int j = 0; j < objectlist[1].GetNumberOfOccurences(); j++)
	{
		modelStack.PushMatrix();
		modelStack.Rotate((j * (360 / objectlist[1].GetNumberOfOccurences())), 0, 1, 0);
		modelStack.Translate(0, 0, outerradius);
		RenderMesh(objectlist[1].GetMeshList()[j], false, true);
		modelStack.PopMatrix();
	}
	
	modelStack.PushMatrix();
	float speedometerRotation = ((abs(Player::instance()->cars.GetCurrentCar()->getcurrentSpeed()/5)*25) / 125.0f) * -270.0f;
	RenderMeshOnScreen(meshList[GEO_SPEEDOMETERBACK], 10, 10, 20, 20, speedometerRotation);//render speedometerback
	modelStack.PopMatrix();
	RenderMeshOnScreen(meshList[GEO_SPEEDOMETERFRONT], 10, 10, 20, 20, 0);//render speedometerfront
	RenderMeshOnScreen(meshList[GEO_CROSSHAIR], 40, 30, 2, 2,0);//render crosshair
	RenderFramerate(meshList[GEO_TEXT], Color(0, 0, 0), 3, 21, 19);
	//RenderTextOnScreen(meshList[GEO_TEXT], (":" + std::to_string(plantlist.sun)), Color(0, 0, 0), 5, 2, 10.5f);//render amount of sun in inventory
}

void DrivingScene::Exit()
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
bool DrivingScene::CheckSquareCollision()
{
	bool collided = false;
	for (int i = 0; i < 2; i++)
	{
		
		for (int j = 0; j < objectlist[i].GetNumberOfOccurences(); j++)
		{
			Mesh* currentmesh = objectlist[i].GetMeshList()[j];

			Vector3 A = currentmesh->ColisionVector1;//front left
			Vector3 B = currentmesh->ColisionVector2;//front right
			Vector3 C = currentmesh->ColisionVector3;//back right
			Vector3 D = currentmesh->ColisionVector4;//back left


			Vector3 A2 = Player::instance()->cars.GetCurrentCar()->GetMeshList()[0]->ColisionVector1;
			Vector3 B2 = Player::instance()->cars.GetCurrentCar()->GetMeshList()[0]->ColisionVector2;
			Vector3 C2 = Player::instance()->cars.GetCurrentCar()->GetMeshList()[0]->ColisionVector3;
			Vector3 D2 = Player::instance()->cars.GetCurrentCar()->GetMeshList()[0]->ColisionVector4;
			/*Mtx44 rotation;
			rotation.SetToRotation((Player::instance()->cars.GetCurrentCar()->GetRotation()[0].y), 0, 1, 0);
			Vector3 A2 = Player::instance()->cars.GetCurrentCar()->GetPostition()[0] + rotation.Multiply(Vector3(2, 0, 0));
			Vector3 B2 = Player::instance()->cars.GetCurrentCar()->GetPostition()[0] + rotation.Multiply(Vector3(0, 0, 2));
			Vector3 C2 = Player::instance()->cars.GetCurrentCar()->GetPostition()[0] - rotation.Multiply(Vector3(2, 0, 0));
			Vector3 D2 = Player::instance()->cars.GetCurrentCar()->GetPostition()[0] - rotation.Multiply(Vector3(0, 0, 2));*/
			Vector3 MidAB = (A + B) * 0.5f;
			Vector3 MidCD = (C + D) * 0.5f;
			Vector3 Center = (MidAB + MidCD) * 0.5f;



			bool x = Physics::CheckCollision(A, B, C, D, A2, B2, C2, D2);
			if (currentmesh->camcollided == false)
			{
				if (x && Player::instance()->cars.GetCurrentCar()->getcurrentSpeed() != 0)
				{
					cout << x;
					currentmesh->camcollided = true;
					bool foundposition = true;
					Vector3 pushback = (Player::instance()->cars.GetCurrentCar()->GetPostition()[0] - Center).Normalized();
					Player::instance()->cars.GetCurrentCar()->setcurrentSpeed(-((6.0f / 10.0f) * Player::instance()->cars.GetCurrentCar()->getcurrentSpeed()));
					currentmesh->camfreezeposition = Player::instance()->cars.GetCurrentCar()->GetPostition()[0];
					collided = true;
					while (foundposition)
					{
						bool x = Physics::CheckCollision(A, B, C, D, A2, B2, C2, D2);
						if (!x)
						{
							break;
						}
						else
						{
							currentmesh->camfreezeposition = currentmesh->camfreezeposition + pushback * 0.1f;
							A2 += pushback * 0.1f;
							B2 += pushback * 0.1f;
							C2 += pushback * 0.1f;
							D2 += pushback * 0.1f;
						}
					}
					currentmesh->camfreezeposition.y = 0;
					Player::instance()->cars.GetCurrentCar()->SetPosition(0, currentmesh->camfreezeposition);


				}
			}
			else if (currentmesh->camcollided)
			{
				Player::instance()->cars.GetCurrentCar()->SetPosition(0, currentmesh->camfreezeposition);
				bool x = Physics::CheckCollision(A, B, C, D, A2, B2, C2, D2);
				if (!x)
				{
					currentmesh->camcollided = false;
					currentmesh->camfreezeposition = Vector3(0, 0, 0);
				}
			}

		}
		
	}

	return collided;

}

void DrivingScene::RenderMesh(Mesh* mesh, bool enableLight, bool hasCollision)
{

	if (hasCollision)
	{
		mesh->ColisionVector1 = mesh->initColisionVector1;
		mesh->ColisionVector2 = mesh->initColisionVector2;
		mesh->ColisionVector3 = mesh->initColisionVector3;
		mesh->ColisionVector4 = mesh->initColisionVector4;
		mesh->ColisionVector1 = modelStack.Top().GetTranspose().Multiply(mesh->initColisionVector1);
		mesh->ColisionVector2 = modelStack.Top().GetTranspose().Multiply(mesh->initColisionVector2);
		mesh->ColisionVector3 = modelStack.Top().GetTranspose().Multiply(mesh->initColisionVector3);
		mesh->ColisionVector4 = modelStack.Top().GetTranspose().Multiply(mesh->initColisionVector4);
		mesh->collison = true;
		mesh->collisionboxcreated = true;
		
		mesh->Collider = MeshBuilder::GenerateCollisonBox("COLLISIONBOX", mesh->initColisionVector1, mesh->initColisionVector2, mesh->initColisionVector3, mesh->initColisionVector4, mesh->initColisionVector1, mesh->initColisionVector2, mesh->initColisionVector3, mesh->initColisionVector4);
		modelStack.PushMatrix();
		RenderMesh(mesh->Collider, false, false);
		modelStack.PopMatrix();

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

void DrivingScene::RenderSkybox()
{
	float size = bordersize;//uniform scaling
	float offset = size / 200;//used to prevent lines appearing
	
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

void DrivingScene::RenderText(Mesh* mesh, std::string text, Color color)
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

void DrivingScene::RenderTextOnScreen(Mesh* mesh, std::string text, Color color, float size, float x, float y)
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
void DrivingScene::RenderFramerate(Mesh* mesh, Color color, float size, float x, float y)
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

void DrivingScene::RenderMeshOnScreen(Mesh* mesh, int x, int y, int sizex, int sizey, float rotation)
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
	modelStack.Rotate(rotation, 0, 0, 1);
	modelStack.Scale(sizex, sizey, 1);
	RenderMesh(mesh, false, false);
	modelStack.PopMatrix();
	viewStack.PopMatrix();
	projectionStack.PopMatrix();
}