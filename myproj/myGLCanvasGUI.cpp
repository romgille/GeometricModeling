#include "myGLCanvasGUI.h"

#include "wx/wx.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "default_constants.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>    

#include "myFace.h"

#include "myFrameGUI.h"
#include "errors.h"
#include <wx/confbase.h>


using namespace std;


wxBEGIN_EVENT_TABLE(myGLCanvasGUI, wxGLCanvas)
EVT_SIZE(myGLCanvasGUI::OnSize)
EVT_PAINT(myGLCanvasGUI::OnPaint)
EVT_ERASE_BACKGROUND(myGLCanvasGUI::OnEraseBackground)
EVT_MOUSE_EVENTS(myGLCanvasGUI::OnMouse)
EVT_KEY_DOWN(myGLCanvasGUI::onKeyPress)
wxEND_EVENT_TABLE()


myGLCanvasGUI::myGLCanvasGUI(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name, const int *attributes)
	: wxGLCanvas(parent, id, attributes, pos, size, style | wxFULL_REPAINT_ON_RESIZE | wxTRANSPARENT_WINDOW, name)
{
	m_glRC = new wxGLContext(this);
	initialized = false;

	reconstructed_mesh = nullptr;
	mesh = nullptr;
	bvh = nullptr;


	buffers.assign(NUM_BUFFERS, 0);
	vaos.assign(NUM_BUFFERS, 0);

	num_triangles = 0;
}

myGLCanvasGUI::~myGLCanvasGUI()
{
	delete m_glRC;
	if (shaderprogram) delete shaderprogram;
	if (reconstructed_mesh) delete reconstructed_mesh;
	if (mesh) delete mesh;
}

void myGLCanvasGUI::OnSize(wxSizeEvent& WXUNUSED(event))
{
	SetCurrent(*m_glRC);

	int w, h;
	GetClientSize(&w, &h);

	camera->setWindowSize(w, h);
	Refresh(false);
}

void myGLCanvasGUI::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
	// Do nothing, to avoid flashing on windows.
}


void myGLCanvasGUI::onKeyPress(wxKeyEvent& event)
{
	cout << "key pressed in canvas\n";
}

void myGLCanvasGUI::closestObject(float x, float y, glm::vec3 & p, myVertex * & v, myHalfedge * & e, myFace * & f) const
{
	mySegment ray = camera->constructRay(x, y);
	ray.p2 = ray.p1 + 100.0f * (ray.p2 - ray.p1);

	f = nullptr;
	e = nullptr;
	v = nullptr; 

	float min_t;

	//0: num faces intersection tested, 1: num faces intersected and updated, 2: number of boxes tested.
	std::tuple<int, int, int, int> stats;

	if (bvh != nullptr)
		bvh->intersect(ray, f, min_t, stats);
	else
		mesh->intersect(ray, f, min_t, stats);

	if (f != nullptr)
	{
		p = ray.p1 + min_t * (ray.p2 - ray.p1);
		f->closestVertexEdge(p, v, e);
	}

	main_window->updateIntersectionStatistics(stats);
}




void myGLCanvasGUI::computeDataStructureInfo(int mouse_x, int mouse_y)
{
	clear_todraw(DATASTRUCTURE);
	
	myFace *f; myHalfedge *e; myVertex *v; glm::vec3 p;
	closestObject(mouse_x, mouse_y, p, v, e, f);

	if (f)
	{
		mySegment s(e->source->point, e->next->source->point);
		float distance_squared_to_e = s.distance_squared(p);
		float distance_to_v = glm::distance(p, v->point);
		float distance_to_f = glm::distance(p, f->centroid());

		if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_meshdatastructure_followvertices])->GetValue() &&
			(distance_to_v  < s.length() / 4.0f || !get<0>(main_window->checkboxes[myFrameGUI::checkbox_meshdatastructure_followedges])->GetValue()) &&
			(distance_to_v <= distance_to_f / 2.0f || !get<0>(main_window->checkboxes[myFrameGUI::checkbox_meshdatastructure_followfaces])->GetValue())
			)
		{
			main_window->updateDataStructureInfo(v);
			vertices_todraw[DATASTRUCTURE].push_back(v);
		}
		else if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_meshdatastructure_followedges])->GetValue() &&
			(distance_squared_to_e  <  distance_to_f * distance_to_f / 2.0f || !get<0>(main_window->checkboxes[myFrameGUI::checkbox_meshdatastructure_followfaces])->GetValue())
			)

		{
			main_window->updateDataStructureInfo(e);
			edges_todraw[DATASTRUCTURE].push_back(e);
		}
		else if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_meshdatastructure_followfaces])->GetValue())
		{
			main_window->updateDataStructureInfo(f);
			faces_todraw[DATASTRUCTURE].push_back(f);
		}
		Refresh(false);
		Update();
	}

}



