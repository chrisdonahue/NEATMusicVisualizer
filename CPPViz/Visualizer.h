#include <stdio.h>
#include <iostream>
#include <wx/wx.h>
#include <pthread.h>

class Visualizer : public wxPanel
{
    struct linkedNode {
        const wxBitmap* bitmap;
        linkedNode* prev;
        linkedNode* next;
    };
	
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
		void setThreadSafety(pthread_mutex_t &lock, pthread_cond_t &cond, wxImage* newest);

	private:
		wxFrame *frame;
		Visualizer *drawPane;
		bool render_loop_on;

        pthread_cond_t imageAvailable;
        pthread_mutex_t newestImageLock;
        wxImage* newestImage;

        bool OnInit();
		void onIdle(wxIdleEvent& evt);
		void activateRenderLoop(bool on);
};
