#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include <glm/gtc/type_ptr.hpp>    
#include "helper_functions.h"
#include <wx/math.h>
#include <algorithm>
#include "errors.h"
#include "default_constants.h"


using namespace std;

myMesh::myMesh()
{
}


myMesh::~myMesh()
{
	clear();
}

void myMesh::clear()
{
	for (size_t i = 0; i < vertices.size(); i++) if (vertices[i]) delete vertices[i];
	for (size_t i = 0; i < halfedges.size(); i++) if (halfedges[i]) delete halfedges[i];
	for (size_t i = 0; i < faces.size(); i++) if (faces[i]) delete faces[i];

	vertices.clear();
	halfedges.clear();
	faces.clear();
}

myMesh::myMesh(std::vector<myVertex*> & in_vertices)
{
	for (size_t i = 0;i < in_vertices.size();i++)
		vertices.push_back(new myVertex(in_vertices[i]->point));
}

bool myMesh::checkMesh()
{
	cout << "Checking all mesh errors.\n";

	bool result = _checkfaces_boundaryedges() | _checkhalfedges_nextprev() | _checkhalfedges_source() | _checkhalfedges_twins() | _checkvertices_fans();

	cout << "Finished checking.\n";

	return result;
}


bool myMesh::_checkhalfedges_twins( )
{
	return _checkhalfedges_twins(halfedges);
}

bool myMesh::_checkhalfedges_twins(vector<myHalfedge *> & HE) const
{
	bool error = true;
	bool skip_further_errors = false;

	cout << "\tChecking the halfedge twin variables: " << HE.size() <<  endl;
	for (size_t i = 0; i < HE.size(); i++)
	{
		myassert(HE[i] != nullptr);

		if (HE[i]->twin == nullptr)
		{
			if (!skip_further_errors)
				cout << "\t\tWarning, boundary edges or otherwise nullptr twins.\n";
			skip_further_errors = true;
		}
		else
			myassert(HE[i] == HE[i]->twin->twin);
	}
	cout << "\tEnded check.\n\n";

	return error;
}

bool myMesh::_checkhalfedges_nextprev()
{
	return _checkhalfedges_nextprev(halfedges);
}

bool myMesh::_checkhalfedges_nextprev(vector<myHalfedge *> & HE) const
{
	bool error = true;

	cout << "\tChecking the halfedge next/prev variables: " << HE.size() << endl;
	for (size_t i = 0; i < HE.size(); i++)
	{
		myassert(HE[i] != nullptr);
		myassert(HE[i]->next != nullptr);
		myassert(HE[i]->prev != nullptr);
		myassert(HE[i]->next->prev == HE[i]);
		myassert(HE[i]->prev->next == HE[i]);
	}
	cout << "\tEnded check.\n\n";

	return error;
}


bool myMesh::_checkhalfedges_source()
{
	return _checkhalfedges_source(halfedges);
}
	
bool myMesh::_checkhalfedges_source(vector<myHalfedge *> & HE) const
{
	bool error = true;

	cout << "\tChecking the halfedge source variables: " << HE.size()  << endl;
	for (size_t i = 0; i < HE.size(); i++)
	{
		myassert(HE[i] != nullptr);
		myassert(HE[i]->source != nullptr);
	}
	cout << "\tEnded check.\n\n";

	return error;
}


bool myMesh::_checkvertices_fans()
{
	return _checkvertices_fans(vertices);
}

bool myMesh::_checkvertices_fans(vector<myVertex *> & V) const
{
	bool error = true;
	
	map<myVertex *, size_t> vertex_outdegrees;

	for (size_t i = 0;i < V.size();++i)
		vertex_outdegrees[V[i]] = 0;

	for (size_t i = 0;i < halfedges.size();++i)
		if (vertex_outdegrees.count(halfedges[i]->source))
			vertex_outdegrees[halfedges[i]->source]++;

	cout << "\tChecking fans of each vertex: " << V.size() << endl;
	for (size_t i = 0; i<V.size(); i++)
	{
		myassert(V[i] != nullptr);
		myassert(V[i]->originof != nullptr);

		myHalfedge *e = V[i]->originof;

		size_t k = 0;
		do 
		{
			myassert(e->prev != nullptr);
			e = e->prev->twin;

			k++;
			myassert(k <= MAX_VERTEX_DEGREE);
		} while (e != nullptr && e != V[i]->originof);
		myassert(k == vertex_outdegrees[V[i]]);
	}
	cout << "\tEnded check.\n\n";

	return error;
}