void myGLCanvasGUI::OnMouse(wxMouseEvent& event)
{
	if (!initialized) return;

	float factor = 1.0f;
	if (wxGetKeyState(WXK_SHIFT))
		factor /= 40.0f;

	if (event.Dragging())
	{
		int dx = event.GetX() - mouse[0];
		int dy = event.GetY() - mouse[1];
		if (dx == 0 && dy == 0) return;

		if (event.RightIsDown())
			camera->panView(dx * factor, dy * factor);
		else camera->crystalball_rotateView(dx, dy);
	}

	if (event.GetWheelRotation())
	{
		float zoom_factor = 4200.0f / factor;
		camera->camera_eye += ( static_cast<float>(event.GetWheelRotation()) / zoom_factor) * camera->camera_forward;
	}

	if (event.LeftDown())
	{
		if (wxGetKeyState(WXK_CONTROL))
		{
			main_window->startTimer("Object Selection");
			myFace *f; myHalfedge *e; myVertex *v; glm::vec3 p;
			closestObject(event.GetX(), event.GetY(), p, v, e, f);
			if (f)
			{
				if ( (get<0>(main_window->listboxes[myFrameGUI::listbox_allowmultipleselectedpoints])->GetSelection() == 0) )
					clear_todraw(SELECTED);
				if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_selectfaces])->GetValue() && std::find(faces_todraw[SELECTED].begin(), faces_todraw[SELECTED].end(), f) == faces_todraw[SELECTED].end() ) faces_todraw[SELECTED].push_back(f);
				if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_selectpoints])->GetValue()) points_todraw[SELECTED].push_back(p);
				if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_selectedges])->GetValue() && e && std::find(edges_todraw[SELECTED].begin(), edges_todraw[SELECTED].end(), e) == edges_todraw[SELECTED].end() ) edges_todraw[SELECTED].push_back(e);
				if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_selectvertices])->GetValue() && v && std::find(vertices_todraw[SELECTED].begin(), vertices_todraw[SELECTED].end(), v) == vertices_todraw[SELECTED].end() ) vertices_todraw[SELECTED].push_back(v);
				main_window->updateMeshInfo();
			}
			else cout << "No object selected.\n";

			main_window->endTimer();
		}
	}

	if (wxGetKeyState(WXK_CONTROL) && (get<0>(main_window->checkboxes[myFrameGUI::checkbox_meshdatastructure_followvertices])->GetValue() || get<0>(main_window->checkboxes[myFrameGUI::checkbox_meshdatastructure_followedges])->GetValue() || get<0>(main_window->checkboxes[myFrameGUI::checkbox_meshdatastructure_followfaces])->GetValue()) )
		computeDataStructureInfo(event.GetX(), event.GetY());

	mouse[0] = event.GetX();
	mouse[1] = event.GetY();

	Refresh(false);
}


