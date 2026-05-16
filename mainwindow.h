#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "project.h" // Include Project header

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_actionNew_triggered();

    void on_actionOpen_triggered();

    void on_actionCPTs_triggered();

private:
    Ui::MainWindow *ui;
    Project *m_currentProject = nullptr; // Member to hold the current project
};
#endif // MAINWINDOW_H