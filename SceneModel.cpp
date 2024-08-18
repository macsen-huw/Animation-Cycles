///////////////////////////////////////////////////
//
//	Hamish Carr
//	October, 2023
//
//	------------------------
//	SceneModel.cpp
//	------------------------
//	
//	The model of the scene
//
//	
///////////////////////////////////////////////////

#include "SceneModel.h"
#include <math.h>
#include <iostream>

// three local variables with the hardcoded file names
const char* groundModelName		= "./models/randomland.dem";
const char* characterModelName	= "./models/human_lowpoly_100.obj";
const char* motionBvhStand		= "./models/stand.bvh";
const char* motionBvhRun		= "./models/fast_run.bvh";
const char* motionBvhveerLeft	= "./models/veer_left.bvh";
const char* motionBvhveerRight	= "./models/veer_right.bvh";
const float cameraSpeed = 0.5;

const Homogeneous4 sunDirection(0.5, -0.5, 0.3, 1.0);
const GLfloat groundColour[4] = { 0.2, 0.5, 0.2, 1.0 };
const GLfloat boneColour[4] = { 0.7, 0.7, 0.4, 1.0 };
const GLfloat sunAmbient[4] = {0.1, 0.1, 0.1, 1.0 };
const GLfloat sunDiffuse[4] = {0.7, 0.7, 0.7, 1.0 };
const GLfloat blackColour[4] = {0.0, 0.0, 0.0, 1.0};



// constructor
SceneModel::SceneModel()
	{ // constructor
	// load the object models from files
	groundModel.ReadFileTerrainData(groundModelName, 3);

	// load the animation data from files
	restPose.ReadFileBVH(motionBvhStand);
	runCycle.ReadFileBVH(motionBvhRun);
	veerLeftCycle.ReadFileBVH(motionBvhveerLeft);
	veerRightCycle.ReadFileBVH(motionBvhveerRight);

	// set the world to opengl matrix
	world2OpenGLMatrix = Matrix4::RotateX(90.0);
	CameraTranslateMatrix = Matrix4::Translate(Cartesian3(-5, 15, -15.5));
	CameraRotationMatrix = Matrix4::RotateX(-30.0) * Matrix4::RotateZ(15.0);

	// initialize the character's position and rotation
	EventCharacterReset();

	// and set the frame number to 0
	frameNumber = 0;

    //Initialise current BVH to restPose
    current = restPose;

    //Set up the character location and rotation
    characterTransform = Matrix4::Identity();
    characterLocation = {0,0,0};
    characterRotation = Matrix4::Identity();
    move = false;
    turn = 0;

	} // constructor

// routine that updates the scene for the next frame
void SceneModel::Update()
	{ // Update()
	// increment the frame counter
    frameNumber++;

	} // Update()

// routine to tell the scene to render itself
void SceneModel::Render()
	{ // Render()
	// enable Z-buffering
	glEnable(GL_DEPTH_TEST);
	
	// set lighting parameters
	glShadeModel(GL_FLAT);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_AMBIENT, sunAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, sunDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, blackColour);
	glLightfv(GL_LIGHT0, GL_EMISSION, blackColour);
	
	// background is sky-blue
	glClearColor(0.7, 0.7, 1.0, 1.0);

	// clear the buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// compute the view matrix by combining camera translation, rotation & world2OpenGL
	viewMatrix = world2OpenGLMatrix * CameraRotationMatrix * CameraTranslateMatrix;

	// compute the light position
  	Homogeneous4 lightDirection = world2OpenGLMatrix * CameraRotationMatrix * sunDirection;
  	
  	// turn it into Cartesian and normalise
  	Cartesian3 lightVector = lightDirection.Vector().unit();

	// and set the w to zero to force infinite distance
 	lightDirection.w = 0.0;
 	 	
	// pass it to OpenGL
	glLightfv(GL_LIGHT0, GL_POSITION, &(lightVector.x));

	// and set a material colour for the ground
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, groundColour);
	glMaterialfv(GL_FRONT, GL_SPECULAR, blackColour);
	glMaterialfv(GL_FRONT, GL_EMISSION, blackColour);

	// render the terrain
    groundModel.Render(viewMatrix);

	// now set the colour to draw the bones
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, boneColour);

    //If the character is moving, update its position
    if(move){
        //Go slower if the character is turning
        if (turn == 1 || turn == 2)
            characterLocation.y = -0.35;
        //Go quickest when running in straight line
        else
            characterLocation.y = -0.5;
    }
    else
        characterLocation.y = 0;

    //Turn the character if necessary
    if(turn == 1)
        characterRotation = Matrix4::RotateZ(355); //Left
    else if (turn == 2)
        characterRotation = Matrix4::RotateZ(5); // Right
    else
        characterRotation = Matrix4::Identity(); // No turn

    //Apply the movement and orientation changes to the character's position
    characterTransform = characterTransform * Matrix4::Translate(characterLocation) * characterRotation;
    Matrix4 characterPosition = viewMatrix * characterTransform;

    //Get the character's position in the ground's coordinate system, so we can get the terrain height
    Cartesian3 characterPos = characterTransform * Cartesian3(0,0,0);
    float groundHeight = groundModel.getHeight(characterPos.x, characterPos.z);

    if(currentlyBlending)
    {
        //Render
        current.RenderBlend(characterPosition, 0.1, frameNumber,  groundHeight, blend, t, blendFrame );
        std::cout << "Currently Blending" << std::endl;

        //Update values for next iteration
        blendFrame += 1;

        t -= (1.0 / 30.0);

        //Check whether to stop blending
        if(t < 0)
        {
            //Make the blended animation the current one
            currentlyBlending = false;
            current = blend;
            frameNumber = blendFrame;

            std::cout << "Stopped Blending" << std::endl;
        }
    }

    else
    {
        //Render the current pose
        //Make it 1/10th of the size in the file by specifying scale = 0.1
        current.Render(characterPosition, 0.1, frameNumber, groundHeight);
    }

    //BLEND HERE
    /*
    Create new function BLEND
    Here, we rotate again and then pass on the rotation to the other
    so current.Blend( BVH Data blended, float t)

    Update t here

    Have a boolean that determines if thingy is here
     *
     *
     * */


    } // Render()