void myGLCanvasGUI::InitGL()
{
	glewInit();

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glClearColor(INITIAL_BACKGROUNDCOLOR.x, INITIAL_BACKGROUNDCOLOR.y, INITIAL_BACKGROUNDCOLOR.z, 0.0f);

	glEnable(GL_MULTISAMPLE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shaderprogram = new myShader("shaders/vertex.shader.glsl", "shaders/fragment.shader.glsl");
	shaderprogram->start();
	shaderprogram->setUniform("transparency", 0);

	camera = new myCamera();

	wxConfigBase *pConfig = wxConfigBase::Get();
	for (int i : {0, 1, 2})
	{
		pConfig->Read(to_string(myFrameGUI::_CURRENTCAMERAEYE_X + i), &camera->camera_eye[i]);
		pConfig->Read(to_string(myFrameGUI::_CURRENTCAMERAFORWARD_X + i), &camera->camera_forward[i]);
		pConfig->Read(to_string(myFrameGUI::_CURRENTCAMERAUP_X + i), &camera->camera_up[i]);
	}


	int w, h;
	GetClientSize(&w, &h);
	camera->setWindowSize(w, h);

	mouse[0] = 0; mouse[1] = 0;
}

void myGLCanvasGUI::updateDisplayBuffers()
{
	if (mesh == nullptr) return;

	myMesh *input_mesh = mesh;

	vector <GLfloat> verts; 
	vector <GLfloat> norms_per_face; 
	vector <GLfloat> norms; 
	vector <GLfloat> verts_and_normals; 

	num_triangles = 0;
	size_t index = 0;
	for (size_t i = 0; i < input_mesh->faces.size(); i++)
	{
		myHalfedge *e = input_mesh->faces[i]->adjacent_halfedge->next;
		do
		{
			verts.push_back(input_mesh->faces[i]->adjacent_halfedge->source->point.x);
			verts.push_back(input_mesh->faces[i]->adjacent_halfedge->source->point.y);
			verts.push_back(input_mesh->faces[i]->adjacent_halfedge->source->point.z);
			input_mesh->faces[i]->adjacent_halfedge->source->index = index;
			index++;

			verts.push_back(e->source->point.x);
			verts.push_back(e->source->point.y);
			verts.push_back(e->source->point.z);
			e->source->index = index;
			index++;

			verts.push_back(e->next->source->point.x);
			verts.push_back(e->next->source->point.y);
			verts.push_back(e->next->source->point.z);
			e->next->source->index = index;
			index++;

			norms_per_face.push_back(input_mesh->faces[i]->normal.x);
			norms_per_face.push_back(input_mesh->faces[i]->normal.y);
			norms_per_face.push_back(input_mesh->faces[i]->normal.z);

			norms_per_face.push_back(input_mesh->faces[i]->normal.x);
			norms_per_face.push_back(input_mesh->faces[i]->normal.y);
			norms_per_face.push_back(input_mesh->faces[i]->normal.z);

			norms_per_face.push_back(input_mesh->faces[i]->normal.x);
			norms_per_face.push_back(input_mesh->faces[i]->normal.y);
			norms_per_face.push_back(input_mesh->faces[i]->normal.z);


			norms.push_back(input_mesh->faces[i]->adjacent_halfedge->source->normal.x);
			norms.push_back(input_mesh->faces[i]->adjacent_halfedge->source->normal.y);
			norms.push_back(input_mesh->faces[i]->adjacent_halfedge->source->normal.z);

			norms.push_back(e->source->normal.x);
			norms.push_back(e->source->normal.y);
			norms.push_back(e->source->normal.z);

			norms.push_back(e->next->source->normal.x);
			norms.push_back(e->next->source->normal.y);
			norms.push_back(e->next->source->normal.z);

			num_triangles++;
			e = e->next;
		} while (e->next != input_mesh->faces[i]->adjacent_halfedge);
	}

	for (size_t i = 0; i < input_mesh->vertices.size(); i++)
	{
		verts_and_normals.push_back(input_mesh->vertices[i]->point.x);
		verts_and_normals.push_back(input_mesh->vertices[i]->point.y);
		verts_and_normals.push_back(input_mesh->vertices[i]->point.z);

		verts_and_normals.push_back((input_mesh->vertices[i]->point.x + input_mesh->vertices[i]->normal.x / 20.0f));
		verts_and_normals.push_back((input_mesh->vertices[i]->point.y + input_mesh->vertices[i]->normal.y / 20.0f));
		verts_and_normals.push_back((input_mesh->vertices[i]->point.z + input_mesh->vertices[i]->normal.z / 20.0f));
	}

	vector <GLuint> indices_edges;
	for (size_t i = 0; i < input_mesh->halfedges.size(); i++)
	{
		if (input_mesh->halfedges[i] == nullptr || input_mesh->halfedges[i]->next->next == nullptr) continue;
		indices_edges.push_back(input_mesh->halfedges[i]->source->index);
		indices_edges.push_back(input_mesh->halfedges[i]->next->source->index);
	}

	vector <GLuint> indices_vertices;
	for (size_t i = 0; i < input_mesh->vertices.size(); i++)
		indices_vertices.push_back(input_mesh->vertices[i]->index);

	glDeleteBuffers(NUM_BUFFERS, &buffers[0]);
	glDeleteVertexArrays(NUM_BUFFERS, &vaos[0]);

	buffers.assign(buffers.size(), 0);
	vaos.assign(vaos.size(), 0);

	glGenBuffers(NUM_BUFFERS, &buffers[0]);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTICES]);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), &verts[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_NORMALS_PERVERTEX]);
	glBufferData(GL_ARRAY_BUFFER, norms.size() * sizeof(GLfloat), &norms[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_NORMALS_PERFACE]);
	glBufferData(GL_ARRAY_BUFFER, norms_per_face.size() * sizeof(GLfloat), &norms_per_face[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTICESFORNORMALDRAWING]);
	glBufferData(GL_ARRAY_BUFFER, verts_and_normals.size() * sizeof(GLfloat), &verts_and_normals[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[BUFFER_INDICES_EDGES]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_edges.size() * sizeof(GLuint), &indices_edges[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[BUFFER_INDICES_VERTICES]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_vertices.size() * sizeof(GLuint), &indices_vertices[0], GL_STATIC_DRAW);


	glGenVertexArrays(NUM_BUFFERS, &vaos[0]);

	glBindVertexArray(vaos[VAO_TRIANGLES_NORMSPERVERTEX]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTICES]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_NORMALS_PERVERTEX]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	glBindVertexArray(vaos[VAO_TRIANGLES_NORMSPERFACE]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTICES]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_NORMALS_PERFACE]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	glBindVertexArray(vaos[VAO_EDGES]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTICES]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_NORMALS_PERVERTEX]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[BUFFER_INDICES_EDGES]);
	glBindVertexArray(0);

	glBindVertexArray(vaos[VAO_VERTICES]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTICES]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_NORMALS_PERVERTEX]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[BUFFER_INDICES_VERTICES]);
	glBindVertexArray(0);

	glBindVertexArray(vaos[VAO_NORMALS]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTICESFORNORMALDRAWING]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	verts.clear();
	norms_per_face.clear();
	norms.clear();
	verts_and_normals.clear();
	indices_edges.clear();
	indices_vertices.clear();
}