bool myMesh::_checkfaces_boundaryedges()
{
	return _checkfaces_boundaryedges(faces);
}

bool myMesh::_checkfaces_boundaryedges(vector<myFace *> & F) const
{
	bool error = true;
	bool istriangular = true;

	cout << "\tChecking edges of each face: " << F.size() << endl;

	size_t num_incidentedgesoverallfaces = 0;
	for (size_t i = 0; i<F.size(); i++)
	{
		myassert(F[i] != nullptr);

		myHalfedge *e = F[i]->adjacent_halfedge;
		size_t k = 0;
		do 
		{
			myassert(e != nullptr);
			e = e->next;

			k++;
			myassert(k <= MAX_FACE_DEGREE);
		} while (e != F[i]->adjacent_halfedge);

		num_incidentedgesoverallfaces += k;
		if (k>3) istriangular = false;
	}
	cout << "\tAverage number of edges per face: " << static_cast<float>(num_incidentedgesoverallfaces) / faces.size() << endl;
	if (istriangular) cout << "\t\tThe mesh is triangular.\n";
	else cout << "\t\tThe mesh is not triangular.\n";
	
	cout << "\tEnded check.\n\n";

	return error;
}


bool myMesh::readFile(std::string filename)
{
	string s, t, u;
	map<pair<int, int>, myHalfedge *> hemap;

	ifstream fin(filename);
	if (!fin.is_open()) 
	{
		PRINT(ERROR_FILEOPEN);
		return false;
	}
	name = filename;

	while (getline(fin, s))
	{
		stringstream myline(s);
		myline >> t;
		if (t == "v")
		{
			myVertex *v = new myVertex();

			myline >> u;
			v->point.x = static_cast<float>(atof((u.substr(0, u.find("/"))).c_str()));

			myline >> u;
			v->point.y = static_cast<float>(atof((u.substr(0, u.find("/"))).c_str()));

			myline >> u;
			v->point.z = static_cast<float>(atof((u.substr(0, u.find("/"))).c_str()));

			vertices.push_back(v);
		}
		else if (t == "f")
		{
			vector<size_t> faceids;
			vector<myHalfedge *> H;

			while (myline >> u) {
				faceids.push_back(atoi((u.substr(0, u.find("/"))).c_str()) - 1);
				H.push_back(new myHalfedge());
			}

			myFace *face = new myFace();

			int halfedgesSize = H.size();
			for (size_t i = 0; i < halfedgesSize; ++i)
			{
				int ipo = (i + 1) % halfedgesSize;
				int imo = (i - 1 + halfedgesSize) % halfedgesSize;
				H[i]->adjacent_face = face;
				H[i]->next = H[ipo];
				H[i]->prev = H[imo];
				H[i]->twin = nullptr;
				H[i]->source = vertices[faceids[i]];

				pair<int, int> a = make_pair(faceids[ipo], faceids[i]);
				if (hemap.find(a) == hemap.end()) {
					hemap[make_pair(faceids[i], faceids[ipo])] = H[i];
				}
				else {
					H[i]->twin = hemap[a];
					hemap[a]->twin = H[i];
				}

				halfedges.push_back(H[i]);
				vertices[faceids[i]]->originof = H[i];
			}

			face->adjacent_halfedge = H[0];
			face->normal = glm::vec3(1.0f, 1.0f, 1.0f);
			faces.push_back(face);
		}
	}
	fin.close();

	//vector<myVertex *> verts(3);
	//vector<myHalfedge *> hes(3);
	//myFace *f = new myFace();
	//
	//verts[0] = new myVertex(glm::vec3(1.0f, 0.0f, 0.0f));
	//verts[1] = new myVertex(glm::vec3(0.0f, 1.0f, 0.0f));
	//verts[2] = new myVertex(glm::vec3(0.0f, 0.0f, 1.0f));
	//for (int i = 0;i < 3;i++) hes[i] = new myHalfedge();
	//
	//for (int i=0;i<3;i++)
	//{
	//	int ipo = (i + 1) % 3;
	//	int imo = (i - 1 + 3) % 3;
	//	hes[i]->next = hes[ipo];
	//	hes[i]->prev = hes[imo];
	//	hes[i]->twin = nullptr;
	//	hes[i]->adjacent_face = f;
	//	hes[i]->source = verts[i];
	//
	//	verts[i]->originof = hes[i];
	//	verts[i]->normal = glm::vec3(1.0f, 1.0f, 1.0f);
	//
	//	vertices.push_back(verts[i]);
	//	halfedges.push_back(hes[i]);
	//}
	//f->adjacent_halfedge = hes[0];
	//f->normal = glm::vec3(1.0f, 1.0f, 1.0f);
	//faces.push_back(f);

	checkMesh();
	
	computeNormals();

	normalize();

	return true;
}

