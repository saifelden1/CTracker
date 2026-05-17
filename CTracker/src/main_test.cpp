#include <QApplication>
#include <QMessageBox>
#include <QDebug>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // Force a message box - this MUST show something
    QMessageBox::information(nullptr, "CTracker", "If you see this, Qt is working!");
    
    return 0;
}
