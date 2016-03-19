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

// This class is exported from the ExponentialMap.dll
class EXPONENTIALMAP_API CExponentialMap {
public:

	CExponentialMap();
	~CExponentialMap();
	// TODO: add your methods here.

	void AssignMesh(CMesh *mesh_);
	void BuildExponentialMap(unsigned centerId, double radius, unsigned covertedVertNum);

private:
	ICH *ich;
	CMesh *mesh;

	std::unordered_map< int, std::pair<double, double> > texMap;
};

extern EXPONENTIALMAP_API int nExponentialMap;

EXPONENTIALMAP_API int fnExponentialMap(void);
