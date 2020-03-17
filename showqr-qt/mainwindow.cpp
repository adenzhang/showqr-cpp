#include "mainwindow.h"
//#include "./ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QTimer>
#include <QFileDialog>
#include <QSizePolicy>
#include <QKeyEvent>
#include <QRDataBlocks.h>

class PaintWidget : public QWidget, public IGraphic
{
public:
    PaintWidget( QWidget *parent ) : QWidget( parent )
    {
    }

    void paintEvent( QPaintEvent * ) override
    {
        if ( mRender )
        {
            mRender->mGraph = this;
            mRender->renderCurrentBlock();
            return;
        }
        Qt::GlobalColor c[5] = {Qt::red, Qt::blue, Qt::gray, Qt::cyan, Qt::yellow};
        QPainter mPaint( this );
        mPaint.setPen( Qt::NoPen );
        mPaint.setBrush( c[++coloridx % 5] );
        mPaint.drawRect( 0, 0, this->width(), this->height() );
    }

    size_t getWidth() const override
    {
        return width();
    }
    size_t getHeight() const override
    {
        return height();
    }
    void drawRect( size_t left, size_t top, size_t width, size_t height, unsigned char colorR, unsigned char colorG, unsigned char colorB ) override
    {
        QPainter mPaint( this );
        mPaint.setBrush( QColor( colorR, colorG, colorB ) );
        mPaint.drawRect( left, top, width, height );
    }

    int coloridx = 0;
    QrRenderControl *mRender = nullptr;
};
class App : public QRUIControl
{
public:
    QWidget *mainWindow;
    PaintWidget *mQrWindow;
    QPushButton *btnOpen, *btnStart, *btnExit;
    QLabel *lblStatus;
    std::vector<char> mBuf;
    QTimer *mTimer;
    QrRenderControl mRender;
    int idx = 0;

    App() : mBuf( 256 )
    {
    }

    /**
 --------------------------------
 |                     | Button1 |
 |                     | Button2 |
 |     GraphWindow     |         |
 |                     |         |
 ---------------------------------
 | status                        |
 ---------------------------------

 VBox:
   - HBox for Graph and buttons:
   - VBox1 for buttons
     */
    void setupUI( QWidget *parent )
    {
        mainWindow = parent;
        mainWindow->setMinimumSize( 1000, 950 );
        QVBoxLayout *vbox = new QVBoxLayout( mainWindow );
        //-- row 1
        QHBoxLayout *hbox = new QHBoxLayout();
        QVBoxLayout *vbox1 = new QVBoxLayout();

        {
            // row 1 left
            mQrWindow = new PaintWidget( mainWindow );
            mQrWindow->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
            hbox->addWidget( mQrWindow );

            // row 1 right
            btnOpen = new QPushButton( "Open File", mainWindow );
            btnOpen->setMaximumWidth( 100 );
            QObject::connect( btnOpen, SIGNAL( clicked() ), mainWindow, SLOT( on_btnOpen_clicked() ) );
            btnStart = new QPushButton( "Start", mainWindow );
            btnStart->setMaximumWidth( 100 );
            QObject::connect( btnStart, SIGNAL( clicked() ), mainWindow, SLOT( on_btnStart_clicked() ) );
            btnExit = new QPushButton( "Exit", mainWindow );
            btnExit->setMaximumWidth( 100 );
            QObject::connect( btnExit, SIGNAL( clicked() ), mainWindow, SLOT( on_btnExit_clicked() ) );

            vbox1->addWidget( btnOpen );
            vbox1->addWidget( btnStart );
            vbox1->addWidget( btnExit );
            //            vbox1->setAlignment( Qt::AlignRight );

            hbox->addLayout( vbox1 );

            vbox->addLayout( hbox );
        }
        //-- row 2
        lblStatus = new QLabel( "Please open a file to start...", mainWindow );
        lblStatus->setMaximumHeight( 20 );
        //        lblStatus->setAlignment( Qt::AlignBottom );
        vbox->addWidget( lblStatus );

        mTimer = new QTimer( mainWindow );
        QObject::connect( mTimer, SIGNAL( timeout() ), mainWindow, SLOT( on_timeout() ) );

        mRender.mGraph = mQrWindow;
        mRender.mUIC = this;
        mQrWindow->mRender = &mRender;
        setUIFocus();
    }

    void print( const char *fmt, ... ) override
    {
        va_list args;
        va_start( args, fmt );
        vsnprintf( &mBuf[0], mBuf.size(), fmt, args );
        va_end( args );
        lblStatus->setText( &mBuf[0] );
    }
    void enableQRTimer( double seconds ) override
    {
        mTimer->start( seconds * 1000 );
    }
    void playStatusChanged( bool active ) override
    {
        btnStart->setText( active ? "Stop" : "Start" );
    }
    void close() override
    {
        mainWindow->close();
    }
    void redraw() override
    {
        mQrWindow->repaint();
    }
    //--- UI events
    void on_timeout()
    {
        print( " idx: %d ", idx++ );
        mQrWindow->repaint();
        if ( mRender.incBlock( true ) )
            mRender.prepareBlock();
        setUIFocus();
    }
    void on_start_click()
    {
        mRender.flipActive();
        setUIFocus();
    }
    void on_open_click()
    {
        QString fileName = QFileDialog::getOpenFileName( mainWindow, "Open a file to encode", "", "All Files (*)" );
        if ( fileName.isEmpty() )
            return;
        try
        {
            mRender.init( fileName.toStdString().c_str() );
            mRender.prepareBlock();
            mQrWindow->repaint();
        }
        catch ( std::runtime_error &err )
        {
            print( "ERROR! Open file :%s", err.what() );
            mRender.mBlockIdx = -1;
            return;
        }
        setUIFocus();
    }
    void setUIFocus()
    {
        mainWindow->setFocus();
    }
};

MainWindow::MainWindow( QWidget *parent ) : QWidget( parent )
{
    mApp = new App();
    mApp->setupUI( this );
}

MainWindow::~MainWindow()
{
    //    delete ui;
    delete mApp;
}

void MainWindow::on_btnOpen_clicked()
{
    mApp->on_open_click();
}

void MainWindow::on_btnStart_clicked()
{
    mApp->on_start_click();
}

void MainWindow::on_btnExit_clicked()
{
    this->destroy();
}

void MainWindow::on_timeout()
{
    mApp->on_timeout();
}

void MainWindow::keyReleaseEvent( QKeyEvent *event )
{
    int k = event->nativeVirtualKey();
    if ( Qt::Key_Return == event->key() )
        k = '\n';
    if ( k > 0 && k <= 127 )
        mApp->mRender.onKeyEvent( k );
}
