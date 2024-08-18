#include "BVHData.h"
#include <math.h>

// constructor
BVHData::BVHData()
	{ // constructor
	} // constructor

// read data from bvh file
// a basic recursive-descent parser
bool BVHData::ReadFileBVH(const char* fileName)
	{ // ReadFileBVH()
	// open a file stream and check validity
	std::ifstream inFile(fileName);
	if (inFile.bad())
		return false;

	// temporary storage to read in a single line
	std::string line;
	// a vector of the tokens on the line
	std::vector<std::string> tokens;
	
	// loop through the file one line at a time
	while (std::getline(inFile, line))
		{ // more lines in the file
		// not an empty line
		if (line.size() != 0)
			{ // non-empty line
			// chop the line into tokens
			StringSplit(line, tokens);

			// if the first token is HIERARCHY, it is the logical structure of the character
			if (tokens[0] == "HIERARCHY")
				{ // hierarchy
				// read in a line and split it into tokens
				NewLine(inFile, tokens);
				// create a joint object 
				Joint joint;
				// read in the hierarchy based at the joint
				ReadHierarchy(inFile, tokens, joint, -1);
				// and set it to be the root
				this->root = joint;
				} // hierarchy
			// otherwise, if the first token is MOTION, it is the animation data
			else if (tokens[0] == "MOTION")
				{ // motion
				ReadMotion(inFile);
				break;
				} // motion
			else
				{ // otherwise
				// ignore everything els
				} // otherwise
			} // non-empty line
		} // more lines in the file

	// do a recursive search to set up the vector of pointers to the joints
	GetAllJoints(this->root, this->all_joints);
	// load all rotation and translation data into this class
	loadAllData(this->boneRotations, this->boneTranslations, this->frames);
	return true;
	} // ReadFileBVH()

// read a single line and tokenise it
void BVHData::NewLine(std::ifstream& inFile, std::vector<std::string>& tokens)
	{ // NewLine()
	// the next line
	std::string line;
	// empty out the array of tokens
	tokens.clear();
	// read in the line
	std::getline(inFile, line);
	// and split it into tokens
	StringSplit(line, tokens);
	} // NewLine()

// split string with the given key character
void BVHData::StringSplit(std::string input, std::vector<std::string>& tokens)
	{ // StringSplit()
	// empty out the array of tokens
	tokens.clear();
	// create a local string stream for tokenising
	std::istringstream iss(input);
	while (!iss.eof()) 
		{ // more tokens to come
		// create a token
		std::string token;
		// read it in
		iss >> token;
		// and add it to the vector
		tokens.push_back(token);
		} // more tokens to come
	} // StringSplit()

// check whether the given string is a number
bool BVHData::isNumeric(const std::string& str) 
	{ // isNumeric()
	// loop through the characters in the string
	for (char c : str) 
		// if it's not a digit, return early
		if (!std::isdigit(c)) 
			return false;
	// return success
	return true;
	} // isNumeric()

// recursive descent parser for the hierarchy
void BVHData::ReadHierarchy(std::ifstream& inFile, std::vector<std::string>& line, Joint& joint, int parent)
	{ // ReadHierarchy()
	// the new joint will have the next available ID 
	// (0 offset means that it will be equal to the existing size)
	joint.id = this->Bones.size();
	// the second token (#1) will be the name of the joint
	joint.joint_name = line[1];
	// add the name to the list
	this->Bones.push_back(joint.joint_name);
	// and set the parent bone
	this->parentBones.push_back(parent);
	// this seems redundant, but removing it crashes. This works. For now.
	int id = this->parentBones.size() - 1;
	// read in the next line & tokenise it
	NewLine(inFile, line);
	// if the beginning of the line is a left brace, we're starting a group of children
	if (line[0] == "{")
		{ // group of children
		// ignore the rest of the line and read in a new one
		NewLine(inFile, line);
		while (line[0] != "}")
			{ // until we hit the close of the group
			// The first token tells us which type of line
			// OFFSET is the offset from the parent
			if (line[0] == "OFFSET")
				{
				joint.joint_offset[0] = std::stof(line[1]);
				joint.joint_offset[1] = std::stof(line[2]);
				joint.joint_offset[2] = std::stof(line[3]);
				}
			// CHANNELS defines how many floats are needed for the animation, and which ones
			else if (line[0] == "CHANNELS")
				{ // channel information
				for (int i = 0; i < std::stoi(line[1]); i++)
					joint.joint_channel.push_back(line[i + 2]);
				} // channel information
			// JOINT defines a new joint
			else if (line[0] == "JOINT")
				{ // joint information
				Joint child;
				ReadHierarchy(inFile, line, child, id);
				joint.Children.push_back(child);
				} // joint information 
			// At the leaf of the hierarchy, there is no joint. Instead it says End 
			else if (line[0] == "End")
				{ // end site
				// read in and ignore three extra lines
				for (int i = 0; i < 3; i++) 
					NewLine(inFile, line);
				} // end site
			// always read the next line when done processing this line
			NewLine(inFile, line);
			} // until we hit the close of the group
		} // group of children
	} // ReadHierarchy()

