#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class App;
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow( QWidget *parent = nullptr );
    ~MainWindow();

    void close()
    {
        destroy();
    }
    App *mApp = nullptr;
    void keyReleaseEvent( QKeyEvent *event ) override;
private slots:
    void on_btnOpen_clicked();

    void on_btnStart_clicked();

    void on_btnExit_clicked();

    void on_timeout();

private:
    //    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
