#include <QApplication>
#include <QMainWindow>
#include <QPushButton>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("Mój pierwszy projekt Qt");
    window.resize(800, 600);

    QPushButton *button = new QPushButton("Kliknij mnie", &window);
    button->move(350, 280);

    window.show();

    return app.exec();
}