// read motion(frames) from file
void BVHData::ReadMotion(std::ifstream& inFile)
	{ // ReadMotion()
	// a single line from the file
	std::string line;
	// and the tokens it breaks into
	std::vector<std::string> tokens;
	// the next line should specify how many frames, so read it in
	NewLine(inFile, tokens);
	// convert to an integer and save
	this->frame_count = std::stoi(tokens[1]);
	// the next line should specify how many seconds per frame, so read it in
	NewLine(inFile, tokens);
	// and convert it to a float
	this->frame_time = std::stof(tokens[2]);
	// after that, we loop until the end of the file
	while (std::getline(inFile, line))
		{ // more data
		// ignore empty lines
		if (line.size() != 0)
			{ // non-empty line
			// split the line into tokens
			StringSplit(line, tokens);
			// create a vector to store all the animation data for the frame
			std::vector<float> frame;
			// loop through the line, one token at a time
			for (size_t i = 0; i < tokens.size(); i++)
				{ // per token
				if (!isNumeric(tokens[i]))
					{ // if it's not numeric
					frame.push_back(std::stof(tokens[i]));
					} // if it's not numeric
				} // per token
			// store frame into this class
			this->frames.push_back(frame);
			}
		} // more data
	} // ReadMotion()

// render hierarchy for a given frame
void BVHData::Render(Matrix4& viewMatrix, float scale, int frame, float groundHeight)
    { // Render()

    //Change the initial height of the skeleton depending on the ground
    Matrix4 initialHeight = Matrix4::Translate({0, groundHeight, 0});

    RenderJoint(viewMatrix, initialHeight, &this->root, scale, frame);
	} // Render()


void BVHData::RenderBlend(Matrix4 &viewMatrix, float scale, int frame, float groundHeight, BVHData &blend, float t, int blendFrame)
{
    //Set initial Height of the ground
    Matrix4 initialHeight = Matrix4::Translate({0, groundHeight, 0});

    //Pass all other information to RenderBlendJoint
    RenderBlendedJoint(viewMatrix, initialHeight, &this->root, &blend.root, blend.boneRotations, blendFrame, blend.frame_count, t, scale, frame);
}

