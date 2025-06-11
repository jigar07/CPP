# Requirements
REST API Design

# What it is and why is it there?
- A protocol we can use for data transfer. Rule set.
- Standard practice.
- Makes things simple.
- Tomorrow if I am creating an API, I would know this I have to create.


- Convention. Are set to make things simple.
  - REST API should be simple so that it will be easier for any external userto use it without reaching out to external user.
- Simplicity is not for self while for others.
- Adobe, MS (for few team) follow convention very strictly
- For interview follow convention

# Notes
- Lot of company asked about REST API design in LLD and HLD
- GetMapping in Java used to map HTTP GET requests to handler methods of a controller class.
- Atlassian expects to write APIs in HLD. UDIT did not write APIs and hence got rejected. So, they did not ask specifically. But they were expecting that interviewee must write APIs
  - And, if you are writing APIs then must follow conventions
- Do not use POST for update, do not add verbs in API as HTTP method itself display verb
- AuthN/AuthZ is sent through header only
- On google chrome inspect->network to check URL, headers, APIs etc
  - cookie can be used for AuthN/AuthZ. Cookie is easier things to do. Sent via header onlys
- For interview can go with REST APIs. If company is using GRPC then go with GRPC only. If want to change API(from REST to GRPC or any other) then should have strong reason for that

# Example

 ```java

class TodoController {


	@GetMapping
	public void createTodo() {

	}

	@PostMapping
	public void createTodo() {

	}
}

```

Booking use case

option1:
GET /api/v1/bookings/create?mentee=abc&expert=pqr

option2:
POST/PUT/BATCH /api/v1/bookings/create?mentee=abc&expert=pqr

option3
POST /api/create_bookings


/api/createTodo
/api/getTodo


# Conventions:

- HTTP Method
- URLs - how to build the URLs
- How to send the request data and how to return the response data.

HTTP REQUEST: METHOD, URL, REQUEST BODY, REQUEST HEADERS.
HTTP RESPONSE: RESPONSE BODY, RESPONSE HEADERS.

- Inspect -> Network -> Sekect any link
  - It will show header, request, response, API etc
## Detail
- HTTP Method
	- GET: 
		- To get any resource
		- headers and body.
		- There should not be any side effects of this on the backend. (Means no update of data)
	- POST: 
		- To create any resource
		- Not idempotent.
	- PUT: 
		- To update any resource. Replacement.
		- Idempotent. - if same thing is done multiple times, its result should be same. F("hi") -> "hello"
		value in store todo: {"id": xyz, "title": "get something done", "createdBy": udit, "dueDate": 2024/07/30, "isCompleted": false}

		put {"id": xyz, "title": "get something done 2", "createdBy": udit, "dueDate": 2024/08/30, "isCompleted": false}
		value in store todo: {"id": xyz, "title": "get something done 2", "createdBy": udit, "dueDate": 2024/08/30, "isCompleted": false}

		put: {"id": xyz, "title": "get something done 3", "dueDate": 2024/09/30}
		value in store todo: {"id": xyz, "title": "get something done 3", "createdBy": null, "dueDate": 2024/09/30, "isCompleted": false}
	- PATCH:
		- Similar to PUT, but in put we have to provide all while in patch only the attribute to be updated.
	- DELETE:
		- To delete the resource.
	- HEAD:
		- To get the response headers.
		- Where it would be helpful

		Google drive - youtube/netflix/images / JS /CSS - BIG FILES.
			IMAGE
			Browser -> Backend to get the video and then play it. download for the first time and cache it locally. Needs to update it regulrly if cached. Every some frquency, it will say refetch. HEAD call to the backend - get the **etag**. etag is hash of the content. etag changes if body of content is changed. If body of image is changed then etag gets changed. If not changed then etag does not change. (Hash does not change if content does not change)

		HEAD calls. Common in spring boot. It does not want you to implement HEAD explicitly. If you implement GET, then spring will automatically give HEAD to you. (In this even it will increase latency as it is getting data. Udit used it initially, then they realise it is increasing latency. Hence, for HEAD calls he added API for HEAD)
			Client HEAD -> API -> SPRING -> GET (BIG FILES) -> CONTROLLER -> BODY & HEADERS -> SPRING -> Remove Body and send HEADERS back to client. 
			One disadvantage of HEAD calls is, server is processing complete reponse even though it only needs header

			Client HEAD -> API -> SPRING -> GET (BIG FILES) -> CONTROLLER -> DATA STORE (S3, Downstream service, ...) ->  BODY & HEADERS -> SPRING -> Remove Body and send HEADERS back to client. 

		GET calls: CLIENT -> GET API -> DATA STORE (S3, Downstream service, ...) -> CLIENT.

		DIRECTORIES - FOLDERS (Udit was fetching all the folders files initially which was increasing latency. Later change it to fetch etag only instead of complete files. This can be debatable. Initially can start with fetching all files using GET then if required then implement HEAD with etag)

	- OPTIONS:
		- To get what all HTTP methods server suuports.

- URLs - how to build the URLs.
	- Resources. URL should identify resource only
	TODO:
	POST /create_todo
	POST /todo/create
	POST /create/todo
	POST /make_todo
	POST /add_todo
	POST /todo/add

	None of the above is valid. Because post itself tells operation. Also, operation name should not be in URL. That's what REST conventions says. REST conventions says, URL should identify resource. It should not tells action. Action is told by HTTP method. Following is correct URL to created todo. todo means resource, POST means create
	POST /todo

	GET /todo

	GET /todo/{todoId}
	GET /todo/t1
	POST /todo/t1 - says nothing. Because t1 is already created. So, this is not right URL. Also, this URL does not exist. For any URL, URL should be exist
	POST /todo/t1/likes - This is right URL - This means add one like to resource t1

	GET /task/t1
	GET /task/t1/likes/l1


	POST /bookings
	GET /bookings/{bookingId}/attendees
	POST /bookings/{bookingId}/attendees

	DELETE /bookings/{bookingId}/attendees/{attendeeId}
	DELETE /bookings/b1/attendees/udit
	

	todo list [t1, t2, ....]

