#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Image.H>
#include <FL/x.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Round_Button.H>

#include <vector>
#include <stdarg.h>
#include <QRDataBlocks.h>



class QRWindow : public Fl_Widget, public IGraphic
{
public:
    QRWindow( int X, int Y, int W, int H ) : Fl_Widget( X, Y, W, H )
    {
    }

    void draw()
    {
        if ( mRender )
        {
            mRender->mGraph = this;
            mRender->renderCurrentBlock();
        }
    }
    void drawRect( size_t left, size_t top, size_t w, size_t h, unsigned char colorR, unsigned char colorG, unsigned char colorB ) override
    {
        fl_rectf( left + x(), top + y(), w, h, colorR, colorG, colorB );
    }
    size_t getWidth() const override
    {
        return w();
    }
    size_t getHeight() const override
    {
        return h();
    }

    // return 1 when handled .
    int handle( int event ) override
    {
        // Need to respond to FOCUS events in order to handle keyboard
        if ( event == FL_FOCUS )
            return 1;
        if ( event == FL_UNFOCUS )
            return 1;

        // Now define responses to KEY DOWN events
        if ( event == FL_KEYBOARD )
            return 1;

        // And we might also need to response to KEY release
        if ( event == FL_KEYUP )
        {
            int key = Fl::event_key();

            if ( key == FL_Enter )
                key = '\n';
            if ( key > 0 && key <= 127 )
                mRender->onKeyEvent( key );
            //            myOutputBox->value( "keyup" );
            return 1; // to indicate we used the key event
        }

        return 0; // we had no interest in the event
    }
    QrRenderControl *mRender = nullptr;
};

struct App : public QRUIControl
{
    Fl_Window *mMainWindow;
    Fl_Button *mBtnStart;
    QRWindow *mQrWindow;
    Fl_Native_File_Chooser *mFileChooser;
    Fl_Box *mLabel;

    Fl_Round_Button *mChkBinMode;

    QrRenderControl mRender;
    bool mStopped = true;

    std::vector<char> mBuf;

    App() : mBuf( 256 )
    {
    }

    static void timer_cb( void *v )
    {
        ( (App *)v )->onTimerEvent();
    }
    static void start_cb( Fl_Widget *w, void *v )
    {
        ( (App *)v )->onBtnStart();
    }
    static void open_cb( Fl_Widget *w, void *v )
    {
        ( (App *)v )->onBtnOpen();
    }
    static void exit_cb( Fl_Widget *w, void *v )
    {
        ( (App *)v )->close();
    }
    static void binmode_cb( Fl_Widget *w, void *v )
    {
        ( (App *)v )->setUIFocus();
    }

    void print( const char *fmt, ... ) override
    {
        va_list args;
        va_start( args, fmt );
        vsnprintf( &mBuf[0], mBuf.size(), fmt, args );
        va_end( args );
        mLabel->copy_label( &mBuf[0] );
    }
    void enableQRTimer( double seconds ) override
    {
        Fl::repeat_timeout( seconds, timer_cb, this );
    }
    void playStatusChanged( bool active ) override
    {
        mBtnStart->copy_label( active ? "Stop" : "Start" );
        if ( active )
            mQrWindow->redraw();
    }
    void redraw() override
    {
        mQrWindow->redraw();
        mLabel->redraw_label();
    }
    void close() override
    {
        exit( 0 );
    }
    void onBtnOpen()
    {
        mFileChooser->title( "Open" );
        mFileChooser->type( Fl_Native_File_Chooser::BROWSE_FILE ); // only picks files that exist
        switch ( mFileChooser->show() )
        {
        case -1:
            break; // Error
        case 1:
            break; // Cancel
        default: // Choice
            mFileChooser->preset_file( mFileChooser->filename() );
            print( "Openned file %s", mFileChooser->filename() );
            startEncodeFile( mFileChooser->filename() );
            break;
        }
        setUIFocus();
    }
    void onTimerEvent()
    {
        redraw();
        if ( mRender.incBlock( true ) )
            mRender.prepareBlock();
        setUIFocus();
    }
    void onBtnStart()
    {
        setUIFocus();
        mRender.flipActive();
    }
    void startEncodeFile( const char *filename )
    {
        try
        {
            mRender.mQrData.mIsBinaryMode = mChkBinMode->value(); // check bin/base64 mode.
            mRender.init( filename );
            mRender.prepareBlock();
            mQrWindow->redraw();
        }
        catch ( std::runtime_error &err )
        {
            print( "ERROR! Open file :%s", err.what() );
            mRender.mBlockIdx = -1;
            return;
        }
    }
    void setUIFocus()
    {
        mQrWindow->take_focus();
    }

    void init( int argc, char **argv )
    {
        const int YGAP = 5, XGAP = 5;
        const int WIN_W = 950, WIN_H = 950;
        const int LABEL_H = 20;
        int posX = 0, posY = 0; // accumulative var
        int w, h; // temp var
        mMainWindow = new Fl_Window( WIN_W, WIN_H );

        // row 1
        Fl_Button *btnOpen = new Fl_Button( posX = 10, posY += YGAP, w = 130, h = 25, "Open File" );
        posX += w;
        btnOpen->callback( open_cb, this );
        btnOpen->tooltip( "Open a file to start." );

        mBtnStart = new Fl_Button( posX + 20, posY, w = 130, h = 25, "Start" );
        posX += 20 + w;
        mBtnStart->callback( start_cb, this );

        auto btnExit = new Fl_Button( posX + 20, posY, w = 130, h = 25, "Exit" );
        posX += 20 + w;
        btnExit->callback( exit_cb, this );

        mChkBinMode = new Fl_Round_Button( posX + 20, posY, w = 130, h = 25, "Bin Mode" );
        posX += 20 + w;
        mChkBinMode->value( 1 );
        mChkBinMode->callback( binmode_cb, this );
        mChkBinMode->tooltip( "Open new file to take effect!" );

        posY += h;

        // row 2
        mQrWindow = new QRWindow( YGAP, posY + YGAP, w = WIN_W - 2 * XGAP, h = WIN_H - posY - 2 * YGAP - LABEL_H );
        posY += h + YGAP;

        // row 3
        mLabel = new Fl_Box( XGAP, posY + YGAP, w = WIN_W - 2 * XGAP, h = LABEL_H, "Please open file to start....." ); //

        // Initialize the file chooser
        mFileChooser = new Fl_Native_File_Chooser();

        mMainWindow->resizable( mMainWindow );
        mMainWindow->end();
        mMainWindow->show( argc, argv );

        //------- set dependancies ------------
        mRender.mGraph = mQrWindow;
        mRender.mUIC = this;
        mQrWindow->mRender = &mRender;
        setUIFocus();
    }

    int run()
    {
        Fl::add_timeout( 1.0f / 2, timer_cb, this );
        return Fl::run();
    }
};
int main( int argc, char **argv )
{
    App app;
    app.init( argc, argv );
    return app.run();
}
