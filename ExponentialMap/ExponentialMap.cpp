// ExponentialMap.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ExponentialMap.h"


// This is an example of an exported variable
EXPONENTIALMAP_API int nExponentialMap=0;

// This is an example of an exported function.
EXPONENTIALMAP_API int fnExponentialMap(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see ExponentialMap.h for the class definition
CExponentialMap::CExponentialMap()
{
	ich = new ICH();
	return;
}

CExponentialMap::~CExponentialMap()
{
	if (ich) delete ich;
}

void CExponentialMap::AssignMesh(CMesh *mesh_)
{
	mesh = mesh_;
}

void CExponentialMap::BuildExponentialMap(unsigned centerId, double radius, unsigned covertedVertNum)
{

}