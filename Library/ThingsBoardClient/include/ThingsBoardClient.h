#ifndef THINGSBOARD_CLIENT_H
#define THINGSBOARD_CLIENT_H

#include <functional>
#include <future>
#include <map>
#include <mqtt/client.h>
#include <sstream>
#include <string>
#include <vector>

/* ================= JsonDocument ================= */

class JsonDocument {
public:
    JsonDocument() {}

    void set(const std::string &key, const std::string &value) {
        values_[key] = value;
    }

    void set(const std::string &key, int value) {
        values_[key] = std::to_string(value);
    }

    void set(const std::string &key, int64_t value) {
        values_[key] = std::to_string(value);
    }

    void set(const std::string &key, double value) {
        values_[key] = std::to_string(value);
    }

    void set(const std::string &key, bool value) {
        values_[key] = value ? "true" : "false";
    }

    std::string to_string() const {
        std::stringstream ss;
        ss << "{";
        bool first = true;
        for (const auto &kv : values_) {
            if (!first) ss << ",";
            ss << "\"" << kv.first << "\":" << kv.second;
            first = false;
        }
        ss << "}";
        return ss.str();
    }

private:
    std::map<std::string, std::string> values_;
};

/* ================= ThingsBoardClient ================= */

class ThingsBoardClient : public mqtt::callback {
public:
    ThingsBoardClient(const std::string &accessToken,
                      const std::string &host,
                      int port = 1883)
        : client_("tcp://" + host + ":" + std::to_string(port), ""),
          requestIdCounter_(0) {
        connOpts_.set_user_name(accessToken);
        client_.set_callback(*this);
    }

    ~ThingsBoardClient() {
        if (client_.is_connected())
            client_.disconnect();
    }

    void connect() {
        client_.connect(connOpts_);
    }

    void disconnect() {
        client_.disconnect();
    }

    /* ===== OLD (ยังใช้ได้) ===== */
    void sendTelemetry(const std::string &key, int value) {
        JsonDocument doc;
        doc.set(key, value);
        sendRaw(doc.to_string());
    }

    void sendTelemetry(const std::string &key,
                       const std::string &value) {
        JsonDocument doc;
        doc.set(key, "\"" + value + "\"");
        sendRaw(doc.to_string());
    }

    /* ===== NEW : รองรับ timestamp ===== */
    void sendTelemetry(int64_t ts, const JsonDocument &values) {
        std::stringstream ss;
        ss << "{"
           << "\"ts\":" << ts << ","
           << "\"values\":" << values.to_string()
           << "}";

        sendRaw(ss.str());
    }

protected:
    void connected(const std::string &) override {}
    void connection_lost(const std::string &) override {}
    void message_arrived(mqtt::const_message_ptr) override {}
    void delivery_complete(mqtt::delivery_token_ptr) override {}

private:
    mqtt::client client_;
    mqtt::connect_options connOpts_;
    int requestIdCounter_;

    void sendRaw(const std::string &payload) {
        auto msg = mqtt::make_message(
            "v1/devices/me/telemetry", payload);
        msg->set_qos(1);
        client_.publish(msg);
    }
};

#endif // THINGSBOARD_CLIENT_H
