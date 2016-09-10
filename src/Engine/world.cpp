#include "world.h"
#include "camera.h"

//constructor
World::World(): id(0)
{
	library(id);
}

//alternate constructor
World::World(long worldID): id(worldID)
{
	library(id);
}

//destructor
World::~World(){}

//allows world change
void World::changeWorld(long worldID)
{
	id = worldID;
	library(id);
}

void World::masterClear()
{
	if(!master.empty())
			master.clear();
}

//clear Temp
void World::tempClear()
{
	if(!temp.empty())
			temp.clear();
}

//adds correct world to temp list
void World::library(long worldID)
{
	switch ( worldID ) {
        case -1:
                masterClear();
                SmallWorld();
                updateList();
	case 0:
		masterClear();
		basicWorld();
		updateList();
		break;
	case 1:
		masterClear();
		boxedIn();
		updateList();
		break;
	default:
		masterClear();
		tempClear();
	
	}
}

//Updates Temp list with any changes to the master list
bool World::updateList()
{
	tempClear();

	if(!master.empty())
	{
		//makes copy from master to temp
		for(std::list<Object>::iterator it = master.begin(); it != master.end(); ++it)
		{
			temp.push_back(*it);
		}
	
		if(!temp.empty())
			return true;
		else
			return false;
	}


	return false;
}

//returns renderList
std::list<Object> World::GetRenderList()
{
	return temp;
}

///////////////////////////////////////////////////////
///////////////// Custom Worlds///////////////////////
//////////////////////////////////////////////////////

void World::SmallWorld() {
	Poly testpoly = Poly(Vertex(-6.666,6.666,0,1,0,0),
					 Vertex(13.333,6.666,0,1,1,0),
					 Vertex(-6.666,-13.333,0,1,0,1),
					 Vertex(),3,Vector3D(0,0,-1),TEXTURED);

	Object testobj = Object(testpoly,floortexsmall,floortexsmallwidth,floortexsmallheight,Vector3D(),Point3D(0,0,15));

	master.push_back(testobj);
}

//Pre-condition: master list is already cleared
//basic world
void World::basicWorld()
{
	Poly testpoly = Poly(Vertex(-6.666,6.666,0,1,0,0),
					 Vertex(13.333,6.666,0,1,1,0),
					 Vertex(-6.666,-13.333,0,1,0,1),
					 Vertex(),3,Vector3D(0,0,-1),TEXTURED);

	Object testobj = Object(boxtexture,boxtexturewidth,boxtextureheight,Vector3D(),Point3D(-10,-5,15));
	Object testobj2 = Object(boxtexture,boxtexturewidth,boxtextureheight,Vector3D(),Point3D(10,-5,15));
	Object testobj3 = Object(stonetexture,stonetexturewidth,stonetextureheight);
	Object testobj4 = Object(testpoly,boxtexture,boxtexturewidth,boxtextureheight,Vector3D(),Point3D(0,0,15));

	testobj.GenerateCube();

	testobj2.GenerateCube();
	testobj2.SetRenderType(COLORED);

	testobj3.GenerateFloor(75,-10);

	master.push_back(testobj);
	master.push_back(testobj2);	
	master.push_back(testobj3);
	master.push_back(testobj4);
}

void World::boxedIn()
{
	float length = 75;
	float depth = -10;

	Object testobj = Object(boxtexture,boxtexturewidth,boxtextureheight,Vector3D(),Point3D(-10,-5,15));
	Object testobj2 = Object(boxtexture,boxtexturewidth,boxtextureheight,Vector3D(),Point3D(-10,-5,15));
	testobj.GenerateCube(15);
	testobj2.GenerateCube();
	testobj2.SetRenderType(COLORED);

	Object wall1 =  Object(stonetexture,stonetexturewidth,stonetextureheight);
	Object wall2 =  Object(stonetexture,stonetexturewidth,stonetextureheight);
	Object wall3 =  Object(stonetexture,stonetexturewidth,stonetextureheight);
	Object wall4 =  Object(stonetexture,stonetexturewidth,stonetextureheight);
	Object floor =  Object(woodtexture,woodtexturewidth,woodtextureheight);
	Object ceiling =  Object(ceilingtexture,ceilingtexturewidth,ceilingtextureheight);

	wall1.GenerateWall(0,length,length + depth);
	wall2.GenerateWall(1,length,length + depth);
	wall3.GenerateWall(2,length,length + depth);
	wall4.GenerateWall(3,length,length + depth);
	

	floor.GenerateFloor(length, depth);
	ceiling.GenerateCeiling(length, (length*2) + depth);

	master.push_back(wall1);
	master.push_back(wall2);
	master.push_back(wall3);
	master.push_back(wall4);
	master.push_back(floor);
	master.push_back(ceiling);
	master.push_back(testobj);
	master.push_back(testobj2);
}