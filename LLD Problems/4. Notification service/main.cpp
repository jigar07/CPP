#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <ctime>

using namespace std;

class User {
public:
    string id;
    string phone;
    string email;
};

// enum class NotificationType {
//     NEW_MESSAGE,
//     FRIEND_REQUEST,
//     LIKE_POST,
//     UNFOLLOW
// };

class IChannelBasedInputData {
public:
    virtual ~IChannelBasedInputData() = default;
};

class EmailChannelBasedInputData : public IChannelBasedInputData {
public:
    string subject;
    string body;
    string attachment;
};

class SmsChannelBasedInputData : public IChannelBasedInputData {
public:
    string body;
};

class Notification {
public:
    string message;
    string emailSubject;
    vector<string> ccEmails;
    string channel;
    shared_ptr<IChannelBasedInputData> channelBasedInput;
    User receiver;
    // NotificationType type;
    time_t timestamp;

    Notification() = default;
};

// The methods getString() and getList() do not modify the JsonNode object.
// Therefore, marking them const tells the compiler: "This method won't change the object," allowing it to be used with const JsonNode&.
class JsonNode {
private:
    map<string, string> stringData;
    map<string, vector<JsonNode>> listData;

public:
    JsonNode() = default;

    void setString(const string& key, const string& value) {
        stringData[key] = value;
    }

    void setList(const string& key, const vector<JsonNode>& value) {
        listData[key] = value;
    }

    string getString(const string& key) const {
        auto it = stringData.find(key);
        if (it != stringData.end()) return it->second;
        return "";
    }

    vector<JsonNode> getList(const string& key) const {
        auto it = listData.find(key);
        if (it != listData.end()) return it->second;
        return {};
    }
};


// Abstract Interface
class INotificationSender {
public:
    virtual ~INotificationSender() = default;
    virtual bool doesSupport(const string& channel) = 0;
    virtual void send(const Notification& notification) = 0;
    virtual shared_ptr<IChannelBasedInputData> parseChannelData(const JsonNode& node) = 0;
};

// Email Sender
class TPEmail {
public:
    void sendEmail(const string& to, const string& subject, const string& body) {
        cout << "Sending Email to: " << to << " Subject: " << subject << " Body: " << body << endl;
    }
};

class EmailNotificationSender : public INotificationSender {
private:
    TPEmail tpEmail;

public:
    bool doesSupport(const string& channel) override {
        return channel == "EMAIL";
    }

    shared_ptr<IChannelBasedInputData> parseChannelData(const JsonNode& node) override {
        auto emailData = make_shared<EmailChannelBasedInputData>();
        emailData->subject = node.getString("subject");
        emailData->body = node.getString("body");
        return emailData;
    }

    void send(const Notification& notification) override {
        auto data = dynamic_pointer_cast<EmailChannelBasedInputData>(notification.channelBasedInput);
        tpEmail.sendEmail(notification.receiver.email, data->subject, data->body);
    }
};

// SMS Sender
class TPSms {
public:
    void sendSms(const string& phone, const string& message) {
        cout << "Sending SMS to: " << phone << " Body: " << message << endl;
    }
};

class SmsNotificationSender : public INotificationSender {
private:
    TPSms tpSms;

public:
    bool doesSupport(const string& channel) override {
        return channel == "SMS";
    }

    shared_ptr<IChannelBasedInputData> parseChannelData(const JsonNode& node) override {
        auto smsData = make_shared<SmsChannelBasedInputData>();
        smsData->body = node.getString("body");
        return smsData;
    }

    void send(const Notification& notification) override {
        auto data = dynamic_pointer_cast<SmsChannelBasedInputData>(notification.channelBasedInput);
        tpSms.sendSms(notification.receiver.phone, data->body);
    }
};

// Notification Controller
class NotificationController {
private:
    map<string, shared_ptr<INotificationSender>> senderMap;

public:
    NotificationController() {
        senderMap["EMAIL"] = make_shared<EmailNotificationSender>();
        senderMap["SMS"] = make_shared<SmsNotificationSender>();
    }

    void sendNotification(const JsonNode& json) {
        string id = json.getString("id");
        string receiverId = json.getString("receiverId");
        string typeStr = json.getString("type");
        string timestampStr = json.getString("timestamp");
        vector<JsonNode> inputChannels = json.getList("channelsInput");

        Notification notification;
        notification.message = json.getString("message");
        notification.receiver = User{receiverId, "9999999999", "receiver@example.com"};
        notification.timestamp = time(nullptr); // simplified

        for (const auto& channel : inputChannels) {
            string channelType = channel.getString("type");
            auto sender = senderMap[channelType];

            if (sender && sender->doesSupport(channelType)) {
                notification.channelBasedInput = sender->parseChannelData(channel);
                notification.channel = channelType;
                sender->send(notification);
            }
        }
    }
};
// {
//   "id": "n1",
//   "channelsInput": [
//     {
//       "type": "SMS",
//       "body": "Liked a message"
//     },
//     {
//       "type": "EMAIL",
//       "body": "Liked a message",
//       "subject": "liked a message subject"
//     }
//   ],
//   "type": "NEW_MESSAGE",
//   "receiverId": "goura-id",
//   "timestamp": "2024-02-11 10:28",
//   "message": "Udit liked your post"
// }
JsonNode createMockJson() {
    JsonNode root;
    root.setString("id", "n1");
    root.setString("type", "NEW_MESSAGE");
    root.setString("receiverId", "goura-id");
    root.setString("timestamp", "2024-02-11 10:28");
    root.setString("message", "Udit liked your post");

    JsonNode smsChannel;
    smsChannel.setString("type", "SMS");
    smsChannel.setString("body", "Liked a message");

    JsonNode emailChannel;
    emailChannel.setString("type", "EMAIL");
    emailChannel.setString("body", "Liked a message");
    emailChannel.setString("subject", "liked a message subject");

    vector<JsonNode> channelsInput = {smsChannel, emailChannel};
    root.setList("channelsInput", channelsInput);

    return root;
}

int main() {
    NotificationController controller;
    JsonNode mockJson = createMockJson();
    controller.sendNotification(mockJson);
    return 0;
}
