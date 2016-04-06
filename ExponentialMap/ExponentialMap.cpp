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

	texMap.clear(); mappedFaces.clear();

	ich->Clear();
	ich->AddSource(centerId);
	ich->SetMaxGeodRadius(radius);

	if (!silentMode)
		cout << "Executing ICH..." << endl;
	ich->Execute();
	if (!silentMode)
		cout << "ICH is done." << endl;

	if (!silentMode)
		cout << "Building exponential map..." << endl;
	Vector3D zAxis = mesh->m_pVertex[centerId].m_vNormal;
	Vector3D xAxis(1.0, 0.0, 0.0);
	xAxis ^= zAxis; xAxis.normalize();
	Vector3D yAxis = zAxis ^ xAxis; yAxis.normalize();
	if (!silentMode)
	{
		cout << "Frame at origin is:" << endl;
		cout << xAxis << endl << yAxis << endl << zAxis << endl;
	}

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
		Vector2D texCoords;
		texCoords[0] = tDir * xAxis * dstLen; texCoords[1] = tDir * yAxis * dstLen;
		texMap[i] = texCoords;
	}

	// store all mapped triangles
	for (int i = 0; i < mesh->m_nFace; ++i)
	{
		bool allVertMapped = true;
		for (int j = 0; j < 3; ++j)
		{
			if (texMap.find(mesh->m_pFace[i].m_piVertex[j]) == texMap.end())
			{
				allVertMapped = false; break;
			}
		}
		if (!allVertMapped) continue;
		mappedFaces.insert(i);
	}
	if (!silentMode)
		cout << "Exponential map is built." << endl;
}

void CExponentialMap::BuildExponentialMap(unsigned faceId_, Vector3D pos_, double radius_)
{
	// basically the same with previous method
	faceId = faceId_;
	centerPos = pos_;
	radius = radius_;

	texMap.clear(); mappedFaces.clear();

	ich->Clear();
	ich->AddSource(faceId, centerPos);
	ich->SetMaxGeodRadius(radius);

	if (!silentMode)
		cout << "Executing ICH..." << endl;
	ich->Execute();
	if (!silentMode)
		cout << "ICH is done." << endl;

	// refine mis-calculated vertices
	for (int i = 0; i < mesh->m_nVertex; ++i)
	{
		auto &vertInfo = ich->GetVertInfo(i);
		if (vertInfo.dist != DBL_MAX) continue;

		for (int j = 0; j < mesh->m_pVertex[i].m_nValence; ++j)
		{
			double eL = mesh->m_pEdge[mesh->m_pVertex[i].m_piEdge[j]].m_length;
			const auto &adjVertInfo = ich->GetVertInfo(mesh->m_pEdge[mesh->m_pVertex[i].m_piEdge[j]].m_iVertex[1]);

			if (adjVertInfo.dist + eL >= vertInfo.dist) continue;

			vertInfo.birthTime = adjVertInfo.birthTime;
			vertInfo.dist = adjVertInfo.dist + eL;
			vertInfo.isSource = false;
			vertInfo.enterEdge = mesh->m_pVertex[i].m_piEdge[j];
			vertInfo.pseudoSrcId = mesh->m_pEdge[mesh->m_pVertex[i].m_piEdge[j]].m_iVertex[1];
			vertInfo.srcId = adjVertInfo.srcId;
		}
	}
	if (!silentMode)
		cout << "Building exponential map..." << endl;
	Vector3D zAxis = mesh->m_pFace[faceId].m_vNormal;
	Vector3D xAxis(1.0, 0.0, 0.0);
	xAxis ^= zAxis; xAxis.normalize();
	Vector3D yAxis = zAxis ^ xAxis; yAxis.normalize();
	if (!silentMode)
	{
		cout << "Frame at origin is:" << endl;
		cout << xAxis << endl << yAxis << endl << zAxis << endl;
	}

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

		tDir = (centerPos - tDir) * zAxis * zAxis + tDir;
		tDir -= centerPos; tDir.normalize();

		// calculate texture coordinates
		Vector2D texCoords;
		texCoords[0] = tDir * xAxis * dstLen; texCoords[1] = tDir * yAxis * dstLen;
		texMap[i] = texCoords;
	}

	// store all mapped triangles
	for (int i = 0; i < mesh->m_nFace; ++i)
	{
		bool allVertMapped = true;
		for (int j = 0; j < 3; ++j)
		{
			if (texMap.find(mesh->m_pFace[i].m_piVertex[j]) == texMap.end())
			{
				allVertMapped = false; break;
			}
		}
		if (!allVertMapped) continue;
		mappedFaces.insert(i);
	}
	if (!silentMode)
		cout << "Exponential map is built." << endl;
}