void myGLCanvasGUI::clear_todraw(myObjectId s)
{
	if (edges_todraw.count(s)) edges_todraw[s].clear();
	if (faces_todraw.count(s)) faces_todraw[s].clear();
	if (vertices_todraw.count(s)) vertices_todraw[s].clear();
	if (points_todraw.count(s)) points_todraw[s].clear();
	if (segments_todraw.count(s)) segments_todraw[s].clear();
	if (boxes_todraw.count(s)) boxes_todraw[s].clear();
}

void myGLCanvasGUI::reset()
{
	if (initialized) updateDisplayBuffers();

	clear_todraw(SELECTED);
	//clear_todraw(CATMULLCLARK_FACEPOINTS);
	//clear_todraw(CATMULLCLARK_EDGEPOINTS);
	//clear_todraw(CATMULLCLARK_VERTEXPOINTS);
	clear_todraw(VORONOI_POINTS);
	clear_todraw(POLES_POINTS);
	clear_todraw(DELAUNAY_EDGES);
	clear_todraw(FRACTALIZE);
	clear_todraw(BVH);
	
	if (bvh)
	{
		delete bvh;
		bvh = nullptr;
	}
	
	
	if (reconstructed_mesh)
	{
		delete reconstructed_mesh;
		reconstructed_mesh = nullptr;
	}
	
	Refresh(false);
}

