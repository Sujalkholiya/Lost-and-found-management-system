LOST & FOUND BACKEND API DOCUMENTATION

==================================================
1. BASE URL
==================================================

http://127.0.0.1:18080


==================================================
2. API OVERVIEW
==================================================

Total APIs: 6

1. POST   /api/users/register
2. POST   /api/users/login
3. GET    /api/items
4. GET    /api/items/:id
5. POST   /api/items
6. GET    /api/admin/items


==================================================
3. AUTHENTICATION
==================================================

After successful login, the backend returns an authentication token.

For protected APIs, the frontend must send the token in the Authorization header.

Format:

Authorization: Bearer YOUR_TOKEN

Example:

Authorization: Bearer abc123xyz456

Protected APIs:

POST /api/items

GET /api/admin/items


==================================================
4. REGISTER USER
==================================================

METHOD:

POST

ENDPOINT:

/api/users/register

FULL URL:

http://127.0.0.1:18080/api/users/register

AUTHENTICATION:

Not required.

REQUEST BODY:

{
    "name": "Priyanshu",
    "email": "priyanshu@gmail.com",
    "password": "yourpassword"
}

SUCCESS STATUS:

201 Created

SUCCESS RESPONSE:

{
    "message": "User registered successfully"
}


==================================================
5. LOGIN USER
==================================================

METHOD:

POST

ENDPOINT:

/api/users/login

FULL URL:

http://127.0.0.1:18080/api/users/login

AUTHENTICATION:

Not required.

REQUEST BODY:

{
    "email": "priyanshu@gmail.com",
    "password": "yourpassword"
}

SUCCESS STATUS:

200 OK

SUCCESS RESPONSE:

{
    "message": "Login successful",
    "user": {
        "id": 1,
        "name": "Priyanshu",
        "email": "priyanshu@gmail.com",
        "role": "student"
    },
    "token": "YOUR_AUTH_TOKEN"
}

IMPORTANT FOR FRONTEND:

After successful login, save the token returned by the backend.

The token must be sent with protected API requests.

Example:

Authorization: Bearer YOUR_AUTH_TOKEN


==================================================
6. GET ALL ITEMS
==================================================

METHOD:

GET

ENDPOINT:

/api/items

FULL URL:

http://127.0.0.1:18080/api/items

AUTHENTICATION:

Not required.

REQUEST BODY:

None.

SUCCESS STATUS:

200 OK

EXAMPLE RESPONSE:

{
    "count": 2,
    "items": [
        {
            "id": 2,
            "title": "Blue Water Bottle",
            "description": "Found near the playground",
            "type": "found",
            "location": "College Playground",
            "user_id": 2
        },
        {
            "id": 1,
            "title": "Black Wallet",
            "description": "Lost near the library",
            "type": "lost",
            "location": "College Library",
            "user_id": 1
        }
    ]
}


==================================================
7. GET ITEM BY ID
==================================================

METHOD:

GET

ENDPOINT:

/api/items/:id

Replace :id with the actual item ID.

EXAMPLE:

/api/items/1

FULL URL:

http://127.0.0.1:18080/api/items/1

AUTHENTICATION:

Not required.

REQUEST BODY:

None.

SUCCESS STATUS:

200 OK

EXAMPLE RESPONSE:

{
    "item": {
        "id": 1,
        "title": "Black Wallet",
        "description": "Lost near the library",
        "type": "lost",
        "location": "College Library",
        "user_id": 1
    }
}

IF ITEM DOES NOT EXIST:

STATUS:

404 Not Found

RESPONSE:

Item not found


==================================================
8. CREATE LOST OR FOUND ITEM
==================================================

METHOD:

POST

ENDPOINT:

/api/items

FULL URL:

http://127.0.0.1:18080/api/items

AUTHENTICATION:

Required.

HEADERS:

Authorization: Bearer YOUR_TOKEN

Content-Type: application/json

REQUEST BODY:

{
    "title": "Blue Water Bottle",
    "description": "Found near the playground",
    "type": "found",
    "location": "College Playground"
}

ALLOWED VALUES FOR TYPE:

