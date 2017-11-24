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
#include "default_constants.h"
#include <chrono>
#include "wx/spinctrl.h"
#include "wx/gbsizer.h"
#include "wx/notebook.h"
#include <tuple>
#include <map>
#include "myMesh.h"

class myGLCanvasGUI;

class myFrameGUI : public wxFrame
{
public:
	myFrameGUI(wxFrame *frame, const wxString& title, const wxPoint& pos, const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);
	~myFrameGUI();


	std::map<int, std::string> meshes_menu;
	void onMenuEvent(wxCommandEvent &);
	void onKeyboard(wxKeyEvent& event);
	void eventHandling(wxCommandEvent &event);

	void onSize(wxSizeEvent& event);
	void reSize(float input_logwindow_height = OUTPUTWINDOW_HEIGHT_PROPORTION);
	
	void updateMeshInfo();
	void readMesh(std::string filename);
	
	void showhideLogWindow(bool);
	
	wxNotebook *tabs;
	enum myTabId { VIEW, CHECKMESH, SELECTION, EDITING, SUBDIVISION, SIMPLIFICATION, RECONSTRUCTION, BVH, _END, _TOPPANEL, _LEFTPANEL };
	std::map<myTabId, std::tuple<wxPanel *, std::string> > tabpanels =
	{
		{ VIEW, std::make_tuple(nullptr, "View") },
		{ CHECKMESH, std::make_tuple(nullptr, "Check Mesh") },
		{ SELECTION, std::make_tuple(nullptr, "Selection") },
		{ EDITING, std::make_tuple(nullptr, "Editing") },
		{ SUBDIVISION, std::make_tuple(nullptr, "Subdivision") },
		{ SIMPLIFICATION, std::make_tuple(nullptr, "Simplification") },
		{ RECONSTRUCTION, std::make_tuple(nullptr, "Reconstruction") },
		{ BVH, std::make_tuple(nullptr, "BVH") },
		{ _END, std::make_tuple(nullptr, "")}, 
		{ _TOPPANEL, std::make_tuple(nullptr, "") },
		{ _LEFTPANEL, std::make_tuple(nullptr, "") }
	};

	enum myCtrlId
	{
		_myCtrlId_MIN = wxID_HIGHEST + 1,
		#define CHECKBOX(a, b, c, d) a,
		#define RADIOBOX(a,b,c,d) a,
		#define SPINCTRL(a,b,c,d,e,f) a,
		#define BUTTON(a,b,c) a,
		#define LISTBOX(a,b,c,d) a,
		#define TEXTCTRL(a,b,c,d,e) a,
		#define STATICBOX(a,b,c,d) a,
		#include "controls.def"
		_CAMERAEYE_X, 
		_CAMERAEYE_Y,
		_CAMERAEYE_Z,
		_CAMERAFORWARD_X,
		_CAMERAFORWARD_Y,
		_CAMERAFORWARD_Z,
		_CAMERAUP_X,
		_CAMERAUP_Y,
		_CAMERAUP_Z,
		_FRAMEPOSITION_X, 
		_FRAMEPOSITION_Y,
		_FRAMESIZE_W,
		_FRAMESIZE_H,
		_CURRENTTAB,
		_CURRENTMESH,
		_CURRENTCAMERAEYE_X,
		_CURRENTCAMERAEYE_Y,
		_CURRENTCAMERAEYE_Z,
		_CURRENTCAMERAFORWARD_X, 
		_CURRENTCAMERAFORWARD_Y, 
		_CURRENTCAMERAFORWARD_Z, 
		_CURRENTCAMERAUP_X,
		_CURRENTCAMERAUP_Y,
		_CURRENTCAMERAUP_Z,
		_myCtrlId_MAX
	};

	std::map<myCtrlId, std::string> ids_to_string =
	{
		#define CHECKBOX(a,b,c,d) {a, #a},
		#define RADIOBOX(a,b,c,d) {a, #a}, 
		#define SPINCTRL(a,b,c,d,e,f) {a, #a}, 
		#define BUTTON(a,b,c) {a, #a}, 
		#define LISTBOX(a,b,c,d) {a, #a}, 
		#define TEXTCTRL(a,b,c,d,e) {a, #a}, 
		#define STATICBOX(a,b,c,d) {a, #a},
		#include "controls.def"
	};

	//wxcheckbox, tab name, label, value
	std::map<myCtrlId, std::tuple<wxCheckBox *, myTabId, std::string, bool>> checkboxes =
	{
		#define CHECKBOX(a,b,c,d) {a, std::make_tuple(nullptr,b,c,d)},
		#include "controls.def"
	};

	//radiobox, tab name, label, value
	std::map<myCtrlId, std::tuple<wxRadioBox *, myTabId, std::string, int>> radioboxes =
	{
		#define RADIOBOX(a,b,c,d) {a, std::make_tuple(nullptr,b,c,d)},
		#include "controls.def"
	};

	//spinctrl, tab name, label, minvalue, maxvalue, value
	std::map<myCtrlId, std::tuple<wxSpinCtrl *, myTabId, std::string, int, int, int>> spinctrls =
	{
		#define SPINCTRL(a,b,c,d,e,f) {a, std::make_tuple(nullptr, b,c,d,e,f)},
		#include "controls.def"
	};

	//button, tab name, label
	std::map<myCtrlId, std::tuple<wxButton *, myTabId, std::string>> buttons =
	{
		#define BUTTON(a,b,c) {a, std::make_tuple(nullptr,b,c)},
		#include "controls.def"
	};

	//listbox, tab name, label, value
	std::map<myCtrlId, std::tuple<wxListBox *, myTabId, std::string, int>> listboxes =
	{
		#define LISTBOX(a,b,c,d) {a, std::make_tuple(nullptr,b,c,d)},
		#include "controls.def"
	};

	//textctrl, tab name, label, style, font size
	std::map<myCtrlId, std::tuple<wxTextCtrl *, myTabId, std::string, long, long>> textctrls =
	{
		#define TEXTCTRL(a,b,c,d,e) {a, std::make_tuple(nullptr, b,c,d,e)}, 
		#include "controls.def"
	};

	//staticbox, tab name, label, font size
	std::map<myCtrlId, std::tuple<wxStaticBox *, myTabId, std::string, long>> staticboxes =
	{
		#define STATICBOX(a,b,c,d) {a, std::make_tuple(nullptr, b,c,d)}, 
		#include "controls.def"
	};
	
	myGLCanvasGUI *canvas = nullptr;
	wxPanel *toppanel = nullptr;
	wxPanel *leftpanel = nullptr;
	wxTextCtrl *logwindow = nullptr;
	wxStreamToTextRedirector *redirect = nullptr;

	bool rememberSettings = true;
	
	void reset();

	void *old_dsinfo = nullptr;
	void updateDataStructureInfo(myVertex *); 
	void updateDataStructureInfo(myHalfedge *);
	void updateDataStructureInfo(myFace *);

	void updateIntersectionStatistics(std::tuple<int, int, int, int> & stats);

	void startTimer(wxString event_name = wxString());
	void endTimer();
	std::chrono::steady_clock::time_point timer_clock;

	void computeFPS(wxTimerEvent &);
	wxTimer *fpsTimer;
	std::chrono::steady_clock::time_point fps_clock;
	long fps_counter = 0;

	wxDECLARE_EVENT_TABLE();
};

