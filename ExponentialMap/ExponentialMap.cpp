// ExponentialMap.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ExponentialMap.h"
using namespace std;

// This is an example of an exported variable
// EXPONENTIALMAP_API int nExponentialMap=0;
// 
// This is an example of an exported function.
// EXPONENTIALMAP_API int fnExponentialMap(void)
// {
// 	return 42;
// }

// This is the constructor of a class that has been exported.
// see ExponentialMap.h for the class definition
CExponentialMap::CExponentialMap()
{
	ich = new ICH();
	gridNum = 5;
	return;
}

CExponentialMap::~CExponentialMap()
{
	if (ich) delete ich;
}

void CExponentialMap::AssignMesh(CMesh *mesh_)
{
	mesh = mesh_;
	ich->AssignMesh(mesh);
}

void CExponentialMap::BuildExponentialMap(unsigned centerId_, double radius_)
{
	// Since ICH does not guarantee to calculate the "true" geodesic until the global termination, 
	// if we set the *radius* and *coveredVertNum* directly, the verts on the "margin" will has wrong paths.
	// So here we double the range to guarantee the correctness.
	centerId = centerId_;
	radius = radius_;

	ich->Clear();
	ich->AddSource(centerId);
	ich->SetMaxGeodRadius(radius);

	cout << "Executing ICH..." << endl;
	ich->Execute();
	cout << "ICH is done." << endl;

	cout << "Building exponential map..." << endl;
	Vector3D zAxis = mesh->m_pVertex[centerId].m_vNormal;
	Vector3D xAxis(1.0, 0.0, 0.0);
	xAxis ^= zAxis; xAxis.normalize();
	Vector3D yAxis = zAxis ^ xAxis; yAxis.normalize();
	cout << "Frame at origin is:" << endl;
	cout << xAxis << endl << yAxis << endl << zAxis << endl;

	for (int i = 0; i < mesh->m_nVertex; ++i)
	{
		double dstLen = ich->GetDistanceTo(i);
		if (dstLen == DBL_MAX) continue;

		unsigned srcId = -1;
		auto geodPath = ich->BuildGeodesicPathTo(i, srcId);

		// project tangent direction onto the tangent plane
		Vector3D tDir;
		if (geodPath.empty())
			tDir = mesh->m_pVertex[i].m_vPosition;
		else if (geodPath.back().isVertex)
			tDir = mesh->m_pVertex[geodPath.back().id].m_vPosition;
		else
		{
			Vector3D v0 = mesh->m_pVertex[mesh->m_pEdge[geodPath.back().id].m_iVertex[0]].m_vPosition;
			Vector3D v1 = mesh->m_pVertex[mesh->m_pEdge[geodPath.back().id].m_iVertex[1]].m_vPosition;
			Vector3D univ = v1 - v0; univ.normalize();
			tDir = v0 + univ * geodPath.back().pos;
		}

		tDir = (mesh->m_pVertex[centerId].m_vPosition - tDir) * zAxis * zAxis + tDir;
		tDir -= mesh->m_pVertex[centerId].m_vPosition; tDir.normalize();

		// calculate texture coordinates
		pair<double, double> texCoords;
		texCoords.first = tDir * xAxis * dstLen; texCoords.second = tDir * yAxis * dstLen;
		texMap[i] = texCoords;
	}
	cout << "Exponential map is built." << endl;
}

void CExponentialMap::OutputWithEmbeddedTexture(const char *fileName)
{
	ofstream output(fileName);
	for (int i = 0; i < mesh->m_nVertex; ++i)
	{
		output << "v " << mesh->m_pVertex[i].m_vPosition;
		auto texIter = texMap.find(i);
		if (texIter == texMap.end()) output << " 255 255 255" << endl;
		else
		{
			double x = texIter->second.first / radius;
			double y = texIter->second.second / radius;
			int xGridId = x / (1.0 / gridNum);
			int yGridId = y / (1.0 / gridNum);

			if ((xGridId % 2 + yGridId % 2) % 2) output << " 0 0 0" << endl;
			else output << " 255 255 255" << endl;
		}
	}

	for (int i = 0; i < mesh->m_nFace; ++i)
	{
		output << "f " << mesh->m_pFace[i].m_piVertex[0] + 1 << " "
			<< mesh->m_pFace[i].m_piVertex[1] + 1 << " "
			<< mesh->m_pFace[i].m_piVertex[2] + 1 << endl;
	}
	output.close();
}

