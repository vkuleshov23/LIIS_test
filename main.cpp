#include <iostream>
#include "Network.hpp"
#include <QtCore/QCoreApplication>
#include <QtNetwork/QNetworkAccessManager>


int main(int argc, char *argv[]) {

	QCoreApplication a(argc, argv);
    auto manager = Network::process_request();
    manager->get(QNetworkRequest(QUrl("https://api.data.gov.sg/v1/environment/air-temperature")));
    return a.exec();
}