void myMesh::computeNormals()
{
	for (vector<myFace *>::iterator it = faces.begin(); it != faces.end(); ++it)
		(*it)->computeNormal();

	for (vector<myVertex *>::iterator it = vertices.begin(); it != vertices.end(); ++it)
		(*it)->computeNormal();
}

void myMesh::normalize()
{
	if (vertices.size() == 0) return;

	enum { MIN, MAX };
	vector<glm::vec3> corner = { vertices[0]->point, vertices[0]->point };

	for (size_t i = 1; i < vertices.size(); i++) 
	{
		for (int coordinate = 0;coordinate < 3; coordinate++)
		{
			corner[MIN][coordinate] = std::min( corner[MIN][coordinate], vertices[i]->point[coordinate] );
			corner[MAX][coordinate] = std::max( corner[MAX][coordinate], vertices[i]->point[coordinate] );
		}
	}
	
	float scale_factor = std::max({ corner[MAX][0] - corner[MIN][0], corner[MAX][1] - corner[MIN][1], corner[MAX][2] - corner[MIN][2] } );

	for (size_t i = 0; i < vertices.size(); i++) 
	{
		for (int coordinate = 0;coordinate < 3; coordinate++)
			vertices[i]->point[coordinate] -= (corner[MAX][coordinate] + corner[MIN][coordinate]) / 2.0f;
		vertices[i]->point /= scale_factor;
	}
}

bool myMesh::writeFile(std::string filename)
{
	return false;
}


vector<glm::vec3> myMesh::voronoiReconstruction( )
{
	return vector<glm::vec3>();
} 

void myMesh::splitFaceTRIS(myFace *f, glm::vec3 p)
{
	myassert(f != nullptr);

}


bool myMesh::splitEdge(myHalfedge *e1, glm::vec3 p)
{
	myassert(e1 != nullptr);

	return false;
}

void myMesh::splitFaceQUADS(myFace *f, glm::vec3 p)
{
	myassert(f != nullptr);
}

void myMesh::splitFace_size6(myFace *f, myHalfedge *starting_edge)
{
	if (f->size() != 6) return;

}



void myMesh::fractalize()
{
}


void myMesh::subdivisionCatmullClark_createNewPoints(std::vector<glm::vec3> & facepoints,
													 std::vector<glm::vec3> & edgepoints,
													 std::vector<glm::vec3> & newvertexpoints)
{
}

void myMesh::subdivisionCatmullClark()
{
}

void myMesh::setIndices()
{
	for (size_t i = 0; i < faces.size(); i++) 
		faces[i]->index = i;
	for (size_t i = 0; i < halfedges.size(); i++) 
		halfedges[i]->index = i;
	for (size_t i = 0; i < vertices.size(); i++)	
		vertices[i]->index = i;
}

void myMesh::triangulate()
{
	for (size_t i = 0; i < faces.size(); i++)
		triangulate(faces[i]);
}

