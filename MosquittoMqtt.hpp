#pragma once
#include <mosquittopp.h>
#include <string>
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

class MosquittoMqtt : public mosqpp::mosquittopp {
public:
	MosquittoMqtt(std::string host, int port, std::string login, std::string password, std::string cafile, std::string id, int keepalive) :
	host(host), port(port), login(login), password(password), cafile(cafile), id(id), keepalive(keepalive) {
		mosqpp::lib_init();
		mosquittopp::username_pw_set(this->login.c_str(), this->password.c_str());
		mosquittopp::tls_set(this->cafile.c_str(), NULL, NULL, NULL, NULL);
	    connect_async(this->host.c_str(), this->port, this->keepalive);
	    loop_start();
		// std::cout << "CREATE\n";
	}

	~MosquittoMqtt() {
		// std::cout << "DESTROY\n";
	    disconnect();
	    loop_stop();
	    mosqpp::lib_cleanup();
	}

	void send_data(std::list<std::pair<std::string, std::string>> data) {
		for(auto& p : data) {
	        std::cout << "Message: " << p.first << " : " << p.second << '\n';
        	// this->subscribe(p.first);
        	this->publish(p.first, p.second);
        }
		this->wait_delivered();
	}

	bool publish(std::string topic, std::string message) {
		std::lock_guard<std::mutex> guard(this->mutex_send);
		this->send += 1;
	    int answer = mosqpp::mosquittopp::publish(nullptr, topic.c_str(), message.length(), message.c_str(), 1, false);
	    return (answer == MOSQ_ERR_SUCCESS);
	}

	bool subscribe(std::string topic) {
		int answer = mosquittopp::subscribe(nullptr, topic.c_str());
        return (answer != MOSQ_ERR_SUCCESS);
	}

	void on_connect(int rc) {
		if ( rc == 0 ) {
	        std::cout << "Connected with mqtt server" << '\n';
	    } else {
	        std::cout << "Impossible to connect with server(" << rc << ")" << '\n';
	    }
	}

	void on_disconnect(int rc) {
		// std::cout << "disconnection(" << rc << ")" << '\n';
	}

	void on_publish(int mid) {
		std::lock_guard<std::mutex> guard(this->mutex_delivered);
		this->delivered += 1;
	    std::cout << "Message published " << '\n';
	}

	void on_message(const struct mosquitto_message *message) {

	    std::string payload = std::string(static_cast<char *>(message->payload));
	    std::string topic = std::string(message->topic);

	    // std::cout << "RECEIVE:\n";
	    // std::cout << "\ttopic: " << topic << '\n';
	    // std::cout << "\tpayload: " << payload << '\n';
	}

	void on_subscribe(int, int, const int *) {
	    std::cout << "Subscription succeeded." << '\n';
	}

	void wait_delivered() {
		while(!is_delivered()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

private:
	bool is_delivered() {
		std::scoped_lock lck{this->mutex_delivered, this->mutex_send};
		return send == delivered;
	}

private:
	std::string id;
	std::string host;
	std::string login;
	std::string password;
	std::string cafile;
	int port;
	int keepalive;
	int send = 0;
	int delivered = 0;
	std::mutex mutex_send;
	std::mutex mutex_delivered;
};