
Problem: https://enginebogie.com/public/question/low-level-design-notification-system/261

## Requirements:
* Users: The system should support users, each identified by a unique user ID. Users can subscribe to different types of notifications and manage their notification preferences.
* Notification Channels: The system should support multiple notification channels, such as email, SMS, push notifications, and in-app notifications. Each channel has its own delivery mechanism and requirements.
* Notification Types: Users can subscribe to different types of notifications, such as new messages, friend requests, system alerts, or custom events. Each notification type may have specific content and delivery requirements.

- API: As command line or anything not given, use API

## ThoughtProcess
- Notification is sent to one user
- There can be different notification type such as Email, SMS etc
  - Hence, we need INotificationSender
  - Input notification can have multiple notification channel such as Email, SMS
  - Based on the notification type parse input and then send notification
  - So, **INotificationSender** have two methods:
    - parseChannelData - returns IChannelBasedInputData which is set inside Notification object
    - send - It receives Notification as arguement. And using IChannelBasedInputData notification is sent to that channel
  - So, **Notification** object is also created for each notification when using **NotificationController**'s sendNotification method.
    - sendNotification has main business logic which:
      - creates Notification object
      - parses messages based on Channel type and assign it to Notification object member
      - sendNotification which parses messages, and send messsage to channel(INotificationSender) based on input
/**
* {
	id: "n1",
	channelsInput: [{type: 'SMS', message: "Liked a message"}, {type: 'EMAIL', message: "Liked a message", subject: "liked a message subject"}],
	type: 'NEW_MESSAGE',
	receiverId: 'goura-id'
	timestamp: '2024-02-11 10:28'

* }
* */
## Notes
- famous for both LLD and HLD
- Personalization and opt-out and preferences are usually asked in HLD not LLD
- point of caution: Directly map request with the input you are getting
- Don't try to increase the scope. In interview "can say that user can subscribes to few things. But right now I will be subscribing everything for every users". Otherwise, complete scope will not be get covered in interview
- Don't try to build all the requirement in one go. Build requirement one by one
- During interview also don't implement JSON parser and all as this is not problem of JSON parser. Just imagine JSON parser is already available
- Don't think about many requirement. Otherwise, it will become difficult to implement
- **Requirements are always source of truth. Whenever in confusion use requirements and ocp**
  - Also, check if reverse of ISP can be used if there is chance of error-prone(Due to null pointer exception or key not available)
  - Requirement must be clear. If requirements are not clear then it will become difficult to solve question
  - Evaluate whether we want run time safety or compile time safetly
  - NotificationType is not used because it will not comply with OCP. Need to add type for any NotificationType
    - So, if enum list becomes very large then better to convert into String
- In interview keep your requirement simple. Solve one requirement, improve it, go to another requirement, improve it and solve it. You can say you are not including this requirement due to time limit. (Notification problem we converted to very complex)
  - We should design for future requirement but not implement it
  - Use it at 2-3 times and for other call out that this needs to be extended. **Don't silently ignore. Callout and infor the interviewer**. Otherwise, interviewer will think you don't know it
  - For notification during interview:
    - Handle different type of notification (INotificationSender)
    - ChannelTypeData
    - Then, if they ask to solve anything else then solve, if they asked to touch upon(Dive deep) on anything then do it
    - I will not discuss that multiple input aayega and all

## Design
- class User
- class Notification
  - String message, List<IChannel> channels (Not required, removed later), User receiver, Date, NotificationType 
- class NotificationController
  - It has sendNotification method
- class JsonNode
- interface IChannel - Actually instead of TPSms and TPEmail is used (TP - third party)
- enum NotificationType (part of Notification)