void myGLCanvasGUI::OnPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);
	SetCurrent(*m_glRC);

	if (!initialized)
	{
		InitGL();
		updateDisplayBuffers();
		initialized = true;
		main_window->showhideLogWindow(get<0>(main_window->radioboxes[myFrameGUI::radiobox_outputwindow])->GetSelection() == 1);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, camera->window_width, camera->window_height);

	glm::mat4 projection_matrix = camera->projectionMatrix();
	glm::mat4 view_matrix = camera->viewMatrix();
	glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(view_matrix)));

	shaderprogram->setUniform("myprojection_matrix", projection_matrix);
	shaderprogram->setUniform("myview_matrix", view_matrix);
	shaderprogram->setUniform("mynormal_matrix", normal_matrix);

	if ((get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawmesh])->GetValue() && vaos[VAO_TRIANGLES_NORMSPERVERTEX]) || get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawsilhouette])->GetValue())
	{
		glLineWidth(1.0);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(2.0f, 2.0f);

		if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawsilhouette])->GetValue() && !get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawmesh])->GetValue()) shaderprogram->setUniform("type", RENDER_DIRECT);
		if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawmesh])->GetValue()) shaderprogram->setUniform("kd", MYCOLOR_MESHCOLOR);
		else shaderprogram->setUniform("kd", MYCOLOR_WHITE);


		if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawsmooth])->GetValue())
		{
			glBindVertexArray(vaos[VAO_TRIANGLES_NORMSPERVERTEX]);
			glDrawArrays(GL_TRIANGLES, 0, num_triangles * 3);
			glBindVertexArray(0);
		}
		else
		{
			glBindVertexArray(vaos[VAO_TRIANGLES_NORMSPERFACE]);
			glDrawArrays(GL_TRIANGLES, 0, num_triangles * 3);
			glBindVertexArray(0);
		}

		shaderprogram->setUniform("type", RENDER_BUFFERS);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawvertices])->GetValue() && vaos[VAO_VERTICES])
	{
		glPointSize(4.0);
		shaderprogram->setUniform("kd", MYCOLOR_BLACK);

		glBindVertexArray(vaos[VAO_VERTICES]);
		glDrawElements(GL_POINTS, mesh->vertices.size(), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}

	if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawwireframe])->GetValue() && vaos[VAO_EDGES])
	{
		glLineWidth(1.0);
		shaderprogram->setUniform("kd", MYCOLOR_BLACK);

		glBindVertexArray(vaos[VAO_EDGES]);
		glDrawElements(GL_LINES, mesh->halfedges.size() * 2, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}

	if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawsilhouette])->GetValue())
	{
		glLineWidth(4.0);
		shaderprogram->setUniform("kd", MYCOLOR_RED);

		vector <GLuint> silhouette_segments;
		vector <myHalfedge *> silhouette_edges;
		mesh->computeSilhouette(silhouette_edges, camera->camera_eye);

		for (size_t i = 0;i<silhouette_edges.size(); i++)
		{
			if (silhouette_edges[i] == nullptr) continue;
			silhouette_segments.push_back(silhouette_edges[i]->source->index);
			silhouette_segments.push_back(silhouette_edges[i]->twin->source->index);
		}

		if (silhouette_segments.size() > 0)
		{
			GLuint silhouette_edges_buffer;
			glGenBuffers(1, &silhouette_edges_buffer);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, silhouette_edges_buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, silhouette_segments.size() * sizeof(GLuint), &silhouette_segments[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTICES]);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_NORMALS_PERVERTEX]);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(1);

			glDrawElements(GL_LINES, silhouette_segments.size(), GL_UNSIGNED_INT, nullptr);

			glDeleteBuffers(1, &silhouette_edges_buffer);
		}		
	}

	if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawnormals])->GetValue() && vaos[VAO_NORMALS])
	{
		glLineWidth(1.0);
		shaderprogram->setUniform("kd", MYCOLOR_LIGHTGRAY);

		glBindVertexArray(vaos[VAO_NORMALS]);
		glDrawArrays(GL_LINES, 0, mesh->vertices.size() * 2);
		glBindVertexArray(0);
	}


	
	if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawselectedfaces])->GetValue())
		draw_faces(faces_todraw[SELECTED], MYCOLOR_RED);
	
	if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawselectedpoints])->GetValue() && points_todraw.count(SELECTED))
		draw_points(points_todraw[SELECTED], MYCOLOR_BLUE);
	
	if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawselectedvertices])->GetValue())
		draw_vertices(vertices_todraw[SELECTED], MYCOLOR_RED);
	
	if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawselectededges])->GetValue())
		draw_edges(edges_todraw[SELECTED], MYCOLOR_YELLOW);

	if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_catmullclark_viewfacepoints])->GetValue() && points_todraw.count(CATMULLCLARK_FACEPOINTS))
		draw_points(points_todraw[CATMULLCLARK_FACEPOINTS], MYCOLOR_WHITE);

	if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_catmullclark_viewedgepoints])->GetValue() && points_todraw.count(CATMULLCLARK_EDGEPOINTS))
		draw_points(points_todraw[CATMULLCLARK_EDGEPOINTS], MYCOLOR_BLUE);

	if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_catmullclark_viewvertexpoints])->GetValue() && points_todraw.count(CATMULLCLARK_VERTEXPOINTS))
		draw_points(points_todraw[CATMULLCLARK_VERTEXPOINTS], MYCOLOR_RED);

	if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawvoronoi])->GetValue() && points_todraw.count(VORONOI_POINTS)  )
		draw_points(points_todraw[VORONOI_POINTS], MYCOLOR_RED);

	if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawpoles])->GetValue() && points_todraw.count(POLES_POINTS))
		draw_points(points_todraw[POLES_POINTS], MYCOLOR_BLUE);

	if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawdelaunay])->GetValue() && segments_todraw.count(DELAUNAY_EDGES) )
		draw_segments(segments_todraw[DELAUNAY_EDGES], MYCOLOR_BLACK);

	if (get<0>(main_window->checkboxes[myFrameGUI::checkbox_drawreconstructedmesh])->GetValue() && reconstructed_mesh != nullptr)
		draw_faces(reconstructed_mesh->faces, MYCOLOR_YELLOW);

	if (wxGetKeyState(WXK_CONTROL) && edges_todraw.count(DATASTRUCTURE))
		draw_edges(edges_todraw[DATASTRUCTURE], MYCOLOR_BLACK, 10.0f);

	if (wxGetKeyState(WXK_CONTROL) && vertices_todraw.count(DATASTRUCTURE))
		draw_vertices(vertices_todraw[DATASTRUCTURE], MYCOLOR_BLACK, 10.0f);

	if (wxGetKeyState(WXK_CONTROL) && faces_todraw.count(DATASTRUCTURE))
		draw_faces(faces_todraw[DATASTRUCTURE], MYCOLOR_BLACK);

	if (bvh != nullptr && get<0>(main_window->checkboxes[myFrameGUI::checkbox_showbvh])->GetValue() && bvh->all_bboxes.size() > get<0>(main_window->spinctrls[myFrameGUI::spinctrl_bvh_depth])->GetValue())
		draw_boxes(bvh->all_bboxes[get<0>(main_window->spinctrls[myFrameGUI::spinctrl_bvh_depth])->GetValue()], MYCOLOR_LIGHTGRAY);

	glFlush();

	SwapBuffers();
}


