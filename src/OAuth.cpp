/*
   Copyright (C) 2013
   Andy Warner
   This file is part of the sockstr class library.

   The sockstr class library and other related programs such as ipctest is free 
   software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The sockstr class library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the sockstr library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

// OAuth.cpp
//
/*
 * This oauth handler is still under development.  It will
 * eventually be able to handle OAuth 2.0 headers, such as:
 * Authorization: OAuth realm="http://photos.example.net/photos",
 *    oauth_consumer_key="dpf43f3p2l4k3l03",
 *    oauth_token="nnch734d00sl2jdk",
 *    oauth_nonce="kllo9940pd9333jh",
 *    oauth_timestamp="1191242096",
 *    oauth_signature_method="HMAC-SHA1",
 *    oauth_version="1.0",
 *    oauth_signature="tR3%2BTy81lMeYAr%2FFid0kMTYa%2FWM%3D"
 */

#include <sockstr/OAuth.h>
#include <stdlib.h>
#include <string.h>
using namespace sockstr;


const char* OauthNonceEncoder::validChars_ = "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789_";



OauthNonceEncoder::OauthNonceEncoder(unsigned int seed)
    : HttpParamEncoder("oauth_nonce")
    , seed_(seed)
{

}

std::string OauthNonceEncoder::toString()
{
    static int szValid = strlen(validChars_);
    int olen = 15 + (rand_r(&seed_) & 0xf);

    for (int i = 0; i < olen; i++)
    {
        int idx = rand_r(&seed_) % szValid;
        nonce_[i] = validChars_[idx];
    }
    nonce_[olen] = 0;

    return std::string(nonce_);
}

const char* OauthNonceEncoder::getValidCharacters()
{
    return validChars_;
}