- Now, write method from top down. **NotificationController** is top level class for this problem because that's where flow starts
  - Clear relation between message (of class Notification) and NotificationType. Message body also comes from NotificationType. So, we might need to remove one of this. Mostly we will remove Message.
  - Right now park it for v1. Once v1 done then will think about this
  - NotificationController::sendNotification
  - channelObjs to send notification to all channels
  - interface INotificationSender
    - No major difference between IChannel and INotificationSender. Both are same things
    - Hence, IChannel is not required. We can use INotificationSender only
  - EmailNotificationSender: what should be body, subject for the email?
    - `private Map<NotificationType, EmailChannelData> messageMap;` added for this.
    - Same thing for SmsNotificationSender
    - But adding new `EmailChannelData` would required change in NotificationController (So, not OCP complient). It is unnecessary addition of classes
      - **If something can be done using property then do not create class**. Interface can be added if there are different behavior like language. If same behavior then do not add interface. In that case can use map, list, or generic function etc.
    - class TPPush and PushNotificationSender created to send notification via app installed
  - `private Map<NotificationType, EmailChannelData> messageMap;` has also another problem. (This needs to be discussed during interview.)
    - It's not about OCP, it is about error proneness. messageMap might run into run-time error (**Reverse of ISP, similarly discussed in splitwise**). Ex. Unfollow notification added and for this EmailChannelData added but SmsChannelData not added. In this case it can run into run time error
    - To fix this:
      - Can add if condition to see whether key exist or not. But this is **silent failure** because user wont receive notification but we won't be aware about this. Its like preventing error (Developer is happy but very very bad user experience). So, this is not acceptable solution
      - We need solution which will report this during compile time only. That's where interface is used when we want to enforce things
        - interface IChannelTypeData can be addded. But it would requirement many classes. Hence, **class ChannelTypeData** added
      - This type of things are very important. Always, try to write code which save future changes
- String message inside Notification class;
  - This gives flexibility to client to provide any message. But this is not requirement. Hence, we have removed this flexibility and added SmsChannelData, EmailChannelData. To in interview mention that and also mention pros and cons. In production we might have requirement of this also
  - Use `private Map<NotificationType, ChannelTypeData> messageMap` if do not want to give flexibility to client.
  - In case want to give flexibility to client then in `class Notification` we can add properties like message, emailSubject, pushImageUrl
    - Can add `Map<String, Object> extraData;` but it is error-prone due to null key or key not vailable
    - `interface IChannelBasedInputData` is better solution instead of emailSubject, pushImageUrl inside Notification class

## Code

