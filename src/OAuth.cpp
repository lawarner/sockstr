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


const char* OAuthNonceEncoder::validChars_ = "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789_";



OAuthNonceEncoder::OAuthNonceEncoder(unsigned int seed)
    : HttpParamEncoder("oauth_nonce")
    , seed_(seed)
{

}

std::string OAuthNonceEncoder::toString()
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

const char* OAuthNonceEncoder::getValidCharacters()
{
    return validChars_;
}


OAuthSignatureEncoder::OAuthSignatureEncoder()
    : HttpParamEncoder("oauth_signature", "NOT YET COMPUTED")
{

}

void OAuthSignatureEncoder::computeSignature(const std::string& httpMethod,
                                             std::vector<HttpParamEncoder*> encoders)
{
    set("computed.");
}



OAuthParamEncoder::OAuthParamEncoder(const std::string& realm, const std::string& key)
    : CompoundEncoder(", \n    ")	// separator
    , consumerKey_(key)
    , signatureEncoder_(new OAuthSignatureEncoder)
{
    TimestampEncoder *tse = new TimestampEncoder(true, TimestampEncoder::DateTimeRaw);
    tse->setName("oauth_timestamp");

    addElement(new FixedStringEncoder("OAuth realm", realm));
    addElement(new FixedStringEncoder("oauth_consumer_key", consumerKey_));
    addElement(new OAuthNonceEncoder);
    addElement(tse);
    addElement(new FixedStringEncoder("oauth_signature_method", "HMAC-SHA1"));
    addElement(new FixedStringEncoder("oauth_version", "1.0"));
    addElement(signatureEncoder_);
}


OAuthParamEncoder::~OAuthParamEncoder()
{

}

std::string OAuthParamEncoder::toString()
{
    if (token_.empty())
        std::cout << "Warning: no OAuth token set." << std::endl;

    // Update the signature
    signatureEncoder_->computeSignature("GET", encoders_);
    
    return CompoundEncoder::toString();
}

void OAuthParamEncoder::setToken(const std::string& token, const std::string& secret)
{
    token_ = token;
    tokenSecret_ = secret;

    addElement(new FixedStringEncoder("oauth_token", token_));
    addElement(new FixedStringEncoder("oauth_token_secret", tokenSecret_));
}
