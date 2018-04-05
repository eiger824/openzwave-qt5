#include <QCoreApplication>
#include <QString>

#include <iostream>
#include <string>
#include <getopt.h>

#include "openzwaveproxyclient.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

void help (QString const & program )
{
    cout << "USAGE: " << program.split('/').last().toStdString()
         << " -n <node_id> -v <value>" << endl;
}

int main(int argc, char *argv[])
{
    int c;
    int nodeId {-1};
    int value {-1};

    while ((c = getopt(argc, argv, "hn:v:")) != -1)
    {
        switch (c)
        {
        case 'h':
            help ( QString::fromStdString(string{argv[0]}) );
            return 0;
        case 'n':
            nodeId = static_cast<uint>( atoi(optarg) );
            break;
        case 'v':
            value = static_cast<uint> (atoi (optarg ));
            break;
        default:
            cerr << "Unrecognized option '" << c << "'" << endl;
            help ( QString::fromStdString(string{argv[0]}) );
            return 1;
        }
    }
    if (nodeId == -1 or value == -1)
    {
        help ( QString::fromStdString(string{argv[0]}) );
        return 1;
    }
    OpenZWaveProxyClient proxy {static_cast<uint>(nodeId), static_cast<uint>(value)};

    QCoreApplication a(argc, argv);
    return a.exec();
}
