/*
   Copyright (C) 2013
   Andy Warner
   This file is part of the sockstr class library.

   The sockstr class library is free software; you can redistribute it and/or
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

// httptest.cpp
//
//  Test program for the HttpStream class
//

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sockstr/HttpStream.h>
#include <sockstr/OAuth.h>

using namespace sockstr;
using namespace std;


//static const char* DEFAULT_URL = "http://www.stackoverflow.com:80";
static const char* DEFAULT_URL = "http://localhost";


int main(int argc, const char *argv[])
{
    cout << argv[0] << ": program started." << endl;

    string url = (argc > 1) ? argv[1] : DEFAULT_URL;
    cout << "Opening url=" << url << endl;

    HttpStream http;
    if (!http.open(url.c_str(), Socket::modeReadWrite))
    {
        cout << "Error opening socket: "
             << errno << ": " << strerror(errno) << endl;
        return(2);
    }

    string strhost((const char *) http);
    strhost = strhost.substr(0, strhost.find_first_of(':'));
    cout << "Client socket open at " << strhost << endl;

    TimestampEncoder dateTime(true);
    OAuthNonceEncoder *nonce1 = new OAuthNonceEncoder(time(0)+1);
    OAuthNonceEncoder *nonce2 = new OAuthNonceEncoder(time(0)+7);
    CompoundEncoder cme;
    cme.addElement(nonce1);
    cme.addElement(nonce2);
    OAuthParamEncoder oauth(url, "consumerKey");
    oauth.setToken("MY TOKEN", "Token Secret");

    http.loadDefaultHeaders();
    http.addHeader("Host", strhost);
    http.addHeader("Date", dateTime.toString());
    http.addHeader("X-Compound", cme.toString());
    http.addHeader("X-Authorize", oauth.toString());
/**
		+ "Authorization: Plaintext realm=\"http://domain.com/\"\r\n"
        + "Content-Type: application/x-www-form-urlencoded  application/xml"
**/

    char buf[512] = "";
    string strbuf;
    string headerbuf;
    string contentbuf;
    int crlfpos = -1;

    int inlen = http.get("/", buf, sizeof(buf));
    while (inlen > 0)
    {
        bool isContentEnd = false;
        // cout << "+=+=+=+=+ Read from socket " << inlen << " characters:" << endl
        //           << string(buf).substr(0, inlen) << endl;
        if (isContentEnd)
        {
            strbuf.erase();
            isContentEnd = false;
        }
        strbuf += string(buf, inlen);

        unsigned int ipos = strbuf.find("Content-Length: ");
        if (ipos != string::npos)
        {
            ipos += 16;         // skip past "Content-Length: "
            crlfpos = strbuf.substr(ipos).find("\n");
            int content_len = strtol(strbuf.substr(ipos, crlfpos).c_str(), NULL, 10);
            //cout << " Content length is: " << content_len
            //          << ".  snippet: " << strbuf.substr(ipos + crlfpos + 2, 10) << endl;
            contentbuf = strbuf.substr(ipos + crlfpos + 2);
            headerbuf = strbuf.substr(0, ipos + crlfpos + 2);

            size_t bufsize = sizeof(buf);
            for (int idx = contentbuf.length(); idx < content_len; )
            {
                size_t seglen = content_len - idx + 1;
                int ilen = min(bufsize, seglen);
//                int ilen = min(sizeof(buf), (unsigned int)(content_len - idx + 1));
                inlen = http.read(buf, ilen);
                if (inlen <= 0)
                    break;
                contentbuf += string(buf, inlen);
                //cout << " +++buf: " << string(buf).substr(0,60) << "..." << endl;
                idx += inlen;
            }
            strbuf = headerbuf + contentbuf;
            isContentEnd = true;
            break;
        }

        // End of JSON detected?
        if (strbuf.rfind("\"}") == (strbuf.length() - 2))
        {
            cout << "End of JSON found." << endl;
            isContentEnd = true;
        }
        else if (strbuf.rfind("</html>") != strbuf.npos
                 || strbuf.rfind("</HTML>") != strbuf.npos)
        {
            cout << "End of HTML found." << endl;
            isContentEnd = true;
        }

		cout << "%%%%%%%% WHOLE BUFFER %%%%%%%%%%%%%" << endl
             << strbuf << endl
             << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;

        inlen = http.read(buf, sizeof(buf));
    }
    cout << "====== Read " << strbuf.length() << " bytes from socket:"
         << endl << strbuf << endl;

    http.close();

    return(0);
}