// render a single joint for a given frame
void BVHData::RenderJoint(Matrix4& viewMatrix, Matrix4 parentMatrix, Joint* joint, float scale, int frame)
    { // RenderJoint

      //Get which frame to render
      int framePos = frame % frame_count;

      //The position of the joint in its own coordinate system is (0,0,0)
      Cartesian3 jointPosition = {0,0,0};

      //Get the joint offset
      Cartesian3 jointOffset = Cartesian3(joint->joint_offset[0], joint->joint_offset[1], joint->joint_offset[2]) * scale;

      //Get the translation matrix
      Matrix4 jointTranslateMatrix = Matrix4::Translate(jointOffset);

      //Get the rotation data from the channels
      //Note we negate the angles, since they are meant for a right hand coordinate system, but the Matrix4::Rotate functions are in a left hand coordinate system
      Cartesian3 jointRotations = -Cartesian3(boneRotations[framePos][joint->id].x, boneRotations[framePos][joint->id].y, boneRotations[framePos][joint->id].z);
      Matrix4 jointRotateX, jointRotateY, jointRotateZ;
      jointRotateX = Matrix4::RotateX(jointRotations.x);
      jointRotateY = Matrix4::RotateY(jointRotations.y);
      jointRotateZ = Matrix4::RotateZ(jointRotations.z);

      //Rotate in ZYX order (looks more natural)
      Matrix4 jointRotateMatrix = jointRotateX * jointRotateY * jointRotateZ;

      //Create transformation matrix
      //Take the parent matrix, then translate it, then rotate it
      Matrix4 jointTransformMatrix = parentMatrix * jointTranslateMatrix;
      jointTransformMatrix = jointTransformMatrix * jointRotateMatrix;

      //We now have the position of the joint in the root coordinate system
      jointPosition = jointTransformMatrix * jointPosition;

      //Loop through all children
      for (Joint child : joint->Children)
      {

          //The child is at {0,0,0} in its own coordinate system
          Cartesian3 childPosition = {0,0,0};

          //Get child translate data and set up matrix
          Cartesian3 childOffset = Cartesian3(child.joint_offset[0], child.joint_offset[1], child.joint_offset[2]) * scale;
          Matrix4 childTranslateMatrix = Matrix4::Translate(childOffset);

          //Get child rotation data and set up matrix
          Cartesian3 childRotations = -Cartesian3(boneRotations[framePos][child.id].x, boneRotations[framePos][child.id].y, boneRotations[framePos][child.id].z);
          Matrix4 childRotateX, childRotateY, childRotateZ;
          childRotateX = Matrix4::RotateX(childRotations.x);
          childRotateY = Matrix4::RotateY(childRotations.y);
          childRotateZ = Matrix4::RotateZ(childRotations.z);

          //Rotate in ZYX order
          Matrix4 childRotationMatrix = childRotateX * childRotateY * childRotateZ;

          //Set up child transformation matrix
          Matrix4 childTransformMatrix = jointTransformMatrix * childTranslateMatrix;
          childTransformMatrix = childTransformMatrix * childRotationMatrix;

          //Get the child's position in the root coordinate system
          childPosition = childTransformMatrix * childPosition;

          //Render cylinder between parent and child joints
          RenderCylinder(viewMatrix, jointPosition, childPosition);

          //Recursively call for each child
          RenderJoint(viewMatrix, jointTransformMatrix, &child, scale, frame);
      }
	} // RenderJoint()

void BVHData::RenderBlendedJoint(Matrix4 &viewMatrix, Matrix4 parentMatrix, Joint *joint, Joint *blendJoint, std::vector<std::vector<Cartesian3>> &blendBoneRotations, int blendFrame, int blendFrameCount, float t, float scale, int frame)
{
    //Get which frame to render
    int framePos = frame % frame_count;

    int blendedFramePos = blendFrame % blendFrameCount;

    //The position of the joint in its own coordinate system is (0,0,0)
    Cartesian3 jointPosition = {0,0,0};

    //Get the joint offset
    Cartesian3 jointOffset = Cartesian3(joint->joint_offset[0], joint->joint_offset[1], joint->joint_offset[2]) * scale;

    //Get the translation matrix
    Matrix4 jointTranslateMatrix = Matrix4::Translate(jointOffset);

    //Get the rotation data from the channels
    //Note we negate the angles, since they are meant for a right hand coordinate system, but the Matrix4::Rotate functions are in a left hand coordinate system
    Cartesian3 jointRotations = -Cartesian3(boneRotations[framePos][joint->id].x, boneRotations[framePos][joint->id].y, boneRotations[framePos][joint->id].z);

    //Interpolate the angles
    jointRotations = jointRotations * t;

    //Now get the rotations from the second animation and do the same
    Cartesian3 blendJointRotations = -Cartesian3(blendBoneRotations[blendedFramePos][blendJoint->id].x, blendBoneRotations[blendedFramePos][blendJoint->id].y, blendBoneRotations[blendedFramePos][blendJoint->id].z);

    //Interpolate the angles
    blendJointRotations = blendJointRotations * (1-t);

    //Add them together for the final angles
    Cartesian3 finalRotation = jointRotations + blendJointRotations;

    Matrix4 jointRotateX, jointRotateY, jointRotateZ;
    jointRotateX = Matrix4::RotateX(finalRotation.x);
    jointRotateY = Matrix4::RotateY(finalRotation.y);
    jointRotateZ = Matrix4::RotateZ(finalRotation.z);

    //Combine to create rotation matrix
    Matrix4 jointRotateMatrix = jointRotateX * jointRotateY * jointRotateZ;

    //Create transformation matrix
    //Take the parent matrix, then translate it, then rotate it
    Matrix4 jointTransformMatrix = parentMatrix * jointTranslateMatrix;
    jointTransformMatrix = jointTransformMatrix * jointRotateMatrix;

    //We now have the position of the joint in the root coordinate system
    jointPosition = jointTransformMatrix * jointPosition;

    //Loop through all children
    //Use numbers since we want access to both current and blend joint
    for(int i = 0; i < joint->Children.size(); i++)
    {
        //The child is at {0,0,0} in its own coordinate system
        Cartesian3 childPosition = {0,0,0};

        //Get child translate data and set up matrix
        Cartesian3 childOffset = Cartesian3(joint->Children[i].joint_offset[0], joint->Children[i].joint_offset[1], joint->Children[i].joint_offset[2]) * scale;
        Matrix4 childTranslateMatrix = Matrix4::Translate(childOffset);

        //Get child rotation data and set up matrix
        Cartesian3 childRotations = -Cartesian3(boneRotations[framePos][joint->Children[i].id].x, boneRotations[framePos][joint->Children[i].id].y, boneRotations[framePos][joint->Children[i].id].z);

        //Interpolate
        childRotations = childRotations * t;

        //Get blend animation's rotations
        Cartesian3 blendChildRotations = -Cartesian3(blendBoneRotations[blendedFramePos][blendJoint->Children[i].id].x, blendBoneRotations[blendedFramePos][blendJoint->Children[i].id].y, blendBoneRotations[blendedFramePos][blendJoint->Children[i].id].z);

        blendChildRotations = blendChildRotations * (1-t);

        Cartesian3 finalChildRotation = childRotations + blendChildRotations;

        Matrix4 childRotateX, childRotateY, childRotateZ;
        childRotateX = Matrix4::RotateX(finalChildRotation.x);
        childRotateY = Matrix4::RotateY(finalChildRotation.y);
        childRotateZ = Matrix4::RotateZ(finalChildRotation.z);

        //Rotate in ZYX order
        Matrix4 childRotationMatrix = childRotateX * childRotateY * childRotateZ;

        //Set up child transformation matrix
        Matrix4 childTransformMatrix = jointTransformMatrix * childTranslateMatrix;
        childTransformMatrix = childTransformMatrix * childRotationMatrix;

        //Get the child's position in the root coordinate system
        childPosition = childTransformMatrix * childPosition;

        //Render cylinder between parent and child joints
        RenderCylinder(viewMatrix, jointPosition, childPosition);

        //Recursively call for each child
        RenderBlendedJoint(viewMatrix, jointTransformMatrix, &joint->Children[i], &blendJoint->Children[i], blendBoneRotations, blendFrame, blendFrameCount, t, scale, frame);
    }


}