void myGLCanvasGUI::draw_faces(vector<myFace *> &F, glm::vec4 color) const
{
	shaderprogram->setUniform("kd", color);
	shaderprogram->setUniform("type", RENDER_DIRECT);

	for (size_t k = 0;k<F.size();k++)
	{
		myFace *f = F[k];
		myHalfedge *e = f->adjacent_halfedge;
		glBegin(GL_TRIANGLE_FAN);
		glVertex3fv(glm::value_ptr(e->source->point));
		glVertex3fv(glm::value_ptr(e->next->source->point));
		e = e->next;
		do
		{
			glVertex3fv(glm::value_ptr(e->next->source->point));
			e = e->next;
		} while (e->next != f->adjacent_halfedge);
		glEnd();
	}

	shaderprogram->setUniform("type", RENDER_BUFFERS);
}

void myGLCanvasGUI::draw_vertices(vector<myVertex *> &V, glm::vec4 color, float size) const
{
	glPointSize(size);
	shaderprogram->setUniform("kd", color);
	shaderprogram->setUniform("type", RENDER_DIRECT);

	glBegin(GL_POINTS);
	for (size_t k = 0; k < V.size();k++)
		glVertex3fv(glm::value_ptr(V[k]->point));
	glEnd();

	shaderprogram->setUniform("type", RENDER_BUFFERS);
}