lost

or

found

IMPORTANT:

The frontend must NOT send user_id.

The backend automatically gets the user ID from the authentication token.

FLOW:

User logs in
        |
        v
Backend returns token
        |
        v
Frontend saves token
        |
        v
Frontend sends token with request
        |
        v
Backend authenticates user
        |
        v
Backend gets authenticated user ID
        |
        v
Item is created with that user ID

SUCCESS STATUS:

201 Created

EXAMPLE SUCCESS RESPONSE:

{
    "message": "Item created successfully",
    "item": {
        "id": 2,
        "title": "Blue Water Bottle",
        "description": "Found near the playground",
        "type": "found",
        "location": "College Playground",
        "user_id": 2
    }
}

POSSIBLE ERRORS:

400 Bad Request

Possible reasons:

- Invalid JSON
- Missing required fields
- Invalid item type

401 Unauthorized

Possible reason:

- Authorization token is missing or invalid.


==================================================
9. ADMIN - GET ALL ITEMS
==================================================

METHOD:

GET

ENDPOINT:

/api/admin/items

FULL URL:

http://127.0.0.1:18080/api/admin/items

AUTHENTICATION:

Required.

ROLE REQUIRED:

admin

HEADERS:

Authorization: Bearer ADMIN_TOKEN

REQUEST BODY:

None.

SUCCESS STATUS:

200 OK

EXAMPLE RESPONSE:

{
    "count": 2,
    "items": [
        {
            "id": 2,
            "title": "Blue Water Bottle",
            "description": "Found near the playground",
            "type": "found",
            "location": "College Playground",
            "status": "active",
            "user_id": 2,
            "created_at": "2026-08-20 17:43:02"
        },
        {
            "id": 1,
            "title": "Black Wallet",
            "description": "Lost near the library",
            "type": "lost",
            "location": "College Library",
            "status": "active",
            "user_id": 1,
            "created_at": "2026-08-20 17:40:10"
        }
    ]
}


IF TOKEN IS MISSING OR INVALID:

STATUS:

401 Unauthorized

RESPONSE:

Unauthorized


IF USER IS NOT AN ADMIN:

STATUS:

403 Forbidden

RESPONSE:

Forbidden: Admin access required


==================================================
10. FRONTEND USER FLOW
==================================================

USER REGISTRATION

POST /api/users/register

        |
        v

USER LOGIN

POST /api/users/login

        |
        v

RECEIVE TOKEN

        |
        v

SAVE TOKEN

        |
        v

USER CAN VIEW ALL ITEMS

GET /api/items

        |

USER CAN VIEW A SINGLE ITEM

GET /api/items/:id

        |

AUTHENTICATED USER CAN CREATE ITEM

POST /api/items

Authorization: Bearer USER_TOKEN

        |

IF USER ROLE IS ADMIN

        |
        v

ADMIN CAN VIEW ALL ITEMS

GET /api/admin/items

Authorization: Bearer ADMIN_TOKEN


==================================================
11. FINAL API LIST
==================================================

1. Register User

POST /api/users/register


2. Login User

POST /api/users/login


3. Get All Items

GET /api/items


4. Get Item By ID

GET /api/items/:id


5. Create Lost or Found Item

POST /api/items


6. Admin Get All Items

GET /api/admin/items


==================================================
12. IMPORTANT NOTES FOR FRONTEND DEVELOPER
==================================================

1. The backend runs on:

http://127.0.0.1:18080


2. Login returns an authentication token.


3. Save the token after login.


4. Send the token using:

Authorization: Bearer YOUR_TOKEN


5. POST /api/items requires authentication.


6. The frontend must not send user_id while creating an item.

The backend automatically identifies the user from the authentication token.


7. GET /api/admin/items requires:

- A valid authentication token
- A user with the admin role


8. Normal users can access:

GET /api/items

GET /api/items/:id


9. The frontend should handle these status codes:

200 OK
201 Created
400 Bad Request
401 Unauthorized
403 Forbidden
404 Not Found
500 Internal Server Error


==================================================
END OF API DOCUMENTATION
==================================================