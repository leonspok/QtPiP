#include "pipwindow.h"
#include <iostream>
#include <QApplication>

static void show_usage()
{
    std::cout << "Usage: \n"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-m,--mode\t\tSpecify mode: mpv/web. Default: mpv.\n"
              << "\t-s,--source URL\t\tSpecify url of video to play.\n"
              << "\t--stdin\t\tRead mode and url from stdin.\n"
              << std::endl;
}

int main(int argc, char *argv[])
{
    QString urlString("");
    PiPWindowMode mode = PiPWindowModePlayer;
    bool shouldReadFromStdIn = false;
    for (int i = 0; i < argc; i++) {
        const char *arg = argv[i];
        if ((strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)) {
            show_usage();
            return 0;
        } else if (strcmp(arg, "-s") == 0 || strcmp(arg, "--source") == 0) {
            i++;
            if (i < argc) {
                urlString = QString(argv[i]);
            }
        } else if (strcmp(arg, "-m") == 0 || strcmp(arg, "--mode") == 0) {
            i++;
            if (i < argc) {
                if (strcmp(argv[i], "mpv") == 0) {
                    mode = PiPWindowModePlayer;
                } else if (strcmp(argv[i], "web") == 0) {
                    mode = PiPWindowModeWeb;
                }
            }
        } else if (strcmp(arg, "--stdin") == 0) {
            shouldReadFromStdIn = true;
        }
    }

    shouldReadFromStdIn = shouldReadFromStdIn || (argc == 1);

    if (shouldReadFromStdIn) {
        std::string cppModeString;
        std::cin >> cppModeString;
        std::string cppUrlString;
        std::cin >> cppUrlString;
        mode = (cppModeString == "web") ? PiPWindowModeWeb : PiPWindowModePlayer;
        urlString = QString::fromUtf8(cppUrlString.c_str());
    }

    QApplication a(argc, argv);
    std::setlocale(LC_NUMERIC, "C");

    PiPWindow w;
    w.show();
    w.moveToDefaultPosition();
    w.openUrl(urlString, mode);

    return a.exec();
}