pair<unsigned, Vector3D> CExponentialMap::ExpMap(Vector2D pos2D)
{
	// TODO: compute expmap of pos
	unsigned fId = -1; Vector3D pos3D;
	double minArea = DBL_MAX;
	for (auto faceIter : mappedFaces)
	{
		Vector2D p[3];
		for (int i = 0; i < 3; ++i)
			p[i] = texMap[mesh->m_pFace[faceIter].m_piVertex[i]];
		double curArea = fabs((p[1] - p[0]) ^ (p[2] - p[0])) / 2.0;
		if (curArea > minArea) continue;
		Vector3D baryCentricCoord = calcBarycentricCoord(pos2D, p[0], p[1], p[2]);
		if (baryCentricCoord[0] < 0.0 || baryCentricCoord[1] < 0.0 || baryCentricCoord[2] < 0.0)
			continue;
		// since for the 2D case, a point is constrained on the plane, 
		// we can terminate the searching once we find a face contain it, 
		fId = faceIter;
		pos3D.x = 0.0; pos3D.y = 0.0; pos3D.z = 0.0;
		for (int i = 0; i < 3; ++i)
			pos3D += baryCentricCoord[i] * mesh->m_pVertex[mesh->m_pFace[faceIter].m_piVertex[i]].m_vPosition;
		minArea = curArea;
	}
	return make_pair(fId, pos3D);
}

Vector2D CExponentialMap::InvExpMap(unsigned fId, Vector3D pos)
{
	// TODO: compute expmap^-1 of pos in face fId
	if (mappedFaces.find(fId) == mappedFaces.end())
	{
		cout << "Face " << fId << " is not in the mapped faces set!";
		return Vector2D();
	}
	Vector3D p[3];
	for (int i = 0; i < 3; ++i) p[i] = mesh->m_pVertex[mesh->m_pFace[fId].m_piVertex[i]].m_vPosition;
	Vector3D baryCentricCoord = calcBarycentricCoord(pos, p[0], p[1], p[2]);

	Vector2D texCoord;
	for (int i = 0; i < 3; ++i)
		texCoord += baryCentricCoord[i] * texMap[mesh->m_pFace[fId].m_piVertex[i]];
	return texCoord;
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
			double x = texIter->second[0] / radius;
			double y = texIter->second[1] / radius;
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
			double x = texIter->second[0] / radius;
			double y = texIter->second[1] / radius;
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
	if (centerId != -1)
		ich->AddSource(centerId);
	else
		ich->AddSource(faceId, centerPos);
	ich->SetMaxGeodRadius(radius);

	cout << "Executing ICH..." << endl;
	ich->Execute();
	cout << "ICH is done." << endl;

	// refine mis-calculated vertices
	for (int i = 0; i < mesh->m_nVertex; ++i)
	{
		auto &vertInfo = ich->GetVertInfo(i);
		if (vertInfo.dist != DBL_MAX) continue;

		for (int j = 0; j < mesh->m_pVertex[i].m_nValence; ++j)
		{
			double eL = mesh->m_pEdge[mesh->m_pVertex[i].m_piEdge[j]].m_length;
			const auto &adjVertInfo = ich->GetVertInfo(mesh->m_pEdge[mesh->m_pVertex[i].m_piEdge[j]].m_iVertex[1]);

			if (adjVertInfo.dist + eL >= vertInfo.dist) continue;

			vertInfo.birthTime = adjVertInfo.birthTime;
			vertInfo.dist = adjVertInfo.dist + eL;
			vertInfo.isSource = false;
			vertInfo.enterEdge = mesh->m_pVertex[i].m_piEdge[j];
			vertInfo.pseudoSrcId = mesh->m_pEdge[mesh->m_pVertex[i].m_piEdge[j]].m_iVertex[1];
			vertInfo.srcId = adjVertInfo.srcId;
		}
	}

	int cnt = 0;
	for (int i = 0; i < mesh->m_nVertex; ++i)
		if (ich->GetDistanceTo(i) != DBL_MAX)  ++cnt;

	ofstream output(fileName);

	for (int i = 0; i < mesh->m_nVertex; ++i)
		output << "v " << mesh->m_pVertex[i].m_vPosition << endl;
	vector<int> pathLen;
	for (auto &texIter : texMap)
	{
		int i = texIter.first;
		unsigned srcId = -1;
		auto geodPath = ich->BuildGeodesicPathTo(i, srcId);
		int geodPathSize = geodPath.size();
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
		output << "v " << (centerId != -1 ? mesh->m_pVertex[centerId].m_vPosition : centerPos) << endl;
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

Vector3D CExponentialMap::calcBarycentricCoord(const Vector3D& p,
	const Vector3D& p0, const Vector3D& p1, const Vector3D& p2)
{
	Vector3D v0 = p0 - p, v1 = p1 - p, v2 = p2 - p;
	Vector3D n0 = v0 ^ v1, n1 = v1 ^ v2, n2 = v2 ^ v0;
	double a0 = n0.normalize();
	double a1 = n1.normalize() * (n1 * n0 > 0.0 ? 1.0 : -1.0);
	double a2 = n2.normalize() * (n2 * n0 > 0.0 ? 1.0 : -1.0);

	Vector3D coord;
	coord[0] = a1 / (a0 + a1 + a2);
	coord[1] = a2 / (a0 + a1 + a2);
	coord[2] = a0 / (a0 + a1 + a2);
	return coord;
}

Vector3D CExponentialMap::calcBarycentricCoord(const Vector2D& p,
	const Vector2D& p0, const Vector2D& p1, const Vector2D& p2)
{
	Vector2D v0 = p0 - p, v1 = p1 - p, v2 = p2 - p;
	double a0 = v0 ^ v1, a1 = v1 ^ v2, a2 = v2 ^ v0;

	Vector3D coord;
	coord[0] = a1 / (a0 + a1 + a2);
	coord[1] = a2 / (a0 + a1 + a2);
	coord[2] = a0 / (a0 + a1 + a2);
	return coord;
}