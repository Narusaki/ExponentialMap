// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the EXPONENTIALMAP_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// EXPONENTIALMAP_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef EXPONENTIALMAP_EXPORTS
#define EXPONENTIALMAP_API __declspec(dllexport)
#else
#define EXPONENTIALMAP_API __declspec(dllimport)
#endif

#ifndef EXPONENTIALMAP_H
#define EXPONENTIALMAP_H

#include <ICH.h>
#include <unordered_map>
#include <unordered_set>
/*#include <queue>*/

// This class is exported from the ExponentialMap.dll
class EXPONENTIALMAP_API CExponentialMap {
public:

	CExponentialMap();
	~CExponentialMap();
	// TODO: add your methods here.

	void AssignMesh(CMesh *mesh_);
	void BuildExponentialMap(unsigned centerId_, double radius_);
	void BuildExponentialMap(unsigned faceId_, Vector3D pos_, double radius_);

	std::pair<unsigned, Vector3D> ExpMap(Vector2D pos2D);
	Vector2D InvExpMap(unsigned fId, Vector3D pos);

	void OutputWithEmbeddedTexture(const char *fileName);
	void OutputWithExternalTexture(const char *fileName);
	void OutputGeodesicPath(const char *fileName);

private:
	Vector3D calcBarycentricCoord(const Vector3D& p,
		const Vector3D& p0, const Vector3D& p1, const Vector3D& p2);
	Vector3D calcBarycentricCoord(const Vector2D& p,
		const Vector2D& p0, const Vector2D& p1, const Vector2D& p2);
private:

	ICH *ich;
	CMesh *mesh;

	std::unordered_map< int, Vector2D > texMap;
	std::unordered_set< int > mappedFaces;
	int centerId = -1;
	int faceId = -1;
	Vector3D centerPos;
	double radius;
	unsigned gridNum;

public:
	bool silentMode = false;
};

// typedef std::pair<int, double> VertDistPair;
// 
// class VertDistPairComp
// {
// public:
// 	bool operator() (const VertDistPair& a, const VertDistPair& b) const { return a.second < b.second; }
// };
// 
// typedef std::priority_queue< VertDistPair, std::vector<VertDistPair>, VertDistPairComp> FilterNearestVertPQ;
// 
// extern EXPONENTIALMAP_API int nExponentialMap;
// 
// EXPONENTIALMAP_API int fnExponentialMap(void);

#endif