//return false if already triangle, true othewise.
//new edge and face array stores an extra index  for shorter code.
bool myMesh::triangulate(myFace *f)
{
	myassert(f != nullptr);

	cout << "before " << f->size() << endl;

	if (f->size() <= 3) return false;
 
	cout << "after" << f->size() << endl;


	unsigned int n = f->size();

	vector<myHalfedge *> IN (n-1);
	vector<myHalfedge *> OUT(n - 2);
	vector<myFace *> F(n - 2);
	vector<myHalfedge *> H;
	vector<myVertex *> V;

	for (int i = 1;i < n - 2; i++) {
		F[i] = new myFace(); 
		faces.push_back(F[i]);
	}
	for (int i = 1;i < n - 2; i++) {
		IN[i] = new myHalfedge(); 
		halfedges.push_back(IN[i]);
	}
	for (int i = 1;i < n - 2; i++) {
		OUT[i] = new myHalfedge(); 
		halfedges.push_back(OUT[i]);
	}

	H.clear();
	V.clear();
	myHalfedge *actual_halfedge = f->adjacent_halfedge;
	do {
		H.push_back(actual_halfedge);
		V.push_back(actual_halfedge->source);
		actual_halfedge = actual_halfedge->next;
	} while (actual_halfedge != f->adjacent_halfedge);

	//IN[0] will not be used.
	OUT[0] = H[0];
	IN[n-2] = H[n-2];
	F[0] = f;

	//from now onwards, not use the variable f.
	for (int i = 1; i < n - 2; ++i) {
		int ipo = (i + 1) ;
		int imo = (i - 1 );

		IN[i]->source = V[ipo];
		IN[i]->adjacent_face = F[imo];
		IN[i]->next = OUT[imo];
		IN[i]->prev = H[i];
		IN[i]->twin = OUT[i];

		OUT[i]->source = V[0];
		OUT[i]->adjacent_face = F[i];
		OUT[i]->next = H[ipo];
		OUT[i]->prev = IN[ipo];
		OUT[i]->twin = IN[i];
	}
	OUT[n - 3]->prev = H[n - 1];

	for (int i = 0; i < n - 2; ++i)  
		F[i]->adjacent_halfedge = IN[i+1];
	 

	for (int i = 1;i < n-1;i++)
	{
		H[i]->next = IN[i];
		H[i]->prev = OUT[i - 1];
		H[i]->adjacent_face = F[i - 1];
	}

	H[0]->adjacent_face = F[0];
	H[0]->next = H[1];
	H[0]->prev = IN[1];
	H[0]->source = V[0];

	H[n - 2]->next = H[n - 1];

	H[n - 1]->adjacent_face = F[n - 3];
	H[n - 1]->next = OUT[n - 3];
	H[n - 1]->prev = H[n - 2];
		
	return true;
}


void myMesh::copy(myMesh *m)
{
	for (size_t i = 0;i < m->vertices.size();i++)
	{
		m->vertices[i]->index = i;
		vertices.push_back(new myVertex());
	}

	for (size_t i = 0; i < m->halfedges.size();i++)
	{
		m->halfedges[i]->index = i;
		halfedges.push_back(new myHalfedge());
	}

	for (size_t i = 0;i < m->faces.size();i++)
	{
		m->faces[i]->index = i;
		faces.push_back(new myFace());
	}

	for (size_t i = 0;i < m->vertices.size(); i++)
	{
		vertices[i]->point = glm::vec3(m->vertices[i]->point);
		vertices[i]->normal = glm::vec3(m->vertices[i]->normal);
		vertices[i]->originof = halfedges[m->vertices[i]->originof->index];
	}

	for (size_t i = 0; i < halfedges.size();i++)
	{
		halfedges[i]->source = vertices[m->halfedges[i]->source->index];
		halfedges[i]->adjacent_face = faces[m->halfedges[i]->adjacent_face->index];
		halfedges[i]->next = halfedges[m->halfedges[i]->next->index];
		halfedges[i]->prev = halfedges[m->halfedges[i]->prev->index];
		if ( m->halfedges[i]->twin != nullptr && m->halfedges[i]->twin->index < m->halfedges.size() ) 
			halfedges[i]->twin = halfedges[m->halfedges[i]->twin->index];
	}

	for (size_t i = 0;i < m->faces.size();i++)
	{
		faces[i]->normal = glm::vec3(faces[i]->normal);
		faces[i]->adjacent_halfedge = halfedges[m->faces[i]->adjacent_halfedge->index];
	}
}

bool myMesh::intersect(mySegment ray, myFace * & picked_face, float & min_t, glm::mat4 model_matrix)
{
	std::tuple<int, int, int, int> tmp_stats;
	return intersect(ray, picked_face, min_t, tmp_stats, model_matrix);
}

bool myMesh::intersect(mySegment ray, myFace * & picked_face, float & min_t, std::tuple<int, int, int, int> & stats, glm::mat4 model_matrix)
{
	bool result = false;

	get<0>(stats) = 0;
	get<1>(stats) = 0;
	get<2>(stats) = 0;

	picked_face = nullptr;

	min_t = std::numeric_limits<float>::max();
	float tmp;

	for (size_t i = 0;i < faces.size();i++)
	{
		myassert(faces[i] != nullptr);
		if (faces[i]->intersect(ray, tmp, model_matrix) && tmp < min_t)
		{
			min_t = tmp;
			picked_face = faces[i];
			result = true;
			get<1>(stats)++;
		}
		get<0>(stats)++;
	}
	return result;
}

void myMesh::smoothen(float delta)
{
}

void myMesh::sharpen(float delta)
{
}

void myMesh::inflate(float delta)
{
}

void myMesh::computeSilhouette(std::vector<myHalfedge *>& silhouette_edges, glm::vec3 camera_position)
{
	
}