void CExponentialMap::OutputWithExternalTexture(const char *fileName)
{
	ofstream output(fileName);

	output << "mtllib texture.mtl" << endl;
	/*output << "usemtl Textured" << endl;*/
	for (int i = 0; i < mesh->m_nVertex; ++i)
		output << "v " << mesh->m_pVertex[i].m_vPosition << endl;;
	
	for (int i = 0; i < mesh->m_nVertex; ++i)
	{
		auto texIter = texMap.find(i);
		if (texIter == texMap.end())
			output << "vt 0.0 0.0" << endl;
		else
		{
			double x = texIter->second.first / radius;
			double y = texIter->second.second / radius;
			output << "vt " << x << " " << y << endl;
		}
	}

	int v[3];
	for (int i = 0; i < mesh->m_nFace; ++i)
	{
		for (int j = 0; j < 3; ++j) v[j] = mesh->m_pFace[i].m_piVertex[j] + 1;
		output << "f";
		for (int j = 0; j < 3; ++j) output << " " << v[j] << "/" << v[j];
		output << endl;
	}
	output.close();
}

void CExponentialMap::OutputGeodesicPath(const char *fileName)
{
	ich->Clear();
	ich->AddSource(centerId);
	ich->SetMaxGeodRadius(radius);

	cout << "Executing ICH..." << endl;
	ich->Execute();
	cout << "ICH is done." << endl;

	ofstream output(fileName);

	for (int i = 0; i < mesh->m_nVertex; ++i)
		output << "v " << mesh->m_pVertex[i].m_vPosition << endl;
	vector<int> pathLen;
	for (auto &texIter : texMap)
	{
		int i = texIter.first;
		unsigned srcId = -1;
		auto geodPath = ich->BuildGeodesicPathTo(i, srcId);
		pathLen.push_back(geodPath.size() + 2);

		// calculate & output 3D positions in the path
		output << "v " << mesh->m_pVertex[i].m_vPosition << endl;
		for (auto iter = geodPath.begin(); iter != geodPath.end(); ++iter)
		{
			Vector3D pos3D;
			if (iter->isVertex)
				pos3D = mesh->m_pVertex[iter->id].m_vPosition;
			else
			{
				Vector3D v0 = mesh->m_pVertex[mesh->m_pEdge[iter->id].m_iVertex[0]].m_vPosition;
				Vector3D v1 = mesh->m_pVertex[mesh->m_pEdge[iter->id].m_iVertex[1]].m_vPosition;
				Vector3D univ = v1 - v0; univ.normalize();
				pos3D = v0 + univ * iter->pos;
			}
			output << "v " << pos3D << endl;
		}
		output << "v " << mesh->m_pVertex[centerId].m_vPosition << endl;
	}

	// output mesh
	for (int i = 0; i < mesh->m_nFace; ++i)
	{
		int v[3];
		for (int j = 0; j < 3; ++j) v[j] = mesh->m_pFace[i].m_piVertex[j];
		output << "f " << v[0] + 1 << " " << v[1] + 1 << " " << v[2] + 1 << endl;
	}
	
	// output paths
	int startVertId = mesh->m_nVertex;
	for (int i = 0; i < pathLen.size(); ++i)
	{
		output << "l";
		for (int j = 0; j < pathLen[i]; ++j)
			output << " " << startVertId + j + 1;
		output << endl;
		startVertId += pathLen[i];
	}
	output.close();
}