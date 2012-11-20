#include <stdio.h>
#include <iostream>
#include <wx/wx.h>
using namespace std;

class Visualizer : public wxPanel
{
	public:
		Visualizer(wxFrame *parent, float **currentFramePtr);
		void paintEvent(wxPaintEvent& evt);
		void paintNow();
		void render( wxDC& dc );

	protected:
		void OnKeyDown(wxKeyEvent& event);
		void OnTimer(wxCommandEvent& event);

	private:
		wxStatusBar *m_stsbar;
		int height;
		int width;
		float **framePtr;
};

class VisApp : public wxApp
{
	public:
		void setFramePtr(float **framePtr);

	private:
		float **framePtr;
		wxFrame *frame;
		Visualizer *drawPane;
		bool render_loop_on;

	    bool OnInit();
		void onIdle(wxIdleEvent& evt);
		void activateRenderLoop(bool on);
};
