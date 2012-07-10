
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

#include "ipcheck.h"

#include <stdlib.h>
#include <string.h>

int isipoctetchar_check(char c, int type) {
    if (c >= '0' && c <= '9') {
        return 1;
    }
    if (c >= 'a' && c <= 'f' && type == 1) {
        return 1;
    }
    if (c >= 'A' && c <= 'F' && type == 1) {
        return 1;
    }
    return 0;
}
int isip_check(const char* str, int type) {
    char val[] = "";
    int octet = 1;
    int octetlen = 0;
    int previousiscolon = 1;
    int doublecolon = 0;
    char colonchar = '.';
    if (type == 1) { // the colon type we want to look for
        colonchar = ':';
        previousiscolon = 0; // we may start with a colon when using ipv6
    }
    int len = strlen(str);
    int r = 0;
    while (r < len) {
        if (!isipoctetchar_check(str[r],type)) { // are we still in an octet? probably not:
            if (str[r] == colonchar && !previousiscolon) { // check if it's a proper separator ending an octet
                if (r == 0) { // IPv6: we *need* to be at a double colon. a single colon at start is forbidden
                    if (len < 2) {
                        return 0;
                    }
                    if (str[r+1] != colonchar) {
                        // we require a double colon
                        return 0;
                    }
                    previousiscolon = 1;
                    octet = 1;r++;continue;
                }
                octet++;octetlen = 0;
                if (octet > 4 && type != 1) {
                    // IPv4: too many octets
                    return 0;
                }
                if (octet > 8 && type == 1 && r > 0) {
                    // IPv6: too many octets
                    return 0;
                }
                if (type != 1) {
                    // IPv4: value may be only 0-255
                    if (atoi(val) > 255) {
                        return 0;
                    }
                    strcpy(val,"");
                }
                previousiscolon = 1;
                r++;continue;
            }
            if (str[r] == colonchar && previousiscolon == 1 && doublecolon == 0 && type == 1) {
                // check if it's a proper double separator in ipv6
                doublecolon = 1;
                r++;continue;
            }
            // invalid double colon use
            return 0;
        }else{
            previousiscolon = 0;
            octetlen++;
            if (octetlen > 3 && type != 1) {
                return 0;
            }
            if (octetlen > 4 && type == 1) {
                return 0;
            }
            if (type != 1) {
                val[strlen(val)+1] = 0;
                val[strlen(val)] = str[r];
            }
        }
        r++;
    }
    if (previousiscolon == 1) {
        if (type != 1 || r < 2 || str[r-2] != colonchar) { // check whether it is not a valid double colon
            // invalid single colon at end
            return 0;
        }
    }
    if (type != 1) {
        if (atoi(val) > 255) {
            return 0;
        }
    }
    if (octetlen <= 0) {
        octet--;
    }
    if (octet < 3 && type != 1) {
        // too few colons for IPv4
        return 0;
    }
    if (octet < 8 && type == 1 && (doublecolon == 0 || octet <= 0)) {
        // not enough colons for IPv6 and no double colon
        return 0;
    }
    return 1;
}
int isipv4ip(const char* str) {
    return isip_check(str,0);
}
int isipv6ip(const char* str) {
    return isip_check(str,1);
}
