#include "Visualizer.h"

Visualizer::Visualizer(wxFrame *parent, float **currentFramePtr)
       : wxPanel(parent, wxID_ANY, wxDefaultPosition,
             wxDefaultSize, wxBORDER_NONE)
{
    m_stsbar = parent->GetStatusBar();
	framePtr = currentFramePtr;

    Connect(wxEVT_PAINT, wxPaintEventHandler(Visualizer::paintEvent));
    //Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(Visualizer::OnKeyDown));
    //Connect(wxEVT_TIMER, wxCommandEventHandler(Visualizer::OnTimer));
}

void Visualizer::paintEvent(wxPaintEvent& evt)
{
    wxPaintDC dc(this);
    render(dc);
}
 
void Visualizer::paintNow()
{
    wxClientDC dc(this);
    render(dc);
}
 
void Visualizer::render( wxDC& dc )
{
    wxSize size = GetClientSize();
	int width = size.GetWidth();
	int height = size.GetHeight();
	unsigned char sigRed = (unsigned char) (**framePtr * 500);
	unsigned char sigGreen = (unsigned char) (**framePtr * 255);
	unsigned char sigBlue = (unsigned char) (**framePtr * 1000);
	
	wxImage *image = new wxImage(width, height, true);
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			image->SetRGB(x, y, sigRed, sigGreen, sigBlue);
		}
	}
	const wxBitmap *bitmap = new wxBitmap(*image);
	const wxPoint loc = wxPoint(0,0);
	
	dc.DrawBitmap(*bitmap, loc, false);
}

void Visualizer::OnKeyDown(wxKeyEvent& event)
{/*
    if (!isStarted || curPiece.GetShape() == NoShape) {  
        event.Skip();
        return;
    }

    int keycode = event.GetKeyCode();

    if (keycode == 'p' || keycode == 'P') {
	Pause();
        return;
    }

    if (isPaused)
        return;

    switch (keycode) {
    case WXK_LEFT:
        TryMove(curPiece, curX - 1, curY);
        break;
    case WXK_RIGHT:
        TryMove(curPiece, curX + 1, curY);
        break;
    case WXK_DOWN:
        TryMove(curPiece.RotateRight(), curX, curY);
        break;
    case WXK_UP:
        TryMove(curPiece.RotateLeft(), curX, curY);
        break;
    case WXK_SPACE:
        DropDown();
        break;
    case 'd':
        OneLineDown();
        break;
    case 'D':
        OneLineDown();
        break;
    default:
        event.Skip();
    }*/
}

void Visualizer::OnTimer(wxCommandEvent& event) {}

bool VisApp::OnInit()
{
	render_loop_on = false;

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    frame = new wxFrame((wxFrame *) NULL, -1,  wxT("Hyperviz"), wxPoint(-1,-1), wxSize(100,100));
	
    drawPane = new Visualizer( (wxFrame*) frame, framePtr);
    sizer->Add(drawPane, 1, wxEXPAND);
	
    frame->SetSizer(sizer);
    frame->SetAutoLayout(true);
	
    frame->Show();
	activateRenderLoop(true);
    return true;
}

void VisApp::setFramePtr(float **frames)
{
	framePtr = frames;
}

void VisApp::activateRenderLoop(bool on)
{
    if(on && !render_loop_on)
    {
        Connect( wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(VisApp::onIdle) );
        render_loop_on = true;
    }
    else if(!on && render_loop_on)
    {
        Disconnect( wxEVT_IDLE, wxIdleEventHandler(VisApp::onIdle) );
        render_loop_on = false;
    }
}

void VisApp::onIdle(wxIdleEvent& evt)
{
    if(render_loop_on)
    {
        drawPane->paintNow();
        evt.RequestMore(); // render continuously, not only once on idle
    }
}
