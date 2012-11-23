#include <stdio.h>
#include <iostream>
#include <wx/wx.h>
using namespace std;

struct linkedNode {
    wxBitmap* bitmap;
    linkedNode* prev;
    linkedNode* next;
}

class Visualizer : public wxPanel
{
	public:
		Visualizer(wxFrame *parent, pthread_mutex_t &lock, pthread_cond_t &cond, wxImage* newest);
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

        pthread_cond_t imageAvailable;
        pthread_mutex_t newestImageLock;
        wxImage* newestImage;

        int currentListSize;
        linkedNode *head;
        linkedNode *tail;
};

class VisApp : public wxApp
{
	public:
		void setThreadSafety(pthread_mutex_t &lock, pthraed_cond_t &cond, wxImage* newest);

	private:
		float **framePtr;
		wxFrame *frame;
		Visualizer *drawPane;
		bool render_loop_on;
        
	    bool OnInit();
		void onIdle(wxIdleEvent& evt);
		void activateRenderLoop(bool on);
};
