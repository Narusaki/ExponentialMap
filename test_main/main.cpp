#include <iostream>
#include "..\ExponentialMap\ExponentialMap.h"

/*#define VERTEX_ORIGIN*/
#define ARBITRARY_ORIGIN

using namespace std;

#ifdef VERTEX_ORIGIN
int main(int argc, char **argv)
{
	if (argc < 4)
	{
		cout << "USAGE: [.exe] [in.obj] [srcId] [radius]" << endl;
		return -1;
	}
	CMesh *mesh = new CMesh();
	if (!mesh->Load(argv[1]))
	{
		cout << "Cannot load mesh " << argv[1] << endl;
		return -2;
	}
	int centerId = stoi(argv[2]);
	double radius = stod(argv[3]);
	CExponentialMap *expMap = new CExponentialMap();
	expMap->AssignMesh(mesh);
	expMap->BuildExponentialMap(centerId, radius);

	string embeddedFile = argv[1];
	embeddedFile = embeddedFile.substr(0, embeddedFile.rfind(".")) + ".center" + argv[2] + ".embedded.obj";

// 	cout << "Outputing embedded mapped mesh..." << endl;
// 	expMap->OutputWithEmbeddedTexture(embeddedFile.c_str());
// 	cout << "Outputing done." << endl;

	string externalFile = argv[1];
	externalFile = externalFile.substr(0, externalFile.rfind(".")) + ".center" + argv[2] + ".external.obj";

	cout << "Outputing external mapped mesh..." << endl;
	expMap->OutputWithExternalTexture(externalFile.c_str());
	cout << "Outputing done." << endl;

	string pathFile = argv[1];
	pathFile = pathFile.substr(0, pathFile.rfind(".")) + ".center" + argv[2] + ".paths.obj";
	cout << "Outputing geodesic paths..." << endl;
	expMap->OutputGeodesicPath(pathFile.c_str());
	cout << "Output done." << endl;

	delete expMap;
	delete mesh;

	return 0;
}
#endif

#ifdef ARBITRARY_ORIGIN
int main(int argc, char **argv)
{
	if (argc < 7)
	{
		cout << "USAGE: [.exe] [in.obj] [faceId] [posX] [posY] [posZ] [radius]" << endl;
		return -1;
	}
	CMesh *mesh = new CMesh();
	if (!mesh->Load(argv[1]))
	{
		cout << "Cannot load mesh " << argv[1] << endl;
		return -2;
	}
	int faceId = stoi(argv[2]);
	Vector3D centerPos(stod(argv[3]), stod(argv[4]), stod(argv[5]));
	double radius = stod(argv[6]);
	CExponentialMap *expMap = new CExponentialMap();
	expMap->AssignMesh(mesh);
	expMap->BuildExponentialMap(faceId, centerPos, radius);

	string embeddedFile = argv[1];
	embeddedFile = embeddedFile.substr(0, embeddedFile.rfind(".")) + ".arbcenter" + argv[2] + ".embedded.obj";

	// 	cout << "Outputing embedded mapped mesh..." << endl;
	// 	expMap->OutputWithEmbeddedTexture(embeddedFile.c_str());
	// 	cout << "Outputing done." << endl;

	string externalFile = argv[1];
	externalFile = externalFile.substr(0, externalFile.rfind(".")) + ".arbcenter" + argv[2] + ".external.obj";

	cout << "Outputing external mapped mesh..." << endl;
	expMap->OutputWithExternalTexture(externalFile.c_str());
	cout << "Outputing done." << endl;

	string pathFile = argv[1];
	pathFile = pathFile.substr(0, pathFile.rfind(".")) + ".arbcenter" + argv[2] + ".paths.obj";
	cout << "Outputing geodesic paths..." << endl;
	expMap->OutputGeodesicPath(pathFile.c_str());
	cout << "Output done." << endl;

	delete expMap;
	delete mesh;

	return 0;
}
#endif