// camera control events: WASD for motion
void SceneModel::EventCameraForward()
	{ // EventCameraForward()
	// update the camera matrix
	CameraTranslateMatrix = CameraTranslateMatrix * CameraRotationMatrix.transpose() * Matrix4::Translate(Cartesian3(0.0f, -cameraSpeed, 0.0f)) * CameraRotationMatrix;
	} // EventCameraForward()

void SceneModel::EventCameraBackward()
	{ // EventCameraBackward()
	// update the camera matrix
	CameraTranslateMatrix = CameraTranslateMatrix * CameraRotationMatrix.transpose() * Matrix4::Translate(Cartesian3(0.0f, cameraSpeed, 0.0f)) * CameraRotationMatrix;
	} // EventCameraBackward()

void SceneModel::EventCameraLeft()
	{ // EventCameraLeft()
	// update the camera matrix
	CameraTranslateMatrix = CameraTranslateMatrix * CameraRotationMatrix.transpose() * Matrix4::Translate(Cartesian3(cameraSpeed, 0.0f, 0.0f)) * CameraRotationMatrix;
	} // EventCameraLeft()
	
void SceneModel::EventCameraRight()
	{ // EventCameraRight()
	// update the camera matrix
	CameraTranslateMatrix = CameraTranslateMatrix * CameraRotationMatrix.transpose() * Matrix4::Translate(Cartesian3(-cameraSpeed, 0.0f, 0.0f)) * CameraRotationMatrix;
	} // EventCameraRight()

// camera control events: RF for vertical motion
void SceneModel::EventCameraUp()
	{ // EventCameraUp()
	// update the camera matrix
	CameraTranslateMatrix = CameraTranslateMatrix * CameraRotationMatrix.transpose() * Matrix4::Translate(Cartesian3(0.0f, 0.0f, -cameraSpeed)) * CameraRotationMatrix;
	} // EventCameraUp()
	
void SceneModel::EventCameraDown()
	{ // EventCameraDown()
	// update the camera matrix
	CameraTranslateMatrix = CameraTranslateMatrix * CameraRotationMatrix.transpose() * Matrix4::Translate(Cartesian3(0.0f, 0.0f, cameraSpeed)) * CameraRotationMatrix;
	} // EventCameraDown()

// camera rotation events: QE for left and right
void SceneModel::EventCameraTurnLeft()
	{ // EventCameraTurnLeft()
	CameraRotationMatrix = CameraRotationMatrix * Matrix4::RotateZ(2.0f);
	} // EventCameraTurnLeft()

void SceneModel::EventCameraTurnRight()
	{ // EventCameraTurnRight()
	CameraRotationMatrix = CameraRotationMatrix * Matrix4::RotateZ(-2.0f);
	} // EventCameraTurnRight()
	
// character motion events: arrow keys for forward, backward, veer left & right
void SceneModel::EventCharacterTurnLeft()
	{ // EventCharacterTurnLeft()

    //Only allow action if blending is not occuring
    if(!currentlyBlending)
    {
        currentlyBlending = true;
        blend = veerLeftCycle;
        move = true;
        turn = 1;
        blendFrame = 0;
        t = 1.0;
    }

	} // EventCharacterTurnLeft()
	
void SceneModel::EventCharacterTurnRight()
    { // EventCharacterTurnRight()
    if(!currentlyBlending)
    {
        currentlyBlending = true;
        blend = veerRightCycle;
        move = true;
        turn = 2;
        blendFrame = 0;
        t = 1.0;
    }


	} // EventCharacterTurnRight()
	
void SceneModel::EventCharacterForward()
	{ // EventCharacterForward()

    if(!currentlyBlending)
    {
        currentlyBlending = true;
        blend = runCycle;
        move = true;
        turn = 0;
        blendFrame = 0;
        t = 1.0;
    }

	} // EventCharacterForward()
	
void SceneModel::EventCharacterBackward()
	{ // EventCharacterBackward()
    if(!currentlyBlending)
    {
        currentlyBlending = true;
        blend = restPose;
        move = false;
        turn = 0;
        blendFrame = 0;
        t = 1.0;
    }

	} // EventCharacterBackward()

// reset character to original position: p
void SceneModel::EventCharacterReset()
	{ // EventCharacterReset()
	this->characterLocation = Cartesian3(0, 0, 0);
	this->characterRotation = Matrix4::Identity();
	} // EventCharacterReset()
