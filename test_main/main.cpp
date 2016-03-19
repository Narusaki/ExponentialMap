#include <iostream>
#include "..\ExponentialMap\ExponentialMap.h"

using namespace std;

int main(int argc, char **argv)
{
	CMesh *mesh = new CMesh();;
	CExponentialMap *expMap = new CExponentialMap();
	expMap->AssignMesh(mesh);
	expMap->BuildExponentialMap(0, DBL_MAX, INT_MAX);
	delete expMap;
	delete mesh;

	return 0;
}