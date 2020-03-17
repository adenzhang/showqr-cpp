#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Image.H>
#include <FL/x.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Native_File_Chooser.H>
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
            mRender->showNextQR();
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
    QrRenderControl *mRender = nullptr;
};

struct App : public QRUIControl
{
    Fl_Window *mMainWindow;
    Fl_Button *mBtnStart;
    QRWindow *mQrWindow;
    Fl_Native_File_Chooser *mFileChooser;
    Fl_Box *mLabel;

    QrRenderControl mRender;
    bool mStopped = true;

    std::vector<char> mBuf;

    double mPlayrateFPS = 2;

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
        takeFocus();
    }
    void onTimerEvent()
    {
        takeFocus();
        mQrWindow->redraw();
        mLabel->redraw_label();
    }
    void onBtnStart()
    {
        takeFocus();
        mRender.flipActive();
    }
    void startEncodeFile( const char *filename )
    {
        try
        {
            mRender.init( filename );
            mQrWindow->redraw();
        }
        catch ( std::runtime_error &err )
        {
            print( "ERROR! Open file :%s", err.what() );
            mRender.mBlockIdx = -1;
            return;
        }
    }
    void takeFocus()
    {
        mBtnStart->take_focus();
    }
    void init( int argc, char **argv )
    {
        const int YGAP = 5, XGAP = 5;
        const int WIN_W = 950, WIN_H = 1000;
        const int LABEL_H = 40;
        int posX = 0, posY = 0; // accumulative var
        int w, h; // temp var
        mMainWindow = new Fl_Window( WIN_W, WIN_H );

        // row 1
        Fl_Button *btnOpen = new Fl_Button( posX = 10, posY += YGAP, w = 130, h = 25, "Open" );
        posX += w;
        btnOpen->callback( open_cb, this );
        btnOpen->tooltip( "Open a file to start." );

        mBtnStart = new Fl_Button( posX + 20, posY, w = 130, h = 25, "Start" );
        posX += 20 + w;
        mBtnStart->callback( start_cb, this );

        auto btnExit = new Fl_Button( posX + 20, posY, w = 130, h = 25, "Exit" );
        posX += 20 + w;
        btnExit->callback( exit_cb, this );

        posY += h;

        // row 2
        mQrWindow = new QRWindow( YGAP, posY + YGAP, w = WIN_W - 2 * XGAP, h = WIN_H - posY - 2 * YGAP - LABEL_H );
        posY += h + YGAP;

        // row 3
        mLabel = new Fl_Box( XGAP, posY + YGAP, w = WIN_W - 2 * XGAP, h = LABEL_H, "Please open\n file....." ); //

        // Initialize the file chooser
        mFileChooser = new Fl_Native_File_Chooser();
        //        fc->filter( "Text\t*.txt\n" );
        //        fc->preset_file( );

        mMainWindow->resizable( mMainWindow );
        mMainWindow->end();
        mMainWindow->show( argc, argv );

        //------- set dependancies ------------
        mRender.mGraph = mQrWindow;
        mRender.mUIC = this;
        mQrWindow->mRender = &mRender;
        takeFocus();
    }
    int run()
    {
        Fl::add_timeout( 1.0f / mPlayrateFPS, timer_cb, this );
        return Fl::run();

        //        while ( Fl::wait() )
        //        {
        //            int key = Fl::event_key();
        //            if ( key && key <= 127 )
        //            {
        //                printf( "key pressed: %d, %d\n", key, key & 0xFF );
        //                mQrWindow->redraw();
        //                mRender.onKeyEvent( key );
        //            }
        //        }
        //        return 0;
    }
};
int main( int argc, char **argv )
{
    App app;
    app.init( argc, argv );
    return app.run();
}