// render cylinder given the start position and the end position
void BVHData::RenderCylinder(Matrix4& viewMatrix, Cartesian3 start, Cartesian3 end)
	{ // RenderCylinder()

    //Rotate so the character stands upright
    Matrix4 view = viewMatrix * Matrix4::RotateX(270);

    //Calculate the vector between the two points
    Cartesian3 difference = end - start;

    //Translate the start to be the origin
    Cartesian3 endTranslated = Matrix4::Translate(-start) * end;
    Cartesian3 zAxis = {0,0,1};

    //Get the rotation between the z axis and the endVector
    Matrix4 rotation = Matrix4::GetRotation(zAxis, endTranslated);

    //Set up transformation matrix
    Matrix4 transformMatrix = rotation;
    transformMatrix = Matrix4::Translate(start) * transformMatrix;

    Matrix4 finalMatrix = view * transformMatrix;

    //We make the cylinder have a radius of 0.2, with 8 slices
    Cylinder(finalMatrix, 0.2, difference.length(), 8);

	} // RenderCylinder()

// render a single cylinder given radius, length and vertical slices
void BVHData::Cylinder(Matrix4& viewMatrix, float radius, float Length, int slices)
	{  // Cylinder()
	// start a set of triangles
	glBegin(GL_TRIANGLES);
	// loop through the given number of slices
	for (int i = 0; i < slices; i++) 
		{ // per slice
		// work out the angles around the main axis for the start and end of the slice
		float theta = (float)(i * 2.0f * M_PI / slices);
		float nextTheta = (float)((i + 1) * 2.0f * M_PI / slices);
		float midTheta = 0.5 * (theta + nextTheta);

		// the top vertex is always in the same place
		Homogeneous4 center_up = viewMatrix * Homogeneous4(0.0, 0.0, Length, 1);
		// we have two points on the upper circle of the cylinder
		Homogeneous4 c_edge1 = viewMatrix * Homogeneous4(radius * cos(theta), radius * sin(theta), Length, 1);
		Homogeneous4 c_edge2 = viewMatrix * Homogeneous4(radius * cos(nextTheta), radius * sin(nextTheta), Length, 1);
		// and two points on the bottom circle
		Homogeneous4 c_edge3 = viewMatrix * Homogeneous4(radius * cos(nextTheta), radius * sin(nextTheta), 0, 1);
		Homogeneous4 c_edge4 = viewMatrix * Homogeneous4(radius * cos(theta), radius * sin(theta), 0, 1);
		// and a point in the middle of the bottom
		Homogeneous4 center_bottom = viewMatrix * Homogeneous4(0.0, 0.0, 0, 1);

		// normal vectors are tricky because we need to AVOID using the translation
		// We can either use a triangle face normal, or we can do a hack ;-)
		// because we know that they are from the origin to given points

		// we have three normals: one for the top
		Cartesian3 normal_up = viewMatrix * Cartesian3(0, 0, 1.0) - viewMatrix * Cartesian3(0.0, 0.0, 0.0);
		// one for the middle
		Cartesian3 normal_edge = viewMatrix * Cartesian3(cos(midTheta), sin(midTheta), 0.0) - viewMatrix * Cartesian3(0.0, 0.0, 0.0);
		// and one for the bottom
		Cartesian3 normal_bottom = viewMatrix * Cartesian3(0, 0, -1.0) - viewMatrix * Cartesian3(0.0, 0.0, 0.0);

		// render the top triangle
		glNormal3fv(&normal_up.x);
		glVertex4fv(&center_up.x);
		glVertex4fv(&c_edge1.x);
		glVertex4fv(&c_edge2.x);

		// and the side triangles
		glNormal3fv(&normal_edge.x);
		glVertex4fv(&c_edge2.x);
        glVertex4fv(&c_edge1.x);
		glVertex4fv(&c_edge4.x);

		glNormal3fv(&normal_edge.x);
		glVertex4fv(&c_edge2.x);
		glVertex4fv(&c_edge4.x);
		glVertex4fv(&c_edge3.x);

		// and the bottom triangle
		glNormal3fv(&normal_bottom.x);
		glVertex4fv(&c_edge3.x);
		glVertex4fv(&c_edge4.x);
		glVertex4fv(&center_bottom.x);

		}
	glEnd();
	} // Cylinder()

