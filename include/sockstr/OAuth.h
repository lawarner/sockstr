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

#ifndef _OAUTH_H_INCLUDED_
#define _OAUTH_H_INCLUDED_
//
//

//
// INCLUDE FILES
//
#include <sockstr/HttpHelpers.h>


namespace sockstr
{

//
// MACRO DEFINITIONS
//
#ifndef DllExport
#define DllExport
#endif

//
// FORWARD CLASS DECLARATIONS
//

//
// TYPE DEFINITIONS
//

//
// CLASS DEFINITIONS
//

/**
 * Handles OAuth nonce encoding that outputs a random, non-repeating string value.
 * Used internally by the OAuthParamEncoder class.
 */
class DllExport OAuthNonceEncoder : public HttpParamEncoder
{
public:
    OAuthNonceEncoder(unsigned int seed = time(0));
    virtual std::string toString();

    const char* getValidCharacters();

private:
    unsigned int seed_;
    char nonce_[32];

    static const char* validChars_;
};

/**
 * Handles OAuth signature.
 * Used internally by the OAuthParamEncoder class.
 */
class DllExport OAuthSignatureEncoder : public HttpParamEncoder
{
public:
    OAuthSignatureEncoder();

    void computeSignature(const std::string& httpMethod,
                          std::vector<HttpParamEncoder*> encoders);
};


/**
 * Handles the various header components for an OAuth session.
 */
class DllExport OAuthParamEncoder : public CompoundEncoder
{
public:
    OAuthParamEncoder(const std::string& realm, const std::string& key = std::string());
    virtual ~OAuthParamEncoder();

    virtual std::string toString();

    void setToken(const std::string& token, const std::string& secret);

private:
    std::string consumerKey_;
    std::string token_;
    std::string tokenSecret_;
    OAuthSignatureEncoder* signatureEncoder_;
};

}  // namespace sockstr

#endif // _OAUTH_H_INCLUDED_
