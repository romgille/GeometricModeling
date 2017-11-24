#include "myFrameGUI.h"
#include "default_constants.h"
#include "wx/wx.h"
#include "wx/log.h"
#include "wx/config.h"


using namespace std;

class MyApp : public wxApp
{
public:
	bool OnInit() override
	{
		if (!wxApp::OnInit())
			return false;

		wxConfigBase *pConfig = wxConfigBase::Get();
		
		mainframe = new myFrameGUI(nullptr, wxT("IN4I12-Meshing"), wxDefaultPosition, wxSize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT));
		
		wxString filename;
		if (mainframe->rememberSettings && pConfig->Read(to_string(myFrameGUI::_CURRENTMESH), &filename))
			mainframe->readMesh( filename.ToStdString() );
		else mainframe->readMesh("models/dolphin.obj");

		mainframe->Show(true);

		return true;
	}

	int OnExit() override
	{
		delete wxConfigBase::Set((wxConfigBase *)nullptr);
		return 0;
	}

	myFrameGUI *mainframe;
};

IMPLEMENT_APP(MyApp)

int main(int argc, char* argv[])
{
	return WinMain(::GetModuleHandle(nullptr), nullptr, nullptr, SW_SHOWNORMAL);
}
