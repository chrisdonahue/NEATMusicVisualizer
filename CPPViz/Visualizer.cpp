#include "Visualizer.h"

Visualizer::Visualizer(wxFrame *parent, pthread_mutex_t &lock, pthread_cond_t &cond, wxImage* newest)
       : wxPanel(parent, wxID_ANY, wxDefaultPosition,
             wxDefaultSize, wxBORDER_NONE)
{
    m_stsbar = parent->GetStatusBar();
    imageAvailable = cond;
    newestImageLock = lock;
    newestImage = newest;

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
    // get size
    wxSize size = GetClientSize();
	width = size.GetWidth();
	height = size.GetHeight();

    // threadsafe access of new column
    pthread_mutex_lock(&newestImageLock);
    while (newestImage == NULL) {
        pthread_cond_wait(&imageAvailable);
    }
    void *nodeMem = malloc(sizeof(linkedNode));
    linkedNode *newnode = (linkedNode *) nodeMem;
    newnode->bitmap = new wxBitmap(*newestImage);
    pthread_mutex_unlock(&newestImageLock);

    // manage linked list
    if (currentListSize == 0) {
        newnode->next = NULL;
        newnode->prev = NULL;
        head = newNode;
        tail = newNode;
        currentListSize = 1;
    }
    else if (currentListSize < width) {
        head->prev = newnode;
        newnode->next = head;
        newnode->prev = NULL;
        head = newnode;
    }
    else {
        head->prev = newnode;
        newnode->next = head;
        newnode->prev = NULL;
        tail->prev->next = NULL;
        linkedNode *oldTail = tail;
        tail = oldTail->prev;
        free((void*) oldTail);
        head = newnode;
    }

    // bitmap might need to be const qq
    linkedNode *currentNode = head;
    for (unsigned x = 0; x < width; x++) {
        const wxPoint loc = wxPoint(x, 0);
        dc.DrawBitmap(currentNode->bitmap, loc, false);
        currentNode = currentNode->next;
    }
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
	
    drawPane = new Visualizer( (wxFrame*) frame, newestImageLock, imageAvailable, newestImage);
    sizer->Add(drawPane, 1, wxEXPAND);
	
    frame->SetSizer(sizer);
    frame->SetAutoLayout(true);
	
    frame->Show();
	activateRenderLoop(true);
    return true;
}

void VisApp:setThreadSafety(pthread_mutex_t &mutex, pthread_cond_t &cond, wxImage* newest)
{
    newestImageLock = *mutex;
    imageAvailable = *cond;
    newestImage = newest;
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
