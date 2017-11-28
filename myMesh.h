#pragma once
#include "myFace.h"
#include "myHalfedge.h"
#include "myVertex.h"
#include "mySegment.h"
#include <vector>
#include <string>
#include <tuple>

class myMesh
{
public:
	std::vector<myVertex *> vertices;
	std::vector<myHalfedge *> halfedges;
	std::vector<myFace *> faces;
	std::string name;

	bool checkMesh();
	bool _checkhalfedges_twins(std::vector<myHalfedge *> & HE) const;
	bool _checkhalfedges_twins();
	bool _checkhalfedges_nextprev(std::vector<myHalfedge *> & HE) const;
	bool _checkhalfedges_nextprev();
	bool _checkhalfedges_source(std::vector<myHalfedge *> & HE) const;
	bool _checkhalfedges_source();
	bool _checkvertices_fans(std::vector<myVertex *> & V) const;
	bool _checkvertices_fans();
	bool _checkfaces_boundaryedges(std::vector<myFace *> & F) const;
	bool _checkfaces_boundaryedges();

	bool readFile(std::string filename);
	void computeNormals();
	void normalize();

	bool writeFile(std::string filename = "noname.obj");
	
	std::vector<glm::vec3> voronoiReconstruction();
	
	void subdivisionCatmullClark_createNewPoints(std::vector<glm::vec3> &, std::vector<glm::vec3> &, std::vector<glm::vec3> &);
	void subdivisionCatmullClark();

	void splitFaceTRIS(myFace *, glm::vec3);
	void splitFace_size6(myFace *, myHalfedge* starting_edge);

	bool splitEdge(myHalfedge *, glm::vec3);
	void splitFaceQUADS(myFace *, glm::vec3);

	void fractalize();

	void computeSilhouette(std::vector<myHalfedge *> & silhouette_edges, glm::vec3 camera_position);
	
	void setIndices();

	void triangulate();
	bool triangulate(myFace *);

	void copy(myMesh *);

	bool intersect(mySegment ray, myFace * & picked_face, float &, glm::mat4 modelview_matrix = glm::mat4(1.0f));
	bool intersect(mySegment ray, myFace * & picked_face, float &, std::tuple<int, int, int, int>  & stats, glm::mat4 modelview_matrix = glm::mat4(1.0f));

	void smoothen(float delta = 0.1f);
	void sharpen(float delta = 0.1f);
	void inflate(float delta = 0.1f);

	void clear();

	myMesh(std::vector<myVertex *> &);
	myMesh();
	~myMesh();
};

