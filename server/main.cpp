#include <QApplication>
#include <QDir>

#include <iostream>
#include <iomanip>
#include <string>
#include <getopt.h>

#include "mainwindow.h"

using std::cout;
using std::cerr;
using std::left;
using std::right;
using std::setw;
using std::endl;

static struct option  long_options[] =
{
    {"tty"          , required_argument , 0             , 'd'   },
    {"config"       , required_argument , 0             , 'c'   },
    {"help"         , no_argument       , 0             , 'h'   },
    {"silent"       , no_argument       , 0             , 's'   },
    {"gui"          , no_argument       , 0             , 'x'   },
    {0              , 0                 , 0             , 0     },
};

void help(QString progname) {
    int width { 30 };
    int internal_width { 12 };

    cout << "USAGE: " << progname.toStdString() << " [args]" << endl;
    cout << "[args]:" << endl;
    cout << left << setw(width - internal_width) << "-c --config" << setw(internal_width) << "<dir>"
        << "Choose where the config files are stored." << endl;
    cout << left << setw(width) << "-h --help" << "Show this help and exit." << right << endl;
    cout << left << setw(width - internal_width) << "-d --tty" << setw(internal_width) << "<tty>"
        << "Choose USB serial device port." << endl;
    cout << left << setw(width) << "-s --silent" << "Don't output anything to console." << endl;
    cout << left << setw(width) << "-x --gui" << "Run the daemon graphically." << endl;
}

int main(int argc, char *argv[])
{
    int c, optindex;
    bool graphic {false}, silent {false};
    QString port{ "/dev/ttyACM0" };
    QString config{ "./config/" };

    while (( c = getopt_long(argc, argv, "c:hd:sx", long_options, &optindex)) != -1)
    {
        switch (c)
        {
        case 'c':
            config = QString::fromStdString( std::string{optarg} );
            break;
        case 'h':
            help( QString::fromStdString( std::string{argv[0]}) );
            return 0;
        case 'd':
            port = QString::fromStdString( std::string{optarg} );
            break;
        case 's':
            silent = true;
            break;
        case 'x':
            graphic = true ;
            break;
        default:
            if (static_cast<char>(c) != 'p')
            {
                help( QString::fromStdString( std::string{argv[0]}) );
                return 1;
            }
            break;
        }
    }

    // Check if the provided path exists, otherwise don't bother starting the program
    QDir confPath {config};
    if (!confPath.exists())
    {
        cerr << "Error: the provided path \""
             << config.toStdString() << "\" does not exist. Exiting application" << endl;
        return 1;
    }

    QApplication a(argc, argv);
    MainWindow w(graphic, silent, port, config);
    if (w.isDBusWorking())
    {
        // This function will trigger a signal that will initiate the OZW
        // background working thread
        w.startOZWEngine();
        /* Only run graphically if flag was set */
        if (graphic)
            w.show();

        return a.exec();
    }
    else
    {
        cerr << "Error: there was a problem creating the D-Bus interface"
             << endl;
        return 1;
    }
}