// get all joints in a sequence by searching the tree structure and store it into this class
void BVHData::GetAllJoints(Joint& joint, std::vector<Joint*>& joint_list)
	{ // GetAllJoints()
    // it's recursive, so push the current one
	joint_list.push_back(&joint);
	// base case: end site, so return
	if (joint.Children.size() == 0)
		return;
	// otherwise, loop through the children
	for (size_t i = 0; i < joint.Children.size(); i++)
		// adding them recursively
		GetAllJoints(joint.Children[i], joint_list);
	} // GetAllJoints()

// load all rotation and translation data into this class
void BVHData::loadAllData(std::vector<std::vector<Cartesian3>>& rotations, std::vector<Cartesian3>& translations, std::vector<std::vector<float>>& frames)
	{ // loadAllData()
	// store all rotations
	for (size_t i = 0; i < frames.size(); i++)
		{ // per frame 
		std::vector<Cartesian3> frame_rotations;
		loadRotationData(frame_rotations, frames[i]);
		rotations.push_back(frame_rotations);
		} // per frame
	// store all offsets/translations
	for (size_t i = 0; i < this->all_joints.size(); i++)
		{ // per joint
		float x = this->all_joints[i]->joint_offset[0];
		float y = this->all_joints[i]->joint_offset[1];
		float z = this->all_joints[i]->joint_offset[2];
		translations.push_back(Cartesian3(x, y, z));
		} // per joint
	} // loadAllData()

// load all rotation data into this class
void BVHData::loadRotationData(std::vector<Cartesian3>& rotations, std::vector<float>& frames)
	{ // loadRotationData()
	for (size_t j = 0, j_c = 0; j < frames.size(); j_c++)
		{ // per frame
		float rotation[3] = { 0, 0, 0 };
		for (size_t k = 0; k < this->all_joints[j_c]->joint_channel.size(); k++) // for each channel
			{ // per channel
			// if the channel includes "rotation" in the name
			if (this->all_joints[j_c]->joint_channel[k].substr(1) == "rotation")
				{ // rotation channel
				int rotation_id = BVH_CHANNEL[this->all_joints[j_c]->joint_channel[k]];
				rotation[rotation_id - 3] = frames[j + k];
				} // rotation channel
			} // per channel
		// convert to a rotation
		Cartesian3 rot_3(rotation[0], rotation[1], rotation[2]);
		rotations.push_back(rot_3);
		j += this->all_joints[j_c]->joint_channel.size();
		} // per frame
	} // loadRotationData()
