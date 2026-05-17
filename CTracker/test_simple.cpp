#include <QApplication>
#include <QMessageBox>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QMessageBox::information(nullptr, "Test", "Simple test - do you see this?");
    return 0;
}
