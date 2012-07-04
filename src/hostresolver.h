
/* blitwizard 2d engine - source code file

  Copyright (C) 2012 Jonas Thiem

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

*/

// asynchronous DNS facility

void* hostresolv_ReverseLookupRequest(const char* ip);
 // Start a reverse lookup request of that ip (no matter if an ipv4 or ipv6 ip).
 // Returns a request handle

void* hostresolv_LookupRequest(const char* host, int ipv6);
 // Start a normal lookup request of that host.
 // Set ipv6 to 1 if you want an ipv6 ip (otherwise to 0).
 // Returns a request handle

int hostresolv_GetRequestStatus(void* handle);
 // Get the current status of the request
#define RESOLVESTATUS_PENDING 1
#define RESOLVESTATUS_SUCCESS 2
#define RESOLVESTATUS_FAILURE 3

const char* hostresolv_GetRequestResult(void* handle);
 // Get the result of the lookup.
 // Only use this if the request status is RESOLVESTATUS_SUCCESS!

void hostresolv_FreeRequest(void* handle);
 // Free the request and all associated data (handle will become invalid).
 // NEVER use this if the request is still pending (will break horribly),
 // see also hostresolv_CancelRequest below.

void hostresolv_CancelRequest(void* handle);
 // Cancel a host resolve request. If the request is already terminated,
 // it will be instantly free'd. Otherwise it will be (automatically)
 // free'd as soon as possible (the cleanup will be processed whenever you
 // call hostresolv_GetRequestResult for any other host request)