void myGLCanvasGUI::draw_edges(vector<myHalfedge *> & E, glm::vec4 color, float size) const
{
	glLineWidth(size);
	shaderprogram->setUniform("kd", color);
	shaderprogram->setUniform("type", RENDER_DIRECT);

	glBegin(GL_LINES);
	for (size_t k = 0; k < E.size(); k++)
	{
		glVertex3fv(glm::value_ptr(E[k]->source->point));
		glVertex3fv(glm::value_ptr(E[k]->next->source->point));
	}
	glEnd();

	shaderprogram->setUniform("type", RENDER_BUFFERS);
}

void myGLCanvasGUI::draw_segments(vector<mySegment> & S, glm::vec4 color, float size) const
{
	glLineWidth(size);
	shaderprogram->setUniform("kd", color);
	shaderprogram->setUniform("type", RENDER_DIRECT);

	glBegin(GL_LINES);
	for (size_t k = 0; k < S.size(); k++)
	{
		glVertex3fv(glm::value_ptr(S[k].p1));
		glVertex3fv(glm::value_ptr(S[k].p2));
	}
	glEnd();

	shaderprogram->setUniform("type", RENDER_BUFFERS);
}

void myGLCanvasGUI::draw_points(vector<glm::vec3> &P, glm::vec4 color, float size) const
{
	glPointSize(size);
	shaderprogram->setUniform("kd", color);
	shaderprogram->setUniform("type", RENDER_DIRECT);

	glBegin(GL_POINTS);
	for (size_t k = 0; k < P.size(); k++)
		glVertex3fv(glm::value_ptr(P[k]));
	glEnd();

	shaderprogram->setUniform("type", RENDER_BUFFERS);
}

void myGLCanvasGUI::draw_boxes(vector<myAABB> & B, glm::vec4 color) const
{
	shaderprogram->setUniform("kd", color);
	shaderprogram->setUniform("type", RENDER_DIRECT);
	shaderprogram->setUniform("transparency", 1);

	for (size_t k = 0; k < B.size(); k++)
	{
		myAABB & b = B[k];

		if (b.flag) shaderprogram->setUniform("kd", MYCOLOR_RED);
		else shaderprogram->setUniform("kd", color);

		glBegin(GL_QUADS);
		glVertex3f(b.min_x(), b.min_y(), b.min_z());
		glVertex3f(b.min_x(), b.min_y(), b.max_z());
		glVertex3f(b.min_x(), b.max_y(), b.max_z());
		glVertex3f(b.min_x(), b.max_y(), b.min_z());

		glVertex3f(b.min_x(), b.min_y(), b.min_z());
		glVertex3f(b.min_x(), b.max_y(), b.min_z());
		glVertex3f(b.max_x(), b.max_y(), b.min_z());
		glVertex3f(b.max_x(), b.min_y(), b.min_z());

		glVertex3f(b.min_x(), b.min_y(), b.min_z());
		glVertex3f(b.max_x(), b.min_y(), b.min_z());
		glVertex3f(b.max_x(), b.min_y(), b.max_z());
		glVertex3f(b.min_x(), b.min_y(), b.max_z());

		glVertex3f(b.max_x(), b.max_y(), b.max_z());
		glVertex3f(b.max_x(), b.max_y(), b.min_z());
		glVertex3f(b.max_x(), b.min_y(), b.min_z());
		glVertex3f(b.max_x(), b.min_y(), b.max_z());

		glVertex3f(b.max_x(), b.max_y(), b.max_z());
		glVertex3f(b.max_x(), b.min_y(), b.max_z());
		glVertex3f(b.min_x(), b.min_y(), b.max_z());
		glVertex3f(b.min_x(), b.max_y(), b.max_z());

		glVertex3f(b.max_x(), b.max_y(), b.max_z());
		glVertex3f(b.min_x(), b.max_y(), b.max_z());
		glVertex3f(b.min_x(), b.max_y(), b.min_z());
		glVertex3f(b.max_x(), b.max_y(), b.min_z());
		glEnd();
	}

	shaderprogram->setUniform("type", RENDER_BUFFERS);
	shaderprogram->setUniform("transparency", 0);
}