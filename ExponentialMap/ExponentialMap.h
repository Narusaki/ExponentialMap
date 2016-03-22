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

#include <ICH.h>
#include <unordered_map>
#include <queue>

// This class is exported from the ExponentialMap.dll
class EXPONENTIALMAP_API CExponentialMap {
public:

	CExponentialMap();
	~CExponentialMap();
	// TODO: add your methods here.

	void AssignMesh(CMesh *mesh_);
	void BuildExponentialMap(unsigned centerId_, double radius_);

	void OutputWithEmbeddedTexture(const char *fileName);
	void OutputWithExternalTexture(const char *fileName);
	void OutputGeodesicPath(const char *fileName);

private:

	ICH *ich;
	CMesh *mesh;

	std::unordered_map< int, std::pair<double, double> > texMap;
	int centerId;
	double radius;
	unsigned gridNum;
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
