#pragma once
#include <list>
#include <sstream>
#include <iomanip>
#include "MosquittoMqtt.hpp"
#include <QtCore/QDebug>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QJsonArray>
#include <QtCore/QCoreApplication>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>

#define host "test.mosquitto.org"
#define port 8885
#define id "VADIM_K"
#define login "rw"
#define password "readwrite"
#define cafile "certs/mosquitto.org.crt"

#define S50 "S50"
#define S107 "S107"
#define S60 "S60"
#define API_INFO "api_info"
#define API_INFO_STATUS "status"

#define TEMP_ITEMS "items"
#define TEMP_VALUES "readings"
#define TEMP_ID "station_id"
#define TEMP_VALUE "value"

#define TEMP_TOPIC "/api/temperature/"
#define API_INFO_TOPIC "/api/status"

class Network {
private:
	using list_p = std::list<std::pair<std::string, std::string>>;

public:
	static QNetworkAccessManager* process_request() {
	    auto manager = new QNetworkAccessManager();
    	QObject::connect(
                manager, 
                &QNetworkAccessManager::finished,
                [=](QNetworkReply *reply) {
	        if (reply->error()) {
	            qDebug() << QString("Error %1").arg(reply->errorString());
	            exit(1);
	        }
	        std::cout << "API data has been recieved\n";
	        QByteArray responseData = reply->readAll();
	        QJsonDocument doc = QJsonDocument::fromJson(responseData);
			list_p data;
			parseJsonData(doc, data);

    		MosquittoMqtt mqtt(host, port, login, password, cafile, id, 60);
    		mqtt.send_data(data);

	        reply->deleteLater();
	        manager->deleteLater();
			mqtt.~MosquittoMqtt();

			exit(0);
    	});
    	return manager;
	}

private:

	void static parseJsonData(QJsonDocument& doc, list_p& data) {

        data.push_back({API_INFO_TOPIC, doc[API_INFO][API_INFO_STATUS].toString().toStdString()});

		QJsonValue vals = doc[TEMP_ITEMS][0][TEMP_VALUES];
        for(const QJsonValue& val : vals.toArray()) {
        	QJsonObject obj = val.toObject();
        	add_suitable(obj, data, TEMP_ID, TEMP_VALUE);
        }
	}

	void static add_suitable(QJsonObject& obj, list_p& data, std::string param, std::string value) {
		std::string p = obj[param.c_str()].toString().toStdString();
		std::ostringstream oss;
		if(p == S50 || p == S60 || p == S107) {
			oss << std::setprecision(3);
			oss << obj[value.c_str()].toDouble();
			data.push_back({TEMP_TOPIC + p, oss.str()});
		}
	}
};