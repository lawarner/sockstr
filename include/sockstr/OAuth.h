/*
   Copyright (C) 2013, 2014
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

#pragma once

#include <sockstr/HttpHelpers.h>
#include <memory>

namespace sockstr {

#ifndef DllExport
#define DllExport
#endif

class DllExport OAuthAuthenticator {
public:
    /**
     * Signature of callback.  This class will allocate and free the provided buffer.
     * @param buf This buffer is allocated and provided by this class.  The callback
     *            implementation is expected to copy a null terminated string
     *            representing the client secret.  The buffer is 64 bytes by 
     *            default (see #setClientSecretHook).
     * @return The callback should return true if successful, otherwise false.
     */
    typedef bool (*OAuthCallback)(char* buf);

public:
    OAuthAuthenticator();
    OAuthAuthenticator(const CompoundEncoder& otherParameters);

    /**
     * Call the auth server to obtain authentication token.
     * The client secret must be handed off here in a secure manner.
     */
    int authenticate(const std::string& authUri, const std::string& redirectUri);

    /** Set client key and use default format of "consumer_key=xxxxx" */
    void setClientKey(const std::string& clientKey);

    /** Set client key from encoder's name/vale. */
    void setClientKey(HttpParamEncoder* clientKey);

    /**
     * Setup the function that will be called when this class needs to obtain
     * the client secret.
     * @param callback Function that will be called.
     * @param maxKeyLength Size of the buffer that will be passed to the callback
     */
    void setClientSecretHook(OAuthCallback callback, int maxKeyLength = 64);

private:
    CompoundEncoder parameters_;
    CompoundEncoder otherParameters_;

    std::unique_ptr<HttpParamEncoder> clientKey_;
    OAuthCallback callback_;

    std::string authUri_;
    std::string redirectUri_;

    int maxKeyLength_;
};

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
