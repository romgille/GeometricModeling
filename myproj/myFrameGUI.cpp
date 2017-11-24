#include "myFrameGUI.h"
#include "myGLCanvasGUI.h"
#include "default_constants.h"
#include <filesystem>
#include <sstream>
#include "errors.h"
#include "helper_functions.h"
#include "fstream"
#include "wx/config.h"
#include <chrono>


using namespace std;

wxBEGIN_EVENT_TABLE(myFrameGUI, wxFrame)
EVT_MENU(wxID_OPEN, myFrameGUI::onMenuEvent)
EVT_MENU(wxID_REVERT_TO_SAVED, myFrameGUI::onMenuEvent)
EVT_MENU(wxID_SAVEAS, myFrameGUI::onMenuEvent)
EVT_MENU(wxID_EXIT, myFrameGUI::onMenuEvent)
EVT_MENU(wxID_HELP, myFrameGUI::onMenuEvent)
EVT_MENU(wxID_VIEW_DETAILS, myFrameGUI::onMenuEvent)
EVT_SIZE(myFrameGUI::onSize)
EVT_ERASE_BACKGROUND(myGLCanvasGUI::OnEraseBackground)
EVT_CHAR_HOOK(myFrameGUI::onKeyboard)
wxEND_EVENT_TABLE()