- How to send the request data and how to return the response data.
Request:
	GET: Path identifies the full resource and other params should be in the query params. They should not be in the body. (Filtering, pagination should be part of query params)
	GET /todo/{todoId}/comments?type=positive&page=0&pageSize=10 is valid URL. Type is filter on type property. It is not resource.
		/todo/{todoId}/comments?type=positive&page=0&pageSize=10, /todo/{todoId}/ is path
	GET /todo?todoId={todo}. path /todo does not identifies full resource. Hence, this is not valid URL

	GET /todo/{todoId}/comments
		response: headers -> status: 200
		body: {type: positive, page: 1, size: 10}
	Why not body? Again related caching.
		- Get is latency heavy.
		- So thats why lots of intermediaries from source to destination - browser/App, OS, ISP, Routers, CDNs
		- body is not visible to intermediaries and body is also encrypted. Hence, body is not suggested for Get

	POST: 
	body {data of the new resource}
	Response:
		Headers:
			Status code: 201 (created), 409, 400, 401, 403, 404, 5xx 
			Location: If it succeeds, location of the new resource (means path of resource, if I do get on resource that it can be done using Location). /todo/{todoId}
		Body: Created resource data if success.
				failure details if it fails.




- Using post for SEARCH APIs - Search is GET but still some people use POST for it.
  - Lot of filter for search. To handle using query params it become complecate and difficult to manage. Also, URL might hits limit. So, in that can move away from get and then convert into POST.
    - why not GET with BODY instead of POST with BODY?
      - Because GET does caching. And we don't want to do cachnig so POST with BODY
  - So, if valid good reason then can move away from following conventionss
  - Sometimes you go away from conventions but conciously. It's not like you always follow conventions.
  - If want to reduce latency or optimised then can go with this. Everything depends on problem you are solving

# Paging
- You are sending the response page by page.
`pageNum=1 and pageSize=10` -> Problem with this (Element which is shown might be shown again and newly created page will not be displayed. If element is deleted then also some page might not be shown to user (which is bad).):
	10, 20, 30, 40, 50, 60, 70, 80

	page 0, 5
	10, 20, 30, 40, 50

	while fetching page another user created one more element 15 and lets say we are fetching pages in sorted order
	10, 15, 20, 30, 40, 50, 60, 70, 80

	page 1, 5
	50, 60, 70, 80

	delete 30
	10, 20, 40, 50, 60, 70, 80
		
	page 1, 5
	70, 80 (60 is not shown which is deleted)


`cursor based paging.`: Solves above problem
	after, size (instead of pagenum, we sent after)
	This will solve - repeation problem, and problem of page not shown



# JWT token- Json Web Token.
- Contains JSON based information of user, name, details, expiration time, claims, tier etc.
- Token sent from backend to the client so that for some time it is used for authorization.
- Because of every request we might need what is the authorization of this user. May be if I have admin rights or not.
	- Getting from backend will be an overhead. Latency issue.
	- How do we solve for impersonation? - signature part of jwt.

Authentication
	- who you are.
Authorization
	- what you can do.

backend asks client to send the user information.
	- username and password send in every request.
	- token
	- govind, pass123 -> login
	- validation this username and password
	- login generates a random string. 
	- saves the token: govind in db. - storage maintainence (in-memory, or DB, or Redis etc)
	- returns the token.

get session details - lld
	- token in request header.
	- backend will find the user associated with it, govind. (AuthN)
	- is govind registered for the cohort lld. (AuthZ)
	- yes he is registered, we return the session details.
	- otherwise return unauthorized.

- Token gets lost if stored in-memory and server is restarted(engine boogie stores in-memory). If in DB then latency issue.
  - For engine boogie, in-memory is fine. For companies like amazon and MS neither in-memory(re-login) fine nor in DB(latency issue)
  - For multiple server also in-memory does not help
  - Udit is planning to move JWT to solve above problems

JWT
backend asks client to send the user information.
	- govind, pass123 -> login
	- validation this username and password
	- {id: govind, email: govind@gmail.com, bookings: [lld, hld]} sent by backend to the client. 
	- {payload: {id: govind, email: govind@gmail.com, bookings: [lld, hld]} , signature: hash(payload)}
	- this is encoded in base64 format.
	- Client stores it somewhere.
	
get session details - lld
	- {json} in request header by client to server. ({json} = `{payload: {id: govind, email: govind@gmail.com, bookings: [lld, hld]} , signature: hash(payload)}` which is sent by backend above)
    	- json is encoded(not encrypted)
	- validate the signature
	- backend will fetch user details from the json, and bookings also from json.
	- will be able to do both authn, authz.

Problem:
- impersonation(client modify and mention that he has LLD course as well) or spoofing(someone spoof it)
  - To resolve this `signature: hash(payload)` is also sent. And client validate signature

sessionId vs JWT is comparible (cookie vs JWT should not be comparible. Cookie is just storage)
client - browsers (location storage (JWT/SESSIONID), cookie(JWT/SESSIONID))

MITM (Man in the middle attack) - Either JWT or SESSIONID then it is stolen and then used. Can be solved by educating the user.

Server - JWT/SESSIONID
	SESSIONID needs to be stored at backend (Token)
	JWT then all data sent to user by backend and then user will send it for every request


