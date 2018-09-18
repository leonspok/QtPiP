#include "pipwindow.h"
#include <iostream>
#include <QApplication>

static void show_usage()
{
    std::cout << "Usage: \n"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-s,--source URL\t\tSpecify url of video to play. If not specified, QtPiP will wait for url from stdin.\n"
              << std::endl;
}

int main(int argc, char *argv[])
{
    QString urlString;
    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        show_usage();
        return 0;
    } else if (argc > 2 && (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--source") == 0)) {
        urlString = QString(argv[2]);
    } else {
        std::string cppString;
        std::cin >> cppString;
        urlString = QString::fromUtf8(cppString.c_str());
    }

    QApplication a(argc, argv);
    std::setlocale(LC_NUMERIC, "C");

    PiPWindow w;
    w.show();
    w.moveToDefaultPosition();
    w.openURL(urlString);

    return a.exec();
}