// MyFrame constructor
myFrameGUI::myFrameGUI(wxFrame *frame, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
	: wxFrame(frame, wxID_ANY, title, pos, size, style | wxFULL_REPAINT_ON_RESIZE)
{
	SetIcon(wxICON(sample));

	wxConfigBase *pConfig = wxConfigBase::Get();

	wxPoint frameposition;
	if (!rememberSettings || !pConfig->Read(to_string(myFrameGUI::_FRAMEPOSITION_X), &frameposition.x))
		frameposition.x = wxDefaultPosition.x;
	if (!rememberSettings || !pConfig->Read(to_string(myFrameGUI::_FRAMEPOSITION_Y), &frameposition.y))
		frameposition.y = wxDefaultPosition.y;

	wxSize framesize;
	if (!rememberSettings || !pConfig->Read(to_string(myFrameGUI::_FRAMESIZE_W), &framesize.x))
		framesize.x = DEFAULT_WINDOW_WIDTH;
	if (!rememberSettings || !pConfig->Read(to_string(myFrameGUI::_FRAMESIZE_H), &framesize.y))
		framesize.y = DEFAULT_WINDOW_HEIGHT;

	SetPosition(frameposition);
	SetSize(framesize);


/*****FILE MENU********************************************************/
	{
		wxMenu *fileMenu = new wxMenu;
		fileMenu->Append(wxID_OPEN, wxT("&Open Mesh\tCtrl+O"));
		fileMenu->Append(wxID_REVERT_TO_SAVED, wxT("&Reload Mesh\tCtrl+R"));

		fileMenu->Append(wxID_SAVEAS, wxT("&Save Mesh to File\tCtrl+S"));
		fileMenu->AppendSeparator();
		fileMenu->Append(wxID_EXIT, wxT("E&xit\tCtrl+X"));

		wxMenu *meshMenu = new wxMenu;
		{
			int k = _myCtrlId_MAX;
			for (std::experimental::filesystem::path p : std::experimental::filesystem::directory_iterator("models/"))
			{
				meshMenu->Append(k, p.string());
				meshes_menu.emplace(k, p.string());
				Bind(wxEVT_COMMAND_MENU_SELECTED, &myFrameGUI::onMenuEvent, this, k);
				k++;
			}
		}

		wxMenu *helpMenu = new wxMenu;
		helpMenu->Append(wxID_HELP, wxT("&About"));
		helpMenu->Append(wxID_VIEW_DETAILS, wxT("&How to use this interface"));

		wxMenuBar *menuBar = new wxMenuBar;
		menuBar->Append(fileMenu, wxT("&File"));
		menuBar->Append(meshMenu, wxT("Mesh List"));
		menuBar->Append(helpMenu, wxT("&Help"));
		SetMenuBar(menuBar);
	}
/*****DEFINING AND POSITIONING MAIN ITEMS*********************************************************/
	{
		enum { X, Y, WIDTH, HEIGHT };

		vector<int> toppanel_placement = { 0, 0, framesize.GetWidth(), TOPPANELGUI_HEIGHT };

		vector<int> left_panel_placement = { 0, toppanel_placement[HEIGHT], LEFTPANELGUI_WIDTH, framesize.GetHeight() - toppanel_placement[HEIGHT] };

		vector<int> outputwindow_placement = { left_panel_placement[WIDTH], static_cast<int>((1.0f - OUTPUTWINDOW_HEIGHT_PROPORTION) * framesize.GetHeight()), framesize.GetWidth() - left_panel_placement[WIDTH], static_cast<int>(OUTPUTWINDOW_HEIGHT_PROPORTION * framesize.GetHeight()) };

		vector<int> canvas_placement = { left_panel_placement[WIDTH], toppanel_placement[HEIGHT], framesize.GetWidth() - left_panel_placement[WIDTH], framesize.GetHeight() - toppanel_placement[HEIGHT] - outputwindow_placement[HEIGHT] };

		toppanel = new wxPanel(this, wxID_ANY, wxPoint(toppanel_placement[0], toppanel_placement[1]), wxSize(toppanel_placement[2], toppanel_placement[3]));

		leftpanel = new wxPanel(this, wxID_ANY, wxPoint(left_panel_placement[0], left_panel_placement[1]), wxSize(left_panel_placement[2], left_panel_placement[3]));

		int attributelist[4] = { WX_GL_RGBA, WX_GL_BUFFER_SIZE, WX_GL_DOUBLEBUFFER, 0 };
		canvas = new myGLCanvasGUI(this, wxID_ANY, wxPoint(canvas_placement[0], canvas_placement[1]), wxSize(canvas_placement[2], canvas_placement[3]), wxSUNKEN_BORDER, "IN4I12", attributelist);
		canvas->main_window = this;

		logwindow = new wxTextCtrl(this, wxID_ANY, wxT("This is the output window.\n"), wxPoint(outputwindow_placement[0], outputwindow_placement[1]),
			wxSize(outputwindow_placement[2], outputwindow_placement[3]), wxTE_MULTILINE | wxTE_READONLY);

		redirect = new wxStreamToTextRedirector(logwindow);
		logwindow->SetBackgroundColour(wxColour(255, 255, 234));
	}
	/************************************************************************/

	wxSizer *sizer_tabpanels, *sizerPanel, *sizer_inside;
	wxStaticBoxSizer *wrapping_sizer;
	wxGridBagSizer *grid;
	wxFont font;
	myCtrlId ID;

/***TOPPANEL CONTROLS*********************************************************************/
	{
		sizerPanel = new wxBoxSizer(wxHORIZONTAL);

		tabs = new wxNotebook(toppanel, wxNB_FIXEDWIDTH);
		tabs->SetPadding(wxSize(14, 14));

		for (int id = VIEW; id != _END; id++)
			get<0>(tabpanels[static_cast<myTabId>(id)]) = new wxPanel(tabs);
		get<0>(tabpanels[_TOPPANEL]) = toppanel;
		get<0>(tabpanels[_LEFTPANEL]) = leftpanel;

		for (auto it = checkboxes.begin(); it != checkboxes.end(); ++it)
		{
			myCtrlId _id = it->first;
			myTabId _tab = get<1>(it->second);
			wxString _label = wxString(get<2>(it->second));
			bool _value = get<3>(it->second);
			bool _savedvalue;
			get<0>(it->second) = new wxCheckBox(get<0>(tabpanels[_tab]), _id, _label);

			if (rememberSettings && pConfig->Read(to_string(_id), &_savedvalue))
				get<0>(it->second)->SetValue(_savedvalue);
			else get<0>(it->second)->SetValue(_value);

			Bind(wxEVT_CHECKBOX, &myFrameGUI::eventHandling, this, _id);
		}

		for (auto it = spinctrls.begin(); it != spinctrls.end(); ++it)
		{
			myCtrlId _id = it->first;
			myTabId _tab = get<1>(it->second);
			wxString _label = wxString(get<2>(it->second));
			int _min = get<3>(it->second);
			int _max = get<4>(it->second);
			int _value = get<5>(it->second);
			int _savedvalue;
			get<0>(it->second) = new wxSpinCtrl(get<0>(tabpanels[_tab]), _id, _label);
			get<0>(it->second)->SetRange(_min, _max);

			if (rememberSettings && pConfig->Read(to_string(_id), &_savedvalue))
				get<0>(it->second)->SetValue(_savedvalue);
			else get<0>(it->second)->SetValue(_value);

			Bind(wxEVT_SPINCTRL, &myFrameGUI::eventHandling, this, _id);
		}

		for (auto it = buttons.begin(); it != buttons.end(); ++it)
		{
			myCtrlId _id = it->first;
			myTabId _tab = get<1>(it->second);
			wxString _label = wxString(get<2>(it->second));
			get<0>(it->second) = new wxButton(get<0>(tabpanels[_tab]), _id, _label);
			Bind(wxEVT_BUTTON, &myFrameGUI::eventHandling, this, _id);
		}

		for (auto it = textctrls.begin(); it != textctrls.end(); ++it)
		{
			myCtrlId _id = it->first;
			myTabId _tab = get<1>(it->second);
			wxString _label = wxString(get<2>(it->second));
			long _style = get<3>(it->second);
			long _fontsize = get<4>(it->second);
			get<0>(it->second) = new wxTextCtrl(get<0>(tabpanels[_tab]), _id, _label, wxDefaultPosition, wxDefaultSize, _style);
			font = get<0>(it->second)->GetFont();
			font.SetPointSize(_fontsize);
			font.SetWeight(wxFONTWEIGHT_BOLD);
			get<0>(it->second)->SetFont(font);
		}

		for (auto it = staticboxes.begin(); it != staticboxes.end(); ++it)
		{
			myCtrlId _id = it->first;
			myTabId _tab = get<1>(it->second);
			wxString _label = wxString(get<2>(it->second));
			long _fontsize = get<3>(it->second);
			get<0>(it->second) = new wxStaticBox(get<0>(tabpanels[_tab]), _id, _label, wxDefaultPosition, wxDefaultSize);
			font = get<0>(it->second)->GetFont();
			font.SetPointSize(_fontsize);
			font.SetWeight(wxFONTWEIGHT_BOLD);
			get<0>(it->second)->SetFont(font);
		}

		/*---VIEW---------------------------------------------*/
		sizer_tabpanels = new wxBoxSizer(wxHORIZONTAL);

		ID = radiobox_outputwindow;
		wxString choices[] = { wxT("Bottom window"), wxT("VSC console") };
		get<0>(radioboxes[ID]) = new wxRadioBox(get<0>(tabpanels[get<1>(radioboxes[ID])]), ID, wxString(get<2>(radioboxes[ID])), wxPoint(10, 10), wxSize(400, wxDefaultSize.GetHeight()), WXSIZEOF(choices), choices, 1, wxRA_SPECIFY_COLS);
		int _savedvalue;
		if (rememberSettings && pConfig->Read(to_string(ID), &_savedvalue))
			get<0>(radioboxes[ID])->SetSelection(_savedvalue);
		else get<0>(radioboxes[ID])->SetSelection(get<3>(radioboxes[ID]));
		Bind(wxEVT_RADIOBOX, &myFrameGUI::eventHandling, this, ID);
		sizer_tabpanels->Add(get<0>(radioboxes[ID]), 0, wxEXPAND, 5);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[VIEW]), wxID_ANY, wxT("Camera"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_resetcamera]), wxGROW | wxALL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[VIEW]), wxID_ANY, wxT("Mesh View"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);

		grid = new wxGridBagSizer(2, 2);

		grid->Add(get<0>(checkboxes[checkbox_drawvertices]), wxGBPosition(0, 0), wxGBSpan(1, 1));
		grid->Add(get<0>(checkboxes[checkbox_drawmesh]), wxGBPosition(0, 1), wxGBSpan(1, 1));
		grid->Add(get<0>(checkboxes[checkbox_drawwireframe]), wxGBPosition(0, 2), wxGBSpan(1, 1));
		grid->Add(get<0>(checkboxes[checkbox_drawsilhouette]), wxGBPosition(1, 0), wxGBSpan(1, 1));
		grid->Add(get<0>(checkboxes[checkbox_drawnormals]), wxGBPosition(1, 1), wxGBSpan(1, 1));
		grid->Add(get<0>(checkboxes[checkbox_drawsmooth]), wxGBPosition(1, 2), wxGBSpan(1, 1));
		grid->Add(get<0>(checkboxes[checkbox_drawcrease]), wxGBPosition(0, 3), wxGBSpan(1, 1));


		wrapping_sizer->Add(grid);

		get<0>(tabpanels[VIEW])->SetSizer(sizer_tabpanels);
		/*------------------------------------------------*/



		/*----CHECKING--------------------------------------------*/
		sizer_tabpanels = new wxBoxSizer(wxHORIZONTAL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[CHECKMESH]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_checkall]), wxGROW | wxALL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[CHECKMESH]), wxID_ANY, wxT("Twins"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_checkhalfedges_twinsselected]), wxGROW | wxALL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[CHECKMESH]), wxID_ANY, wxT("Next/Prev"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_checkhalfedges_nextprevselected]), wxGROW | wxALL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[CHECKMESH]), wxID_ANY, wxT("Sources"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_checkhalfedges_sourceselected]), wxGROW | wxALL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[CHECKMESH]), wxID_ANY, wxT("Vertex Fans"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_checkvertices_fansselected]), wxGROW | wxALL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[CHECKMESH]), wxID_ANY, wxT("Face Boundaries"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_checkfaces_boundaryedgesselected]), wxGROW | wxALL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[CHECKMESH]), wxID_ANY, wxT("DS Info"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(checkboxes[checkbox_meshdatastructure_followvertices]), wxSizerFlags().Expand());
		wrapping_sizer->Add(get<0>(checkboxes[checkbox_meshdatastructure_followedges]), wxSizerFlags().Expand());
		wrapping_sizer->Add(get<0>(checkboxes[checkbox_meshdatastructure_followfaces]), wxSizerFlags().Expand());


		get<0>(tabpanels[CHECKMESH])->SetSizer(sizer_tabpanels);
		/*------------------------------------------------*/



		/*----SELECTION--------------------------------------------*/

		sizer_tabpanels = new wxBoxSizer(wxHORIZONTAL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[SELECTION]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_clearselected]), wxGROW | wxALL);

		ID = listbox_allowmultipleselectedpoints;
		wxString allowselectedchoice[] = { wxT("Single object"), wxT("Multiple objects") };
		get<0>(listboxes[ID]) = new wxListBox(get<0>(tabpanels[get<1>(listboxes[ID])]), ID,
			wxDefaultPosition, wxDefaultSize, 2, allowselectedchoice, wxLB_SINGLE | wxLB_ALWAYS_SB);
		get<0>(listboxes[ID])->SetSelection(get<3>(listboxes[ID]));
		Bind(wxEVT_LISTBOX, &myFrameGUI::eventHandling, this, ID);
		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[SELECTION]), wxID_ANY, wxT("Object Selection"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxEXPAND, 5);
		wrapping_sizer->Add(get<0>(listboxes[ID]), wxSizerFlags().Expand());


		wrapping_sizer = new wxStaticBoxSizer(get<0>(staticboxes[staticbox_numpoints]), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(checkboxes[checkbox_selectpoints]), wxSizerFlags().Border().Left());
		wrapping_sizer->Add(get<0>(checkboxes[checkbox_drawselectedpoints]), wxSizerFlags().Border().Left());

		wrapping_sizer = new wxStaticBoxSizer(get<0>(staticboxes[staticbox_numvertices]), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(checkboxes[checkbox_selectvertices]), wxSizerFlags().Border().Left());
		wrapping_sizer->Add(get<0>(checkboxes[checkbox_drawselectedvertices]), wxSizerFlags().Border().Left());

		wrapping_sizer = new wxStaticBoxSizer(get<0>(staticboxes[staticbox_numedges]), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(checkboxes[checkbox_selectedges]), wxSizerFlags().Border().Left());
		wrapping_sizer->Add(get<0>(checkboxes[checkbox_drawselectededges]), wxSizerFlags().Border().Left());

		wrapping_sizer = new wxStaticBoxSizer(get<0>(staticboxes[staticbox_numfaces]), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(checkboxes[checkbox_selectfaces]), wxSizerFlags().Border().Left());
		wrapping_sizer->Add(get<0>(checkboxes[checkbox_drawselectedfaces]), wxSizerFlags().Border().Left());

		get<0>(tabpanels[SELECTION])->SetSizer(sizer_tabpanels);
		/*------------------------------------------------*/


		/*---EDITING---------------------------------------------*/

		sizer_tabpanels = new wxBoxSizer(wxHORIZONTAL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[EDITING]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_normalize]), wxGROW | wxALL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[EDITING]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_triangulate]), wxGROW | wxALL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[EDITING]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_fractalize]), wxGROW | wxALL);

		//INFLATE SLIDER
		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[EDITING]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);

		wrapping_sizer->Add(get<0>(spinctrls[spinctrl_inflate]), 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_inflate]), wxSizerFlags().Expand().Bottom());

		//SMOOTHEN SLIDER
		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[EDITING]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);

		wrapping_sizer->Add(get<0>(spinctrls[spinctrl_smoothen]), 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_smoothen]), wxSizerFlags().Expand());

		//SHARPEN SLIDER
		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[EDITING]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);

		wrapping_sizer->Add(get<0>(spinctrls[spinctrl_sharpen]), 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_sharpen]), wxSizerFlags().Expand());

		//CUTTING WITH PLANE
		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[EDITING]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);

		wrapping_sizer->Add(get<0>(buttons[button_cuttingwithplane]), wxGROW | wxALL);

		get<0>(tabpanels[EDITING])->SetSizer(sizer_tabpanels);
		/*------------------------------------------------*/


		/*----SUBDIVISION--------------------------------------------*/

		sizer_tabpanels = new wxBoxSizer(wxHORIZONTAL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[SUBDIVISION]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_splithalfedge]), wxGROW | wxALL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[SUBDIVISION]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_splitface]), wxGROW | wxALL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[SUBDIVISION]), wxID_ANY, wxT("Catmull-Clark Subdivision"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);

		sizer_inside = new wxBoxSizer(wxHORIZONTAL);
		sizer_inside->Add(get<0>(buttons[button_catmullclark_newpoints]), wxSizerFlags().Expand());
		sizer_inside->Add(get<0>(buttons[button_catmullclark]), wxSizerFlags().Expand());
		wrapping_sizer->Add(sizer_inside, 0, wxGROW | wxALL, 5);

		sizer_inside = new wxBoxSizer(wxHORIZONTAL);
		sizer_inside->Add(get<0>(checkboxes[checkbox_catmullclark_viewfacepoints]), wxSizerFlags().Expand());
		sizer_inside->Add(get<0>(checkboxes[checkbox_catmullclark_viewedgepoints]), wxSizerFlags().Expand());
		sizer_inside->Add(get<0>(checkboxes[checkbox_catmullclark_viewvertexpoints]), wxSizerFlags().Expand());
		wrapping_sizer->Add(sizer_inside, 0, wxEXPAND, 5);

		get<0>(tabpanels[SUBDIVISION])->SetSizer(sizer_tabpanels);
		/*------------------------------------------------*/



		/*----SIMPLIFICATION--------------------------------------------*/

		sizer_tabpanels = new wxBoxSizer(wxHORIZONTAL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[SIMPLIFICATION]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_contracthalfedge]), wxGROW | wxALL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[SIMPLIFICATION]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_contractface]), wxGROW | wxALL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[SIMPLIFICATION]), wxID_ANY, wxT("Quadric Metric Simplification"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);

		get<0>(tabpanels[SIMPLIFICATION])->SetSizer(sizer_tabpanels);
		/*------------------------------------------------*/



		/*---RECONSTRUCTION---------------------------------------------*/

		sizer_tabpanels = new wxBoxSizer(wxHORIZONTAL);

		//sizer_tab_reconstruction->Add(new wxButton(tab_reconstruction, ID_m_button_surfacereconstruction, wxT("&Just a button")), flagsBorder);
		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[RECONSTRUCTION]), wxID_ANY, wxT("Voronoi Surface Reconstruction"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);

		grid = new wxGridBagSizer(2, 2);

		grid->Add(get<0>(buttons[button_voronoidelaunay]), wxGBPosition(0, 0), wxGBSpan(1, 1));
		grid->Add(get<0>(buttons[button_surfacereconstruction]), wxGBPosition(0, 1), wxGBSpan(1, 1));

		grid->Add(get<0>(checkboxes[checkbox_drawvoronoi]), wxGBPosition(0, 2), wxGBSpan(1, 1));
		grid->Add(get<0>(checkboxes[checkbox_drawdelaunay]), wxGBPosition(0, 3), wxGBSpan(1, 1));
		grid->Add(get<0>(checkboxes[checkbox_drawpoles]), wxGBPosition(0, 4), wxGBSpan(1, 1));
		grid->Add(get<0>(checkboxes[checkbox_drawreconstructedmesh]), wxGBPosition(0, 5), wxGBSpan(1, 1));

		wrapping_sizer->Add(grid);

		get<0>(tabpanels[RECONSTRUCTION])->SetSizer(sizer_tabpanels);
		/*------------------------------------------------*/


		/*-----------BVH-------------------------------------*/
		sizer_tabpanels = new wxBoxSizer(wxHORIZONTAL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[BVH]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(buttons[button_bvh_construct]), wxSizerFlags().Expand());
		wrapping_sizer->Add(get<0>(buttons[button_bvh_reset]), wxSizerFlags().Expand());


		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[BVH]), wxID_ANY, wxT("BVH boxes by depth"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(spinctrls[spinctrl_bvh_depth]), 0, wxGROW | wxALL, 5);
		wrapping_sizer->Add(get<0>(checkboxes[checkbox_showbvh]), wxSizerFlags().Expand());


		ID = listbox_bvh_splittingmethod;
		wxString allowselectedchoicesplittingmethod[] = { wxT("Midpoint"), wxT("Median") };
		get<0>(listboxes[ID]) = new wxListBox(get<0>(tabpanels[get<1>(listboxes[ID])]), ID,
			wxDefaultPosition, wxDefaultSize, 2, allowselectedchoicesplittingmethod, wxLB_SINGLE | wxLB_ALWAYS_SB);
		int saved_val;
		if (rememberSettings && pConfig->Read(to_string(ID), &saved_val))
			get<0>(listboxes[ID])->SetSelection(saved_val);
		else get<0>(listboxes[ID])->SetSelection(get<3>(listboxes[ID]));


		Bind(wxEVT_LISTBOX, &myFrameGUI::eventHandling, this, ID);
		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[BVH]), wxID_ANY, wxT("Splitting Method"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		wrapping_sizer->Add(get<0>(listboxes[ID]), wxSizerFlags().Border().Center());
		sizer_tabpanels->Add(wrapping_sizer, 0, wxEXPAND, 5);


		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[BVH]), wxID_ANY, wxT("Intersection statistics"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		wrapping_sizer->Add(get<0>(textctrls[textctrl_bvh_stats]), wxSizerFlags().Expand());
		get<0>(textctrls[textctrl_bvh_stats])->SetInitialSize(wxSize(toppanel->GetSize().GetWidth() / 4.0f, toppanel->GetSize().GetHeight()));
		sizer_tabpanels->Add(wrapping_sizer, wxSizerFlags().Expand());


		get<0>(tabpanels[BVH])->SetSizer(sizer_tabpanels);
		/*------------------------------------------------*/




		/****** ADDING IN ALL TABS******************/
		font = tabs->GetFont();
		font.SetPointSize(14);
		font.SetWeight(wxFONTWEIGHT_BOLD);
		tabs->SetFont(font);

		for (int id = VIEW; id != _END; id++)
		{
			wxPanel *_p = get<0>(tabpanels[static_cast<myTabId>(id)]);
			wxString _l = wxString(get<1>(tabpanels[static_cast<myTabId>(id)]));
			_p->SetBackgroundColour(wxColour(188, 188, 188));
			tabs->AddPage(_p, _l);
		}
		sizerPanel->Add(tabs, wxGROW | wxALL);
		/*------------------------------------------------*/
		toppanel->SetSizerAndFit(sizerPanel);




/***LEFTPANEL CONTROLS*********************************************************************/


		sizerPanel = new wxBoxSizer(wxVERTICAL);

		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(leftpanel, wxID_ANY, wxT("Mesh Information"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);

		wrapping_sizer->Add(get<0>(textctrls[textctrl_meshinfovertices]), wxSizerFlags().Expand());
		get<0>(textctrls[textctrl_meshinfovertices])->SetInitialSize(wxSize(leftpanel->GetSize().GetWidth(), wxDefaultSize.GetHeight()));
		wrapping_sizer->Add(get<0>(textctrls[textctrl_meshinfoedges]), wxSizerFlags().Expand());
		wrapping_sizer->Add(get<0>(textctrls[textctrl_meshinfofaces]), wxSizerFlags().Expand());
		wrapping_sizer->Add(get<0>(textctrls[textctrl_fpsinfo]), wxSizerFlags().Expand());
		get<0>(textctrls[textctrl_fpsinfo])->SetForegroundColour(wxColour(255, 0, 0));
		sizerPanel->Add(wrapping_sizer, wxTOP);


		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(leftpanel, wxID_ANY, wxT("Data-Structure Information"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		wrapping_sizer->Add(get<0>(textctrls[textctrl_meshdatastructure]), wxGROW | wxALL);
		get<0>(textctrls[textctrl_meshdatastructure])->SetInitialSize(wxSize(leftpanel->GetSize().GetWidth(), leftpanel->GetSize().GetHeight() / 6.0f));
		sizerPanel->Add(wrapping_sizer, wxGROW | wxALL);



		wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(leftpanel, wxID_ANY, wxT("Runtime"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
		wrapping_sizer->Add(get<0>(textctrls[textctrl_runningtime]), wxGROW | wxALL);
		get<0>(textctrls[textctrl_runningtime])->SetInitialSize(wxSize(leftpanel->GetSize().GetWidth(), leftpanel->GetSize().GetHeight() / 3.0f));
		sizerPanel->Add(wrapping_sizer, wxGROW | wxALL);

		leftpanel->SetSizerAndFit(sizerPanel);




	}

	fpsTimer = new wxTimer();
	fpsTimer->SetOwner(this);
	fpsTimer->Start(1);
	Bind(wxEVT_TIMER, &myFrameGUI::computeFPS, this, wxID_ANY);
	fps_clock = std::chrono::high_resolution_clock::now();

	int current_tab;
	if (rememberSettings && pConfig->Read(to_string(_CURRENTTAB), &current_tab))
		tabs->SetSelection(current_tab);

	SetAutoLayout(true);

	Show(true);
}

myFrameGUI::~myFrameGUI()
{
	wxConfigBase *pConfig = wxConfigBase::Get();

	for (auto it = checkboxes.begin(); it != checkboxes.end(); ++it)
		pConfig->Write(to_string(it->first), get<0>(it->second)->GetValue());
	for (auto it = radioboxes.begin(); it != radioboxes.end(); ++it)
		pConfig->Write(to_string(it->first), get<0>(it->second)->GetSelection());
	for (auto it = listboxes.begin(); it != listboxes.end(); ++it)
		pConfig->Write(to_string(it->first), get<0>(it->second)->GetSelection());
	for (auto it = spinctrls.begin(); it != spinctrls.end(); ++it)
		pConfig->Write(to_string(it->first), get<0>(it->second)->GetValue());
	
	if (canvas->mesh != nullptr)
		pConfig->Write(to_string(_CURRENTMESH), wxString(canvas->mesh->name));

	pConfig->Write(to_string(_FRAMEPOSITION_X), GetPosition().x);
	pConfig->Write(to_string(_FRAMEPOSITION_Y), GetPosition().y);

	pConfig->Write(to_string(_FRAMESIZE_W), GetSize().GetWidth());
	pConfig->Write(to_string(_FRAMESIZE_H), GetSize().GetHeight());

	if (tabs != nullptr)
		pConfig->Write(to_string(_CURRENTTAB), tabs->GetSelection());

	if (canvas != nullptr && canvas->camera != nullptr)
	{
		for (int i : {0, 1, 2})
		{
			pConfig->Write(to_string(_CURRENTCAMERAEYE_X + i), canvas->camera->camera_eye[i]);
			pConfig->Write(to_string(_CURRENTCAMERAFORWARD_X + i), canvas->camera->camera_forward[i]);
			pConfig->Write(to_string(_CURRENTCAMERAUP_X + i), canvas->camera->camera_up[i]);
		}
	}


	fpsTimer->Stop();
}


void myFrameGUI::onMenuEvent(wxCommandEvent & event)
{
	startTimer("menu event");

	if (event.GetId() >= _myCtrlId_MAX && event.GetId() < _myCtrlId_MAX + meshes_menu.size())
	{
		if (!meshes_menu.count(event.GetId()))
		{
			PRINT(ERROR_FILEOPEN);
			return;
		}

		readMesh(meshes_menu[event.GetId()]);
	}

	switch (event.GetId())
	{
		case wxID_OPEN:
		{
			wxString filename = wxFileSelector(wxT("Choose Mesh"), wxT(""), wxT(""), wxT(""),
				wxT("Wavefront Obj Files (*.obj)|*.obj;|All files (*.*)|*.*"), wxFD_OPEN);

			if (filename.IsEmpty())
			{
				PRINT(ERROR_FILEOPEN);
				return;
			}

			readMesh(string(filename.c_str()));
			break;
		}
		case wxID_REVERT_TO_SAVED:
		{
			if (canvas->mesh->name.empty())
			{
				PRINT(ERROR_FILEOPEN);
				return;
			}

			readMesh(canvas->mesh->name);
			break;
		}
		case wxID_SAVEAS:
		{
			wxString filename = wxFileSelector(wxT("Choose Filename"), wxT(""), wxT(""), wxT(""),
				wxT("Wavefront Obj Files (*.obj)|*.obj;|All files (*.*)|*.*"), wxFD_OPEN);

			if (filename.IsEmpty())
			{
				PRINT(ERROR_FILEOPEN);
				return;
			}

			canvas->mesh->writeFile(string(filename.c_str()));
			break;
		}
		case wxID_EXIT:
		{
			Close(true);

			break;
		}
		case wxID_HELP:
		{
			wxMessageBox(wxT("IN4I12: unite de Nabil Mustafa."));

			break;
		}
		case wxID_VIEW_DETAILS:
		{
			wxString usage;
			usage << "Ctrl + left mouse button:        selection of vertices, edges and faces.\n";
			usage << "left mouse button dragging:      model rotation.\n";
			usage << "right mouse button dragging:     model panning.\n";
			usage << "Ctrl + R:                        reload the previous mesh.\n";
			usage << "Shift + right mouse button dragging: slow model panning.\n";
			usage << "Mouse wheel:                     zoomin/zoomout.\n";
			usage << "Shift + Mouse wheel:             slow zoomin/zoomout.\n";
			wxMessageBox(usage);
			break;
		}
		default:
		{
			break;
		}
	}

	endTimer();
}


void myFrameGUI::reSize(float input_logwindow_height)
{
	wxSize size = GetClientSize();

	if (toppanel != nullptr && get<0>(radioboxes[radiobox_outputwindow]) != nullptr && get<0>(radioboxes[radiobox_outputwindow])->GetSelection())
		input_logwindow_height = 0.0f;

	enum { X, Y, WIDTH, HEIGHT };
	vector<int> _toppanel = { 0, 0,
		size.GetWidth(),
		TOPPANELGUI_HEIGHT };

	vector<int> _leftpanel = { 0, _toppanel[HEIGHT],
		LEFTPANELGUI_WIDTH, size.GetHeight() - _toppanel[HEIGHT] };

	vector<int> _outputwindow = { _leftpanel[WIDTH],  	static_cast<int>((1.0f - input_logwindow_height) * size.GetHeight()),
		size.GetWidth() - _leftpanel[WIDTH], static_cast<int>(input_logwindow_height * size.GetHeight()) };

	vector<int> _canvas = { _leftpanel[WIDTH], _toppanel[HEIGHT],
		size.GetWidth() - _leftpanel[WIDTH], size.GetHeight() - _toppanel[HEIGHT] - _outputwindow[HEIGHT] };


	if (toppanel != nullptr)
	{
		toppanel->SetPosition(wxPoint(_toppanel[0], _toppanel[1]));
		toppanel->SetSize(_toppanel[2], _toppanel[3]);
	}

	if (leftpanel != nullptr)
	{
		leftpanel->SetPosition(wxPoint(_leftpanel[0], _leftpanel[1]));
		leftpanel->SetSize(_leftpanel[2], _leftpanel[3]);
	}

	if (logwindow != nullptr)
	{
		logwindow->SetPosition(wxPoint(_outputwindow[0], _outputwindow[1]));
		logwindow->SetSize(_outputwindow[2], _outputwindow[3]);
	}

	if (canvas != nullptr && canvas->initialized)
	{
		canvas->SetPosition(wxPoint(_canvas[0], _canvas[1]));
		canvas->SetSize(_canvas[2], _canvas[3]);
		canvas->camera->setWindowSize(_canvas[2], _canvas[3]);
		canvas->Refresh(false);
	}

}

void myFrameGUI::onSize(wxSizeEvent& event)
{
	reSize();
}

void myFrameGUI::readMesh(std::string filename)
{
	if (toppanel == nullptr || canvas == nullptr) return;

	if (canvas->mesh) delete canvas->mesh;

	canvas->mesh = new myMesh();
	if (!canvas->mesh->readFile(filename))
		cout << "Error: Unable to open mesh file " << filename << endl;

	reset();
}

void myFrameGUI::showhideLogWindow(bool toshow)
{
	if (toshow)
	{
		if (redirect)
		{
			delete redirect;
			redirect = nullptr;
			reSize(0.0f);
		}
	}
	else
	{
		if (redirect == nullptr)
			redirect = new wxStreamToTextRedirector(logwindow);
		reSize();
	}
}


void myFrameGUI::updateMeshInfo()
{

	wxString s;
	std::stringstream ss;

	ss.imbue(std::locale(""));
	ss << std::fixed << canvas->mesh->vertices.size();
	s << "Vertices:  " << ss.str();
	get<0>(textctrls[textctrl_meshinfovertices])->SetLabel(s);

	ss.str(std::string()); ss.clear();
	ss.imbue(std::locale(""));
	ss << std::fixed << canvas->mesh->halfedges.size();
	s.clear();
	s << "Halfedges: " << ss.str();
	get<0>(textctrls[textctrl_meshinfoedges])->SetLabel(s);

	ss.str(std::string()); ss.clear();
	ss.imbue(std::locale(""));
	ss << std::fixed << canvas->mesh->faces.size();
	s.clear();
	s << "Faces:    " << ss.str();
	get<0>(textctrls[textctrl_meshinfofaces])->SetLabel(s);

	s = "";
	s << "Points selected: " << canvas->points_todraw[myGLCanvasGUI::SELECTED].size();
	get<0>(staticboxes[staticbox_numpoints])->SetLabel(s);

	s = "";
	s << "Vertices selected: " << canvas->vertices_todraw[myGLCanvasGUI::SELECTED].size();
	get<0>(staticboxes[staticbox_numvertices])->SetLabel(s);

	s = "";
	s << "Halfedges selected: " << canvas->edges_todraw[myGLCanvasGUI::SELECTED].size();
	get<0>(staticboxes[staticbox_numedges])->SetLabel(s);

	s = "";
	s << "Faces selected: " << canvas->faces_todraw[myGLCanvasGUI::SELECTED].size();
	get<0>(staticboxes[staticbox_numfaces])->SetLabel(s);

}


void myFrameGUI::startTimer(wxString event_name)
{
	get<0>(textctrls[textctrl_runningtime])->SetBackgroundColour(wxColour(255, 0, 0));

	*get<0>(textctrls[textctrl_runningtime]) << "Event: " << event_name << "\r\n";

	this->Refresh();
	this->Update();

	timer_clock = std::chrono::high_resolution_clock::now();
}

void myFrameGUI::endTimer()
{
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - timer_clock;

	get<0>(textctrls[textctrl_runningtime])->SetBackgroundColour(wxColour(255, 255, 255));

	*get<0>(textctrls[textctrl_runningtime]) << "Time: " << elapsed.count() << " millisecs.\r\n";

	this->Refresh();
	this->Update();
}

void myFrameGUI::computeFPS(wxTimerEvent &event)
{
	fps_counter++;
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - fps_clock;

	if (elapsed.count() > 1000)
	{
		float fps = fps_counter * 1000.0f / elapsed.count();
		fps_clock = end;
		fps_counter = 0;

		stringstream ss;
		ss << "FPS: " << fps << endl;
		get<0>(textctrls[textctrl_fpsinfo])->SetLabel(wxString(ss.str()));
	}
}


void myFrameGUI::onKeyboard(wxKeyEvent& event)
{
	//cout << wxAcceleratorEntry(0 /* no modifiers */, event.GetKeyCode()).ToString() << " pressed.\n";
	//cout << tabs->GetSelection() << endl;

	switch(event.GetKeyCode())
	{
		case WXK_ESCAPE:
		{
			Close(true);
			break;
		}
		case WXK_CONTROL:
		{
			wxPoint pt = wxGetMousePosition() - canvas->GetScreenPosition();// -toppanel->GetPosition();
			canvas->computeDataStructureInfo(pt.x , pt.y);
			break;
		}
	}
 
	event.Skip();
}


void myFrameGUI::eventHandling(wxCommandEvent &event)
{
	startTimer(ids_to_string[static_cast<myCtrlId>(event.GetId())]);

	switch (event.GetId())
	{
	case listbox_bvh_splittingmethod:
	{
		break;
	}
	case spinctrl_bvh_depth:
	{
		break;
	}
	case checkbox_showbvh:
	{
		break;
	}
	case checkbox_meshdatastructure_followvertices:
	case checkbox_meshdatastructure_followedges:
	case checkbox_meshdatastructure_followfaces:
	{
		if (!get<0>(checkboxes[static_cast<myCtrlId>(event.GetId())])->GetValue())
		{
			canvas->clear_todraw(myGLCanvasGUI::DATASTRUCTURE);
			get<0>(textctrls[textctrl_meshdatastructure])->SetLabel(wxString(""));
		}
		break;
	}
	case button_bvh_construct:
	{
		if (canvas->bvh) delete canvas->bvh;
		canvas->bvh = new myBVH();
		canvas->bvh->buildBVH(canvas->mesh, static_cast<myBVH::splittingMethod>(get<0>(listboxes[listbox_bvh_splittingmethod])->GetSelection()));
		get<0>(spinctrls[spinctrl_bvh_depth])->SetValue(0);
		break;
	}
	case button_bvh_reset:
	{
		if (canvas->bvh) delete canvas->bvh;
		canvas->bvh = nullptr;
		break;
	}
	case checkbox_drawvoronoi:
	{
		break;
	}
	case checkbox_drawdelaunay:
	{
		break;
	}
	case checkbox_drawpoles:
	{
		break;
	}
	case checkbox_drawreconstructedmesh:
	{
		break;
	}
	case button_surfacereconstruction:
	{
		if (canvas->reconstructed_mesh) delete canvas->reconstructed_mesh;
		canvas->reconstructed_mesh = new myMesh(canvas->mesh->vertices);
		canvas->points_todraw[myGLCanvasGUI::POLES_POINTS] = canvas->reconstructed_mesh->voronoiReconstruction();
		break;
	}
	case button_voronoidelaunay:
	{
		vector<glm::uvec4> df;
		vector<glm::vec3> input_points;
		for (size_t i = 0; i < canvas->mesh->vertices.size(); i++)
			input_points.push_back(canvas->mesh->vertices[i]->point);
		computeVoronoiDelaunay(input_points, canvas->points_todraw[myGLCanvasGUI::VORONOI_POINTS], df);

		canvas->segments_todraw[myGLCanvasGUI::DELAUNAY_EDGES].clear();
		for (size_t i = 0; i < df.size(); i++)
			for (size_t j = 0; j < 4; j++)
				canvas->segments_todraw[myGLCanvasGUI::DELAUNAY_EDGES].push_back(mySegment(input_points[df[i][j]], input_points[df[i][(j + 1) % 4]]));
		break;
	}
	case button_resetcamera:
	{
		canvas->camera->reset();
		break;
	}
	case checkbox_catmullclark_viewfacepoints:
	{
		break;
	}
	case checkbox_catmullclark_viewedgepoints:
	{
		break;
	}
	case checkbox_catmullclark_viewvertexpoints:
	{
		break;
	}
	case button_catmullclark:
	{
		canvas->mesh->subdivisionCatmullClark();
		canvas->mesh->computeNormals();
		reset();
		break;
	}
	case button_catmullclark_newpoints:
	{
		canvas->mesh->subdivisionCatmullClark_createNewPoints(
			canvas->points_todraw[myGLCanvasGUI::CATMULLCLARK_FACEPOINTS],
			canvas->points_todraw[myGLCanvasGUI::CATMULLCLARK_EDGEPOINTS],
			canvas->points_todraw[myGLCanvasGUI::CATMULLCLARK_VERTEXPOINTS]);
		break;
	}
	case button_splitface:
	{
		if (canvas->faces_todraw[myGLCanvasGUI::SELECTED].size() && canvas->points_todraw[myGLCanvasGUI::SELECTED].size())
		{
			myFace *f = canvas->faces_todraw[myGLCanvasGUI::SELECTED].back();
			glm::vec3 p = canvas->points_todraw[myGLCanvasGUI::SELECTED].back();
			canvas->mesh->splitFaceTRIS(f, p);
			canvas->mesh->computeNormals();
			reset();
		}
		else cout << "Unable to split face.\n";
		break;
	}
	case button_splithalfedge:
	{
		if (canvas->edges_todraw[myGLCanvasGUI::SELECTED].size() && canvas->points_todraw[myGLCanvasGUI::SELECTED].size())
		{
			myHalfedge *e = canvas->edges_todraw[myGLCanvasGUI::SELECTED].back();
			glm::vec3 p = canvas->points_todraw[myGLCanvasGUI::SELECTED].back();
			canvas->mesh->splitEdge(e, p);
			canvas->mesh->computeNormals();
			reset();
		}
		else cout << "Unable to split halfedge.\n";
		break;
	}
	case checkbox_selectpoints:
	{
		break;
	}
	case checkbox_selectvertices:
	{
		break;
	}
	case checkbox_selectedges:
	{
		break;
	}
	case checkbox_selectfaces:
	{
		break;
	}
	case listbox_allowmultipleselectedpoints:
	{
		if ((get<0>(listboxes[static_cast<myCtrlId>(event.GetId())])->GetSelection() == 0))
			canvas->clear_todraw(myGLCanvasGUI::SELECTED);
		break;
	}
	case radiobox_outputwindow:
	{
		showhideLogWindow(get<0>(radioboxes[static_cast<myCtrlId>(event.GetId())])->GetSelection() == 1);
		break;
	}
	case button_checkhalfedges_twinsselected:
	{
		canvas->mesh->_checkhalfedges_twins(canvas->edges_todraw[myGLCanvasGUI::SELECTED]);
		break;
	}
	case button_checkhalfedges_nextprevselected:
	{
		canvas->mesh->_checkhalfedges_nextprev(canvas->edges_todraw[myGLCanvasGUI::SELECTED]);
		break;
	}
	case button_checkhalfedges_sourceselected:
	{
		canvas->mesh->_checkhalfedges_source(canvas->edges_todraw[myGLCanvasGUI::SELECTED]);
		break;
	}
	case button_checkvertices_fansselected:
	{
		canvas->mesh->_checkvertices_fans(canvas->vertices_todraw[myGLCanvasGUI::SELECTED]);
		break;
	}
	case button_checkfaces_boundaryedgesselected:
	{
		canvas->mesh->_checkfaces_boundaryedges(canvas->faces_todraw[myGLCanvasGUI::SELECTED]);
		break;
	}
	case button_checkall:
	{
		canvas->mesh->checkMesh();
		break;
	}
	case button_sharpen:
	{
		canvas->mesh->sharpen(get<0>(spinctrls[spinctrl_sharpen])->GetValue() / 10.0f);
		canvas->mesh->computeNormals();
		reset();
		break;
	}
	case button_smoothen:
	{
		canvas->mesh->smoothen(get<0>(spinctrls[spinctrl_smoothen])->GetValue() / 10.0f);
		canvas->mesh->computeNormals();
		reset();
		break;
	}
	case button_inflate:
	{
		canvas->mesh->inflate(get<0>(spinctrls[spinctrl_inflate])->GetValue() / 1000.0f);
		canvas->mesh->computeNormals();
		reset();
		break;
	}
	case button_triangulate:
	{
		canvas->mesh->triangulate();
		canvas->mesh->computeNormals();
		reset();
		break;
	}
	case button_fractalize:
	{
		canvas->mesh->fractalize();
		canvas->mesh->computeNormals();
		reset();
		break;
	}
	case button_normalize:
	{
		canvas->mesh->normalize();
		canvas->mesh->computeNormals();
		reset();
		break;
	}
	case checkbox_drawselectedfaces:
	{
		break;
	}
	case checkbox_drawselectededges:
	{
		break;
	}
	case checkbox_drawselectedvertices:
	{
		break;
	}
	case checkbox_drawselectedpoints:
	{
		break;
	}
	case button_clearselected:
	{
		canvas->clear_todraw(myGLCanvasGUI::SELECTED);
		updateMeshInfo();
		break;
	}
	case checkbox_drawvertices:
	{
		break;
	}
	case checkbox_drawmesh:
	{
		break;
	}
	case checkbox_drawwireframe:
	{
		break;
	}
	case checkbox_drawsilhouette:
	{
		break;
	}
	case checkbox_drawnormals:
	{
		break;
	}
	case checkbox_drawsmooth:
	{
		break;
	}
	case checkbox_drawcrease:
	{
	}
	default:
	{
		cout << "Unhandled event.\n";
		break;
	}
	}
	endTimer();

	canvas->Refresh(false);
}


void myFrameGUI::reset()
{
	logwindow->Clear();
	canvas->reset();
	updateMeshInfo();
}

void myFrameGUI::updateDataStructureInfo(myVertex *v)
{
	if (v == nullptr)
	{
		//get<0>(textctrls[textctrl_meshdatastructure])->SetLabel(wxString(""));
		return;
	}

	if ( static_cast<myVertex *>(old_dsinfo) == v) return;

	std::stringstream ss;
	ss << std::hex << std::noshowbase;
	ss << "-----VERTEX-----------------\r\n";
	ss << "Vertex: " << reinterpret_cast<intptr_t>(v) << "\r\n";
	ss << "Originof: " << reinterpret_cast<intptr_t>(v->originof) << "\r\n";
	ss << "Point: " << reinterpret_cast<intptr_t>(&v->point) << "\r\n";

	*get<0>(textctrls[textctrl_meshdatastructure]) << ss.str(); //->SetLabel(wxString(ss.str()));

	old_dsinfo = static_cast<void *>(v);
}

void myFrameGUI::updateDataStructureInfo(myHalfedge *e)
{
	if (e == nullptr)
	{
		//get<0>(textctrls[textctrl_meshdatastructure])->SetLabel(wxString(""));
		return;
	}

	if ( static_cast<myHalfedge *>(old_dsinfo) == e) return;

	std::stringstream ss;
	ss << "-----HALFEDGE-----------------\r\n";
	ss << std::hex << std::noshowbase;
	ss << "Edge: " << reinterpret_cast<intptr_t>(e) << "\r\n";
	ss << "Next: " << reinterpret_cast<intptr_t>(e->next) << "\r\n";
	ss << "Prev: " << reinterpret_cast<intptr_t>(e->prev) << "\r\n";
	ss << "Twin: " << reinterpret_cast<intptr_t>(e->twin) << "\r\n";
	ss << "Face: " << reinterpret_cast<intptr_t>(e->adjacent_face) << "\r\n";
	ss << "Source: " << reinterpret_cast<intptr_t>(e->source) << "\r\n";

	*get<0>(textctrls[textctrl_meshdatastructure]) << ss.str(); //->SetLabel(wxString(ss.str()));
	old_dsinfo = static_cast<void *>(e);
}

void myFrameGUI::updateDataStructureInfo(myFace *f)
{
	if (f == nullptr)
	{
		//get<0>(textctrls[textctrl_meshdatastructure])->SetLabel(wxString(""));
		return;
	}

	if ( static_cast<myFace *>(old_dsinfo) == f) return;

	std::stringstream ss;
	ss << "-----FACE-----------------\r\n";
	ss << std::hex << std::noshowbase;
	ss << "Face: " << reinterpret_cast<intptr_t>(f) << "\r\n";
	ss << "Adjacent_edge: " << reinterpret_cast<intptr_t>(f->adjacent_halfedge) << "\r\n";

	*get<0>(textctrls[textctrl_meshdatastructure]) << ss.str(); //->SetLabel(wxString(ss.str()));
	old_dsinfo = static_cast<void *>(f);
}

void myFrameGUI::updateIntersectionStatistics(std::tuple<int, int, int, int> & stats)
{
	*get<0>(textctrls[textctrl_bvh_stats]) << "Faces tested for intersection: " << get<0>(stats) << "\r\n";
	*get<0>(textctrls[textctrl_bvh_stats]) << "Faces passed intersection test: " << get<1>(stats) << "\r\n";
	*get<0>(textctrls[textctrl_bvh_stats]) << "Boxes tested for intersection: " << get<2>(stats) << "\r\n";
	*get<0>(textctrls[textctrl_bvh_stats]) << "Boxes passed intersection test: " << get<3>(stats) << "\r\n";
	*get<0>(textctrls[textctrl_bvh_stats]) << "--------------\r\n";
}

