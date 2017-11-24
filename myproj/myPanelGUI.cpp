#include "myPanelGUI.h"

#include "myFrameGUI.h"
#include "myGLCanvasGUI.h"
#include "default_constants.h"
#include <sstream>
#include <ctime>
#include "helper_functions.h"
#include "fstream"
#include "wx/config.h"
#include <chrono>
#include "errors.h"

using namespace std;



myPanelGUI::myPanelGUI(wxFrame * frame, int x, int y, int w, int h)
	: wxPanel(frame, wxID_ANY, wxPoint(x, y), wxSize(w, h), wxFULL_REPAINT_ON_RESIZE) //wxSize(w, h) or wxDefaultSize
{
	wxSizer *sizer_tabpanels, *sizerPanel, *sizer_inside;
	wxStaticBoxSizer *wrapping_sizer;
	wxGridBagSizer *grid;
	wxFont font;
	myPanelGUI::myCtrlId ID;
	
	wxConfigBase *pConfig = wxConfigBase::Get();
	pConfig->SetPath(wxT("/Controls"));
	
	sizerPanel = new wxBoxSizer(wxHORIZONTAL);


	tabs = new wxNotebook(this, wxNB_FIXEDWIDTH);
	tabs->SetPadding(wxSize(14, 14));

	for (int id = VIEW; id != _PANEL; id++)
		get<0>(tabpanels[static_cast<myTabId>(id)]) = new wxPanel(tabs);
	 
	for (auto it = checkboxes.begin(); it != checkboxes.end(); ++it)
	{
		myCtrlId _id = it->first;
		myTabId _tab = get<1>(it->second);
		wxString _label = wxString(get<2>(it->second));
		bool _value = get<3>(it->second);
		bool _savedvalue;
		get<0>(it->second) = new wxCheckBox(get<0>(tabpanels[_tab]), _id, _label );

		if (rememberSettings && pConfig->Read(to_string(_id), &_savedvalue))
			get<0>(it->second)->SetValue(_savedvalue); 
		else get<0>(it->second)->SetValue(_value);
		
		Bind(wxEVT_CHECKBOX, &myPanelGUI::eventHandling, this, _id);
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

		Bind(wxEVT_SPINCTRL, &myPanelGUI::eventHandling, this, _id);
	}

	for (auto it = buttons.begin(); it != buttons.end(); ++it)
	{
		myCtrlId _id = it->first;
		myTabId _tab = get<1>(it->second);
		wxString _label = wxString(get<2>(it->second));
		get<0>(it->second) = new wxButton(get<0>(tabpanels[_tab]), _id, _label);
		Bind(wxEVT_BUTTON, &myPanelGUI::eventHandling, this, _id);
	}
	
	for (auto it = textctrls.begin(); it != textctrls.end(); ++it)
	{
		myCtrlId _id = it->first;
		myTabId _tab = get<1>(it->second);
		wxString _label = wxString(get<2>(it->second));
		long style = get<3>(it->second);
		long _fontsize = get<4>(it->second);
	    get<0>(it->second) = new wxTextCtrl(get<0>(tabpanels[_tab]), _id, _label, wxDefaultPosition, wxDefaultSize, style);
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
	Bind(wxEVT_RADIOBOX, &myPanelGUI::eventHandling, this, ID);
	sizer_tabpanels->Add(get<0>(radioboxes[ID]), 0, wxEXPAND, 5);

	wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[VIEW]), wxID_ANY, wxT("Camera"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
	sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
	wrapping_sizer->Add(get<0>(buttons[button_resetcamera]), wxSizerFlags().Expand());
		
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
	wrapping_sizer->Add(get<0>(buttons[button_checkall]), wxSizerFlags().Expand());

	wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[CHECKMESH]), wxID_ANY, wxT("Twins"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
	sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
	wrapping_sizer->Add(get<0>(buttons[button_checkhalfedges_twinsselected]), wxSizerFlags().Expand());

	wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[CHECKMESH]), wxID_ANY, wxT("Next/Prev"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
	sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
	wrapping_sizer->Add(get<0>(buttons[button_checkhalfedges_nextprevselected]), wxSizerFlags().Expand());

	wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[CHECKMESH]), wxID_ANY, wxT("Sources"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
	sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
	wrapping_sizer->Add(get<0>(buttons[button_checkhalfedges_sourceselected]), wxSizerFlags().Expand());

	wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[CHECKMESH]), wxID_ANY, wxT("Vertex Fans"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
	sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
	wrapping_sizer->Add(get<0>(buttons[button_checkvertices_fansselected]), wxSizerFlags().Expand());

	wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[CHECKMESH]), wxID_ANY, wxT("Face Boundaries"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
	sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
	wrapping_sizer->Add(get<0>(buttons[button_checkfaces_boundaryedgesselected]), wxSizerFlags().Expand());

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
	wrapping_sizer->Add(get<0>(buttons[button_clearselected]), wxSizerFlags().Expand());

	ID = listbox_allowmultipleselectedpoints;
	wxString allowselectedchoice[] = { wxT("Single object"), wxT("Multiple objects") };
	get<0>(listboxes[ID]) = new wxListBox(get<0>(tabpanels[get<1>(listboxes[ID])]), ID,
		wxDefaultPosition, wxDefaultSize, 2, allowselectedchoice, wxLB_SINGLE | wxLB_ALWAYS_SB);
	get<0>(listboxes[ID])->SetSelection(get<3>(listboxes[ID]));
	Bind(wxEVT_LISTBOX, &myPanelGUI::eventHandling, this, ID);
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
	wrapping_sizer->Add(get<0>(buttons[button_normalize]), wxSizerFlags().Expand());

	wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[EDITING]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
	sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
	wrapping_sizer->Add(get<0>(buttons[button_triangulate]), wxSizerFlags().Expand());

	wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[EDITING]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
	sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
	wrapping_sizer->Add(get<0>(buttons[button_fractalize]), wxSizerFlags().Expand());

	//INFLATE SLIDER
	wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[EDITING]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
	sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);

	wrapping_sizer->Add(get<0>(spinctrls[spinctrl_inflate]), 0, wxGROW | wxALL, 5);
	wrapping_sizer->Add(get<0>(buttons[button_inflate]), wxSizerFlags().Expand());

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

	wrapping_sizer->Add(get<0>(buttons[button_cuttingwithplane]), wxSizerFlags().Expand());

	get<0>(tabpanels[EDITING])->SetSizer(sizer_tabpanels);
/*------------------------------------------------*/


/*----SUBDIVISION--------------------------------------------*/

	sizer_tabpanels = new wxBoxSizer(wxHORIZONTAL);

	wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[SUBDIVISION]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
	sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
	wrapping_sizer->Add(get<0>(buttons[button_splithalfedge]), wxSizerFlags().Expand());

	wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[SUBDIVISION]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
	sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
	wrapping_sizer->Add(get<0>(buttons[button_splitface]), wxSizerFlags().Expand());
		
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
	wrapping_sizer->Add(get<0>(buttons[button_contracthalfedge]), wxSizerFlags().Expand());

	wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[SIMPLIFICATION]), wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
	sizer_tabpanels->Add(wrapping_sizer, 0, wxGROW | wxALL, 5);
	wrapping_sizer->Add(get<0>(buttons[button_contractface]), wxSizerFlags().Expand());

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


	Bind(wxEVT_LISTBOX, &myPanelGUI::eventHandling, this, ID);
	wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[BVH]), wxID_ANY, wxT("Splitting Method"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
	wrapping_sizer->Add(get<0>(listboxes[ID]), wxSizerFlags().Border().Center());
	sizer_tabpanels->Add(wrapping_sizer, 0, wxEXPAND, 5);

		
	wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(get<0>(tabpanels[BVH]), wxID_ANY, wxT("Intersection statistics"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
	wrapping_sizer->Add(get<0>(textctrls[textctrl_bvh_stats]), wxSizerFlags().Expand());
	get<0>(textctrls[textctrl_bvh_stats])->SetInitialSize(wxSize(wxDefaultSize.GetWidth(), h * 2));
	sizer_tabpanels->Add(wrapping_sizer, wxSizerFlags().Expand());


	get<0>(tabpanels[BVH])->SetSizer(sizer_tabpanels);
/*------------------------------------------------*/

	

	
	/****** ADDING IN ALL TABS******************/
	font = tabs->GetFont();
	font.SetPointSize(14);
	font.SetWeight(wxFONTWEIGHT_BOLD);
	tabs->SetFont(font);

	for (int id = VIEW; id != _PANEL; id++)
	{
		wxPanel *_p = get<0>(tabpanels[static_cast<myTabId>(id)]);
		wxString _l = wxString(get<1>(tabpanels[static_cast<myTabId>(id)]));
		_p->SetBackgroundColour(wxColour(188, 188, 188));
		tabs->AddPage(_p, _l);
	}
	sizerPanel->Add(tabs, wxGROW | wxALL);
	/*------------------------------------------------*/




	wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Data-Structure Information"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
	wrapping_sizer->Add(get<0>(textctrls[textctrl_meshdatastructure]), wxSizerFlags().Expand());
	get<0>(textctrls[textctrl_meshdatastructure])->SetInitialSize(wxSize(wxDefaultSize.GetWidth(), h * 3));
	sizerPanel->Add(wrapping_sizer, wxGROW | wxALL);




		
	wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Mesh Information"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
	
	wrapping_sizer->Add(get<0>(textctrls[textctrl_meshinfovertices]), wxSizerFlags().Expand());
	wrapping_sizer->Add(get<0>(textctrls[textctrl_meshinfoedges]), wxSizerFlags().Expand());
	wrapping_sizer->Add(get<0>(textctrls[textctrl_meshinfofaces]), wxSizerFlags().Expand());
	wrapping_sizer->Add(get<0>(textctrls[textctrl_fpsinfo]), wxSizerFlags().Expand());
	get<0>(textctrls[textctrl_fpsinfo])->SetForegroundColour(wxColour(255, 0, 0));
	sizerPanel->Add(wrapping_sizer, wxGROW | wxALL);

	

	
	wrapping_sizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Runtime"), wxDefaultPosition, wxDefaultSize), wxVERTICAL);
	get<0>(textctrls[textctrl_runningtime])->SetInitialSize(wxSize(wxDefaultSize.GetWidth(), h * 3));
	wrapping_sizer->Add(get<0>(textctrls[textctrl_runningtime]), wxSizerFlags(0).Expand());
	sizerPanel->Add(wrapping_sizer, wxGROW | wxALL);


	SetSizerAndFit(sizerPanel);


	fpsTimer = new wxTimer();
	fpsTimer->SetOwner(this);
	fpsTimer->Start(1);
	Bind(wxEVT_TIMER, &myPanelGUI::computeFPS, this, wxID_ANY);
	fps_clock = std::chrono::high_resolution_clock::now();
}
 

myPanelGUI::~myPanelGUI()
{
	wxConfigBase *pConfig = wxConfigBase::Get();
	pConfig->SetPath(wxT("/Controls"));

	for (auto it = checkboxes.begin(); it != checkboxes.end(); ++it)
		pConfig->Write(to_string(it->first), get<0>(it->second)->GetValue());
	for (auto it = radioboxes.begin(); it != radioboxes.end(); ++it)
		pConfig->Write(to_string(it->first), get<0>(it->second)->GetSelection());
	for (auto it = listboxes.begin(); it != listboxes.end(); ++it)
		pConfig->Write(to_string(it->first), get<0>(it->second)->GetSelection());
	for (auto it = spinctrls.begin(); it != spinctrls.end(); ++it)
		pConfig->Write(to_string(it->first), get<0>(it->second)->GetValue());	

	if (m_canvas->mesh != nullptr)
		pConfig->Write(to_string(id_MAX), wxString(m_canvas->mesh->name));

	fpsTimer->Stop();
}

void myPanelGUI::updateMeshInfo()
{
	if (m_canvas == nullptr) return;

	wxString s;
	std::stringstream ss;

	ss.imbue(std::locale(""));
	ss << std::fixed << m_canvas->mesh->vertices.size();
	s << "Vertices:  " << ss.str();
	get<0>(textctrls[textctrl_meshinfovertices])->SetLabel(s);

	ss.str(std::string()); ss.clear();
	ss.imbue(std::locale(""));
	ss << std::fixed << m_canvas->mesh->halfedges.size();
	s.clear();
	s << "Halfedges: " << ss.str();
	get<0>(textctrls[textctrl_meshinfoedges])->SetLabel(s);

	ss.str(std::string()); ss.clear();
	ss.imbue(std::locale(""));
	ss << std::fixed << m_canvas->mesh->faces.size();
	s.clear();
	s << "Faces:    " << ss.str();
	get<0>(textctrls[textctrl_meshinfofaces])->SetLabel(s);

	s = "";
	s << "Points selected: " << m_canvas->points_todraw["SELECTED"].size();
	get<0>(staticboxes[staticbox_numpoints])->SetLabel(s);

	s = "";
	s << "Vertices selected: " << m_canvas->vertices_todraw["SELECTED"].size();
	get<0>(staticboxes[staticbox_numvertices])->SetLabel(s);

	s = "";
	s << "Halfedges selected: " << m_canvas->edges_todraw["SELECTED"].size();
	get<0>(staticboxes[staticbox_numedges])->SetLabel(s);

	s = "";
	s << "Faces selected: " << m_canvas->faces_todraw["SELECTED"].size();
	get<0>(staticboxes[staticbox_numfaces])->SetLabel(s);
}

void myPanelGUI::updateDataStructureInfo(myVertex *v) 
{
	if (v == nullptr)
	{
		get<0>(textctrls[textctrl_meshdatastructure])->SetLabel(wxString(""));
		return;
	}

	std::stringstream ss;
	ss << std::hex << std::noshowbase;
	ss << "Vertex: " << reinterpret_cast<intptr_t>(v) << "\r\n";
	ss << "Originof: " << reinterpret_cast<intptr_t>(v->originof) << "\r\n";
	ss << "Point: " << reinterpret_cast<intptr_t>(&v->point) << "\r\n";

	get<0>(textctrls[textctrl_meshdatastructure])->SetLabel(wxString(ss.str()));
}

void myPanelGUI::updateDataStructureInfo(myHalfedge *e)
{
	if (e == nullptr)
	{
		get<0>(textctrls[textctrl_meshdatastructure])->SetLabel(wxString(""));
		return;
	}

	std::stringstream ss;
	ss << std::hex << std::noshowbase;
	ss << "Edge: " << reinterpret_cast<intptr_t>(e) << "\r\n";
	ss << "Next: " << reinterpret_cast<intptr_t>(e->next) << "\r\n";
	ss << "Prev: " << reinterpret_cast<intptr_t>(e->prev) << "\r\n";
	ss << "Twin: " << reinterpret_cast<intptr_t>(e->twin) << "\r\n";
	ss << "Face: " << reinterpret_cast<intptr_t>(e->adjacent_face) << "\r\n";
	ss << "Source: " << reinterpret_cast<intptr_t>(e->source) << "\r\n";

	get<0>(textctrls[textctrl_meshdatastructure])->SetLabel(wxString(ss.str()));
}

void myPanelGUI::updateDataStructureInfo(myFace *f)
{
	if (f == nullptr)
	{
		get<0>(textctrls[textctrl_meshdatastructure])->SetLabel(wxString(""));
		return;
	}

	std::stringstream ss;
	ss << std::hex << std::noshowbase;
	ss << "Face: " << reinterpret_cast<intptr_t>(f) << "\r\n";
	ss << "Adjacent_edge: " << reinterpret_cast<intptr_t>(f->adjacent_halfedge) << "\r\n";

	get<0>(textctrls[textctrl_meshdatastructure])->SetLabel(wxString(ss.str()));
}

void myPanelGUI::updateIntersectionStatistics(std::tuple<int, int, int, int> & stats)
{
	*get<0>(textctrls[myPanelGUI::textctrl_bvh_stats]) << "Faces tested for intersection: " << get<0>(stats) << "\r\n";
	*get<0>(textctrls[myPanelGUI::textctrl_bvh_stats]) << "Faces passed intersection test: " << get<1>(stats) << "\r\n";
	*get<0>(textctrls[myPanelGUI::textctrl_bvh_stats]) << "Boxes tested for intersection: " << get<2>(stats) << "\r\n";
	*get<0>(textctrls[myPanelGUI::textctrl_bvh_stats]) << "Boxes passed intersection test: " << get<3>(stats) << "\r\n";
	*get<0>(textctrls[myPanelGUI::textctrl_bvh_stats]) << "--------------\r\n";
}


void myPanelGUI::eventHandling(wxCommandEvent &event)
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
			if (!get<0>(checkboxes[ static_cast<myCtrlId>(event.GetId()) ])->GetValue())
			{
				m_canvas->clear_todraw("DATASTRUCTURE");
				get<0>(textctrls[textctrl_meshdatastructure])->SetLabel(wxString(""));
			}
			break;
		}
		case button_bvh_construct:
		{
			if (m_canvas->bvh) delete m_canvas->bvh;
			m_canvas->bvh = new myBVH();
			m_canvas->bvh->buildBVH(m_canvas->mesh, static_cast<myBVH::splittingMethod>(get<0>(listboxes[listbox_bvh_splittingmethod])->GetSelection()) );
			get<0>(spinctrls[spinctrl_bvh_depth])->SetValue(0);
			break;
		}
		case button_bvh_reset:
		{
			if (m_canvas->bvh) delete m_canvas->bvh;
			m_canvas->bvh = nullptr;
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
			if (m_canvas->reconstructed_mesh) delete m_canvas->reconstructed_mesh;
			m_canvas->reconstructed_mesh = new myMesh(m_canvas->mesh->vertices);
			m_canvas->points_todraw["POLES-POINTS"] = m_canvas->reconstructed_mesh->voronoiReconstruction( );
			break;
		}
		case button_voronoidelaunay:
		{
			vector<glm::uvec4> df;
			vector<glm::vec3> input_points;
			for (size_t i = 0; i < m_canvas->mesh->vertices.size(); i++)
				input_points.push_back(m_canvas->mesh->vertices[i]->point);
			computeVoronoiDelaunay(input_points, m_canvas->points_todraw["VORONOI-POINTS"], df);

			m_canvas->segments_todraw["DELAUNAY-EDGES"].clear();
			for (size_t i = 0; i < df.size(); i++)
				for (size_t j = 0; j<4; j++)
					m_canvas->segments_todraw["DELAUNAY-EDGES"].push_back( mySegment( input_points[df[i][j]], input_points[df[i][(j+1)%4]] ) );
			break;
		}
		case button_resetcamera:
		{
			m_canvas->camera->reset();
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
			m_canvas->mesh->subdivisionCatmullClark();
			m_canvas->mesh->computeNormals();
			main_window->updateMeshInfo();
			break;
		}
		case button_catmullclark_newpoints:
		{
			m_canvas->mesh->subdivisionCatmullClark_createNewPoints(
				m_canvas->points_todraw["CATMULLCLARK-FACEPOINTS"],
				m_canvas->points_todraw["CATMULLCLARK-EDGEPOINTS"],
				m_canvas->points_todraw["CATMULLCLARK-VERTEXPOINTS"]);
			break;
		}
		case button_splitface:
		{
			if (m_canvas->faces_todraw["SELECTED"].size() && m_canvas->points_todraw["SELECTED"].size())
			{
				myFace *f = m_canvas->faces_todraw["SELECTED"].back();
				glm::vec3 p = m_canvas->points_todraw["SELECTED"].back();
				m_canvas->mesh->splitFaceTRIS(f, p);
				m_canvas->mesh->computeNormals();
				main_window->updateMeshInfo();
			}
			else cout << "Unable to split face.\n";
			break;
		}
		case button_splithalfedge:
		{
			if (m_canvas->edges_todraw["SELECTED"].size() && m_canvas->points_todraw["SELECTED"].size())
			{
				myHalfedge *e = m_canvas->edges_todraw["SELECTED"].back();
				glm::vec3 p = m_canvas->points_todraw["SELECTED"].back();
				m_canvas->mesh->splitEdge(e, p);
				m_canvas->mesh->computeNormals();
				main_window->updateMeshInfo();
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
			if ( (get<0>(listboxes[static_cast<myCtrlId>(event.GetId())])->GetSelection() == 0) )
				m_canvas->clear_todraw("SELECTED");
			break;
		}
		case radiobox_outputwindow:
		{
			main_window->showhideLogWindow(get<0>(radioboxes[ static_cast<myCtrlId>(event.GetId()) ])->GetSelection() == 1 );
			break;
		}
		case button_checkhalfedges_twinsselected:
		{
			m_canvas->mesh->_checkhalfedges_twins(m_canvas->edges_todraw["SELECTED"]);
			break;
		}
		case button_checkhalfedges_nextprevselected:
		{
			m_canvas->mesh->_checkhalfedges_nextprev(m_canvas->edges_todraw["SELECTED"]);
			break;
		}
		case button_checkhalfedges_sourceselected:
		{
			m_canvas->mesh->_checkhalfedges_source(m_canvas->edges_todraw["SELECTED"]);
			break;
		}
		case button_checkvertices_fansselected:
		{
			m_canvas->mesh->_checkvertices_fans(m_canvas->vertices_todraw["SELECTED"]);
			break;
		}
		case button_checkfaces_boundaryedgesselected:
		{
			m_canvas->mesh->_checkfaces_boundaryedges(m_canvas->faces_todraw["SELECTED"]);
			break;
		}
		case button_checkall:
		{
			m_canvas->mesh->checkMesh();
			break;
		}
		case button_sharpen:
		{
			m_canvas->mesh->sharpen(get<0>(spinctrls[spinctrl_sharpen])->GetValue() / 10.0f);
			m_canvas->mesh->computeNormals();
			main_window->updateMeshInfo();
			break;
		}
		case button_smoothen:
		{
			m_canvas->mesh->smoothen(get<0>(spinctrls[spinctrl_smoothen])->GetValue() / 10.0f);
			m_canvas->mesh->computeNormals();
			main_window->updateMeshInfo();
			break;
		}
		case button_inflate:
		{
			m_canvas->mesh->inflate(get<0>(spinctrls[spinctrl_inflate])->GetValue() / 1000.0f);
			m_canvas->mesh->computeNormals();
			main_window->updateMeshInfo();
			break;
		}
		case button_triangulate:
		{
			m_canvas->mesh->triangulate();
			m_canvas->mesh->computeNormals();
			main_window->updateMeshInfo();
			break;
		}
		case button_fractalize:
		{
			m_canvas->mesh->fractalize( );
			m_canvas->mesh->computeNormals();
			main_window->updateMeshInfo();
			break;
		}
		case button_normalize:
		{
			m_canvas->mesh->normalize();
			m_canvas->mesh->computeNormals();
			main_window->updateMeshInfo();
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
			m_canvas->clear_todraw("SELECTED");
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
	
	m_canvas->Refresh(false);
}


void myPanelGUI::startTimer(wxString event_name)
{
	get<0>(textctrls[textctrl_runningtime])->SetBackgroundColour(wxColour(255, 0, 0));

	*get<0>(textctrls[textctrl_runningtime]) << "Event: " << event_name << "\r\n";

	this->Refresh();
	this->Update();

	timer_clock = std::chrono::high_resolution_clock::now();
}

void myPanelGUI::endTimer()
{
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - timer_clock;

	get<0>(textctrls[textctrl_runningtime])->SetBackgroundColour(wxColour(255, 255, 255));

	*get<0>(textctrls[textctrl_runningtime]) << "Time: " << elapsed.count() << " millisecs.\r\n";

	this->Refresh();
	this->Update();
}


void myPanelGUI::onKeyboard(wxKeyEvent& event)
{
	cout << "panel: Key pressed!\n";
}


void myPanelGUI::computeFPS(wxTimerEvent &event)
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