```java
class User {
	String id;
	String phone;
	String email;
}

enum NotificationType {
	NEW_MESSAGE,
	FRIEND_REQUEST,
	LIKE_POST,
	UNFOLLOW
}

class Notification {
	String message; // TODO: See if message is required here or in specific channel. "Udit liked your post".
	String emailSubject;
	List<String> ccEmails;
	// String pushImageUrl;

	// Map<String, Object> extraData; // email-> emailSubject, push->pushImageUrl
	IChannelBasedInputData channelbasedData;
	String channel;
	User receiver;
	String type;

	Date timestamp;
}

interface IChannelBasedInputData {

}

class EmailChannelBasedInputData implements IChannelBasedInputData {
	String subject; // "hi i am udit", "udit am i hi", "Hi <user>, new received getBody"
	String body;
	String attachment;
}

class SmsChannelBasedInputData implements IChannelBasedInputData {
	String body;
}


class JsonNode {

	public Object get(String key) {

	}
}

/**
 * {
 	  id: "n1",
 	  channelsInput: [{type: 'SMS', message: "Liked a message"}, {type: 'EMAIL', message: "Liked a message", subject: "liked a message subject"}],
 	  type: 'NEW_MESSAGE',
 	  receiverId: 'goura-id'
 	  timestamp: '2024-02-11 10:28'

 * }
 * */
class NotificationController {

=	private List<INotificationSender> notificationSenders;

	private Map<String, INotificationSender> notificationSenderMap;

	public void sendNotification(@RequestBody JsonNode json) {
		String id = json.get("id");
		String message = json.get("message")
		String receiverId = json.get("receiverId")
		List<JsonNode> inputChannels = json.get("channelsInput");
		String type = json.get("type");
		String timestamp = json.get("timestamp");

		// final List<IChannel> channelObjs = inputChannels.stream().map(inputChannel -> channels.get(inputChannel)).collect(Collectors:toList);
		// final Notification notification = new Notification(message,channelObjs, "<whatToSend?>"m type, timestamp);

 		// for (IChannel channel: channelObjs) {
 		// 	channel.send(notification);
 		// }

 		for (JsonNode channel: inputChannels) {
 			String channelType = channel.getType();
 			INotificationSender ns = notificationSenderMap.get(channel);
			IChannelBasedInputData channelBasedInput = ns.parseChannelData(channel);
 			final Notification notification = new Notification(message,channelBasedInput, "<whatToSend?>", timestamp);
			
 			ns.send(notification);

	 		// for (INotificationSender notificationSender: notificationSenders) {
	 		// 	if (notificationSender.doesSupport(channel)) {
	 		// 		notificationSender.send(notification);
	 		// 	}
	 		// }
 		}
 		
	}
}

class SmsChannelData  {
	String body;
}


class EmailChannelData  {
	String subject; // "hi i am udit", "udit am i hi", "Hi <user>, new received getBody"
	String body;
	String attachment;
}

// Unnecessary addition of classes

interface NotificationTemplate {
	public String parseTemplate(String msg, Notification notification) {
		return msg.replace("<user>", notification.getUser());
	}
}


class PushNotification {
	String image;
	String body;
}

interface INotificationSender {
	boolean doesSupport(String channel);
	void send(Notification notification);
}

interface IChannelTypeData {
	EmailChannelData getEmailData();
	SmsChannelData getSmsData();
}


class ChannelTypeData {

	@NotNull
	EmailChannelData emailData;

	@NotNull
	SmsChannelData smsData;

	public ChannelTypeData(EmailChannelData emailData, SmsChannelData smsData, PushNotificationData pushData) {

	}
}

class ChannelTypeDataService {
	new ChannelTypeData(emailData, null)

}

/*
messageMap.put(NotificationType.NEW_MESSAGE, new EmailChannelData("new message received"), "you have received a new message");
*/
class EmailNotificationSender implements INotificationSender {
	private TPEmail tpEmail;
	private Map<NotificationType, ChannelTypeData> messageMap;
	private NotificationTemplate notificationTemplate;

	void send(Notification notification) {
		EmailChannelBasedInputData emailData = (EmailChannelBasedInputData)notification.channelBasedInput;
		tpEmail.sendEmail(notification.getReceiver().getEmail(), emailData.getSubject(), notification.getMessage());

		// Strign body = messageMap.get(type).getBody();
		// notificationTemplate.parseTemplate(body, notification);
		// tpEmail.sendEmail(notification.getReceiver().getEmail(), messageMap.get(type).getSubject(), messageMap.get(type).getBody());
		
	}

	boolean doesSupport(IChannel channel) {
		return channel.equals("EMAIL");
	}
}

class SmsNotificationSender implements INotificationSender {
	private TPSms tpSms;
	private Map<NotificationType, ChannelTypeData> messageMap;

	void send(Notification notification, IChannelBasedInputData channelBasedInput) {
		SmsChannelBasedInputData smsData = (SmsChannelBasedInputData)channelBasedInput;

		tpSms.sendSms(notification.getReceiver().getPhone(), smsData.getMessage());

		// tpSms.sendSms(notification.getReceiver().getPhone(), messageMap.get(type).getSmsData().getBody());
	}

	boolean doesSupport(IChannel channel) {
		return channel.equals("SMS");
	}
}





class PushNotificationSender implements INotificationSender {
	private TPPush tpPush;
	// private Map<NotificationType, PushNotification> messageMap;

	void send(Notification notification) {
		tpPush.send(notification.getReceiver().getPushToken(), notification.getPushImage(), notification.getMessage());
		// tpEmail.send(notification.getReceiver().getEmail(), messageMap.get(type).getSubject(), messageMap.get(type).getBody());
	}

	boolean doesSupport(IChannel channel) {
		return channel.equals("EMAIL");
	}
}

class 

class TPSms {

	public void sendSms(String phone, String message) {

	}
}

class TPEmail {

	public void sendEmail(String toEmail, String subject, String body) {

	}
}

class TPPush {

	public void sendEmail(String pushToken, Image image, String body) {

	}
}

```