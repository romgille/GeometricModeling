#pragma once
#include <GL/glew.h>

#include "wx/defs.h"
#include "wx/app.h"
#include "wx/menu.h"
#include "wx/dcclient.h"
#include "wx/wfstream.h"
#include "wx/glcanvas.h"
#include "wx/zstream.h"
#include "wx/wx.h"

#include "myShader.h"
#include <vector>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>    

#include "myMesh.h"
#include "myCamera.h"
#include <unordered_map>
#include "myBVH.h"

class myFrameGUI;

class myGLCanvasGUI : public wxGLCanvas
{
public:
	myGLCanvasGUI(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxT("TestGLCanvas"), const int *attributes = nullptr);
	virtual ~myGLCanvasGUI();

//protected:
	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
	void onKeyPress(wxKeyEvent& event);
	void OnMouse(wxMouseEvent& event);


//private:
	void InitGL();
	void updateDisplayBuffers();

	wxGLContext* m_glRC;
	myFrameGUI *main_window;
	
	bool initialized;

	myShader *shaderprogram;
	myCamera *camera;

	int mouse[2];

	myMesh *mesh;
	myBVH *bvh;

	void closestObject(float x, float y, glm::vec3 & p, myVertex * & v, myHalfedge * & e, myFace * & f) const;
	void computeDataStructureInfo(int, int);

	enum { RENDER_BUFFERS, RENDER_DIRECT };

	enum myObjectId { 
		DATASTRUCTURE, SELECTED, CATMULLCLARK_FACEPOINTS, CATMULLCLARK_EDGEPOINTS, CATMULLCLARK_VERTEXPOINTS, 
		VORONOI_POINTS, POLES_POINTS, DELAUNAY_EDGES, FRACTALIZE, BVH,
		_MAXID
	};
	std::unordered_map<myObjectId, std::vector<myVertex *> > vertices_todraw;
	std::unordered_map<myObjectId, std::vector<myHalfedge *> > edges_todraw;
	std::unordered_map<myObjectId, std::vector<myFace *> > faces_todraw;
	std::unordered_map<myObjectId, std::vector<glm::vec3> > points_todraw;
	std::unordered_map<myObjectId, std::vector<mySegment> > segments_todraw;
	std::unordered_map<myObjectId, std::vector<myAABB> > boxes_todraw;

	void draw_faces(std::vector<myFace *> &F, glm::vec4 color) const;
	void draw_edges(std::vector<myHalfedge *> &E, glm::vec4 color, float size = 4.0f) const;
	void draw_segments(std::vector<mySegment> & S, glm::vec4 color, float size = 4.0f) const;
	void draw_vertices(std::vector<myVertex *> &V, glm::vec4 color, float size = 6.0f) const;
	void draw_points(std::vector<glm::vec3> &P, glm::vec4 color, float size = 6.0f) const;
	void draw_boxes(std::vector<myAABB> &B, glm::vec4 color) const;

	void clear_todraw(myObjectId);
	void reset();

	myMesh *reconstructed_mesh;
 
	unsigned int num_triangles;
	
	enum {
		BUFFER_VERTICES = 0, BUFFER_NORMALS_PERFACE, BUFFER_NORMALS_PERVERTEX, BUFFER_VERTICESFORNORMALDRAWING,
		BUFFER_INDICES_TRIANGLES, BUFFER_INDICES_EDGES, BUFFER_INDICES_VERTICES
	};
	std::vector<GLuint> buffers;

	enum { VAO_TRIANGLES_NORMSPERVERTEX = 0, VAO_TRIANGLES_NORMSPERFACE, VAO_EDGES, VAO_VERTICES, VAO_NORMALS };
	std::vector<GLuint> vaos;

	wxDECLARE_NO_COPY_CLASS(myGLCanvasGUI);
	wxDECLARE_EVENT_TABLE();
};




  