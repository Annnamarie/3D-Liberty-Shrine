///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
* LoadSceneTextures()
*
* This method is used for preparing the 3D scene by loading
* the shapes, textures in memory to support the 3D scene
* rendering
***********************************************************/
void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;
	bReturn = CreateGLTexture(
		"../../Utilities/textures/stoneTexture.jpg",
		"stone");
	bReturn = CreateGLTexture(
		"../../Utilities/textures/bushTexture.jpg",
		"bush");
	bReturn = CreateGLTexture(
		"../../Utilities/textures/groundTexture.jpg",
		"ground");
	bReturn = CreateGLTexture(
		"../../Utilities/textures/skyTexture.jpg",
		"sky");

	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

/**************************************************************/
/*** MILESTONE 2                                            ***/
/*** Annamarie Cortes                                       ***/
/*** September 22, 2024                                     ***/
/*** Shapes - Plane, box, torus                             ***/
/**************************************************************/

void SceneManager::DefineObjectMaterials()
{
	/*** STUDENTS - add the code BELOW for defining object materials. ***/
	/*** There is no limit to the number of object materials that can ***/
	/*** be defined. Refer to the code in the OpenGL Sample for help  ***/

	// Box 1 Material
	OBJECT_MATERIAL box1Material;
	box1Material.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	box1Material.ambientStrength = 0.1f;
	box1Material.diffuseColor = glm::vec3(0.6f, 0.5f, 0.4f); // Light brown
	box1Material.specularColor = glm::vec3(0.2f, 0.3f, 0.4f);
	box1Material.shininess = 0.5;
	box1Material.tag = "box1";
	m_objectMaterials.push_back(box1Material);

	// Box 2 Material
	OBJECT_MATERIAL box2Material;
	box2Material.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	box2Material.ambientStrength = 0.1f;
	box2Material.diffuseColor = glm::vec3(0.6f, 0.5f, 0.4f); // Light brown
	box2Material.specularColor = glm::vec3(0.2f, 0.3f, 0.4f);
	box2Material.shininess = 0.5;
	box2Material.tag = "box2";
	m_objectMaterials.push_back(box2Material);

	// Box 3 Material
	OBJECT_MATERIAL box3Material;
	box3Material.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	box3Material.ambientStrength = 0.1f;
	box3Material.diffuseColor = glm::vec3(0.6f, 0.5f, 0.4f); // Light brown
	box3Material.specularColor = glm::vec3(0.2f, 0.3f, 0.4f);
	box3Material.shininess = 0.5;
	box3Material.tag = "box3";
	m_objectMaterials.push_back(box3Material);

	// Box 4 Material
	OBJECT_MATERIAL box4Material;
	box4Material.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	box4Material.ambientStrength = 0.1f;
	box4Material.diffuseColor = glm::vec3(0.6f, 0.5f, 0.4f); // Light brown
	box4Material.specularColor = glm::vec3(0.2f, 0.3f, 0.4f);
	box4Material.shininess = 0.5;
	box4Material.tag = "box4";
	m_objectMaterials.push_back(box4Material);

	// Prism Material
	OBJECT_MATERIAL prismMaterial;
	prismMaterial.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	prismMaterial.ambientStrength = 0.1f;
	prismMaterial.diffuseColor = glm::vec3(0.8f, 0.7f, 0.5f); // Lighter brown for prism
	prismMaterial.specularColor = glm::vec3(0.2f, 0.3f, 0.4f);
	prismMaterial.shininess = 0.5;
	prismMaterial.tag = "prism";
	m_objectMaterials.push_back(prismMaterial);

	// Torus Material
	OBJECT_MATERIAL torusMaterial;
	torusMaterial.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	torusMaterial.ambientStrength = 0.1f;
	torusMaterial.diffuseColor = glm::vec3(0.3f, 0.7f, 0.5f); // Greenish
	torusMaterial.specularColor = glm::vec3(0.2f, 0.3f, 0.4f);
	torusMaterial.shininess = 0.7; // More shiny for the torus
	torusMaterial.tag = "torus";
	m_objectMaterials.push_back(torusMaterial);

	// Top Plane Material
	OBJECT_MATERIAL topPlaneMaterial;
	topPlaneMaterial.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	topPlaneMaterial.ambientStrength = 0.1f;
	topPlaneMaterial.diffuseColor = glm::vec3(0.9f, 0.9f, 0.9f); // Light gray for top plane
	topPlaneMaterial.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
	topPlaneMaterial.shininess = 0.8; // Slightly shiny
	topPlaneMaterial.tag = "topPlane";
	m_objectMaterials.push_back(topPlaneMaterial);

	// Bottom Plane Material
	OBJECT_MATERIAL bottomPlaneMaterial;
	bottomPlaneMaterial.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	bottomPlaneMaterial.ambientStrength = 0.1f;
	bottomPlaneMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f); // Dark gray for bottom plane
	bottomPlaneMaterial.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
	bottomPlaneMaterial.shininess = 0.3; // Less shiny
	bottomPlaneMaterial.tag = "bottomPlane";
	m_objectMaterials.push_back(bottomPlaneMaterial);
}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure the light
 *  sources for the 3D scene.  There are up to 4 light sources.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	// this line of code is NEEDED for telling the shaders to render 
	// the 3D scene with custom lighting, if no light sources have
	// been added then the display window will be black - to use the 
	// default OpenGL lighting then comment out the following line
	//m_pShaderManager->setBoolValue(g_UseLightingName, true);

	/*** STUDENTS - add the code BELOW for setting up light sources ***/
	/*** Up to four light sources can be defined. Refer to the code ***/
	/*** in the OpenGL Sample for help                              ***/

	//light source 1 - mimicking the sun at midday (slight yellow tint)
	
	//sunlight angle 30-45 degrees above the horixon
	m_pShaderManager->setVec3Value("lightSources[0].position", 10.0f, 14.0f, 5.0f);
	//light blue tint for the sky
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.2f, 0.2f, 0.5f);
	//yellowish sunlight tone
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 1.0f, 0.95f, 0.8f);
	//bright highlights
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", 1.0f, 1.0f, 0.9f);
	//shininess factor
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 64.0f);
	//intensity for reflective areas
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.8f);


	//after more research - adding more sources of light as light bounces off and it is not just from the sunlight
	//light source 2 - Fill light - light reflecting off surrounding objects

	//opposite side of the sunlight
	m_pShaderManager->setVec3Value("lightSources[1].position", -5.0f, 5.0f, -3.0f);
	//soft green tint for light reflecting off the bushes
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.05f, 0.1f, 0.05f);
	//low intensity, greenish fill light
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.2f, 0.3f, 0.2f);
	//Fill
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", 0.0f, 0.0f, 0.0f);
	//m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 32.0f);
	//no reflection
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.0f);

	//light source 3 - Bounce Light  - stimulating the light bouncing off the ground
	
	//near the ground, close to the monument
	m_pShaderManager->setVec3Value("lightSources[2].position", 0.0f, 0.5f, 0.0f);
	//warm reflection off the ground
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", 0.05f, 0.04f, 0.03f);
	//low intensity, soft light
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", 0.1f, 0.1f, 0.08f);
	//No specular
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", 0.0f, 0.0f, 0.0f);
	//m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 23.0f);
	//no specular reflection
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.0f);

	//light soure 4 - Backlight to provide constrast, ligthing the scene from behind the monument

	//behind the monument
	m_pShaderManager->setVec3Value("lightSources[3].position", 0.0f, 14.0f, -10.0f);
	//soft backlight
	m_pShaderManager->setVec3Value("lightSources[3].ambientColor", 0.05f, 0.05f, 0.05f);
	//low intensity
	m_pShaderManager->setVec3Value("lightSources[3].diffuseColor", 0.2f, 0.2f, 0.2f);
	//no specular
	m_pShaderManager->setVec3Value("lightSources[3].specularColor", 0.0f, 0.0f, 0.0f);
	//m_pShaderManager->setFloatValue("lightSources[3].focalStrength", 23.0f);
	//no specular reflection
	m_pShaderManager->setFloatValue("lightSources[3].specularIntensity", 0.0f);

	//enabling custom lighting
	m_pShaderManager->setBoolValue("bUseLighting", true);
}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// load the texture image files for the textures applied
	// to objects in the 3D scene
	LoadSceneTextures();
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadTorusMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();
	m_basicMeshes->LoadPrismMesh();

}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// Bottom Plane - GROUND
	
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1, 1, 1, 1);

	//ground texture
	SetShaderTexture("ground");

	//lighting
	SetShaderMaterial("bottomPlane");

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/
	// Top Plane - Background

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 9.0f, -10.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1, 1, 1, 1);

	//sky texture
	SetShaderTexture("sky");

	//lighting
	SetShaderMaterial("topPlane");

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/

	/****************************************************************/
	// Torus

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(10.0f, 6.0f, 2.0f);

	// set the XYZ rotation for the mesh
	// rotated to 90 degress to match the ground
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//setting color to green - hedge torys
	SetShaderColor(0.243, 0.651, 0.286, 1);

	//torus texture
	SetShaderTexture("bush");

	//lighting
	SetShaderMaterial("torus");


	// draw the mesh with transformation values
	m_basicMeshes->DrawTorusMesh();
	/****************************************************************/

	/****************************************************************/
	// Box 1 - first box for the structure (from bottom to top)

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(7.0f, 4.0f, 3.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 1.0f, 2.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//Structure Color - Light Brown
	SetShaderColor(0.871, 0.804, 0.675, 1);
	//Texture to stone
	SetShaderTexture("stone");

	//lighting
	SetShaderMaterial("box1");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	// Box 2 -  box for the structure (from bottom to top)

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(5.0f, 2.5f, 3.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f,3.5f, 2.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//Structure Color - Light Brown
	SetShaderColor(0.871, 0.804, 0.675, 1);
	//Texture to stone
	SetShaderTexture("stone");

	//ligting
	SetShaderMaterial("box2");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	// Box 3 -  box for the structure (from bottom to top)

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(3.5f, 3.0f, 2.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 6.0f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//Structure Color - Light Brown
	SetShaderColor(0.871, 0.804, 0.675, 1);
	//Texture to stone
	SetShaderTexture("stone");

	//lighting
	SetShaderMaterial("box3");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	// Box 4 -  box for the structure (from bottom to top)

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(2.0f, 1.0f, 2.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 8.0f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//Structure Color - Light Brown
	SetShaderColor(0.871, 0.804, 0.675, 1);
	//Texture to stone
	SetShaderTexture("stone");

	//lighting
	SetShaderMaterial("box4");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	/****************************************************************/
	// Prism -  prism for the top for the structure (from bottom to top)
	// there will be a sphere on top of prism which will be added later

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(1.75f, 2.0f, 2.3f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = -90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 9.3f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//Structure Color - Light Brown
	SetShaderColor(0.871, 0.804, 0.675, 1);
	//Texture to stone
	SetShaderTexture("stone");
	
	//lighting
	SetShaderMaterial("prism");

	// draw the mesh with transformation values
	m_basicMeshes->DrawPrismMesh();
	/****************************************************************/
}
