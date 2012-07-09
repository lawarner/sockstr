/*
   Copyright (C) 2012
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

// fb2read.cpp
//
//  Gtk+ GUI application to browse facebook using the SockStr api
//

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sockstr/IPAddress.h>
#include <sockstr/Socket.h>

using namespace sockstr;
using namespace std;


// Info for facebook api testing, application=tsutestapp
static const char* FACEBOOK_APP_ID = "123258924377204";
static const char* FACEBOOK_API_KEY = "3f93db9f0c6ba5f1d05afe13a2932471";

//static const char* MYFACEBOOK_ID = "100000374898275";
static const char* FBURL = "http://www.facebook.com:80";


int main(int argc, const char *argv[])
{
    cout << argv[0] << ": program started." << endl;

    string url = FBURL;

    cout << "Opening url=" << url << endl;

    Socket sock;
    if (!sock.open(url.c_str(), Socket::modeReadWrite))
    {
        cout << "Error opening socket: " << errno << endl;
        return(2);
    }

    cout << "Client socket open at " << (const char *) sock << endl;

	ostringstream fbids;
	ostringstream fbid2;
    char buf[512] = "";

////	fbids << "oauth/access_token?client_id=" << FACEBOOK_APP_ID

//	fbids << "/login.php?api_key=" << FACEBOOK_API_KEY
	fbids << "/common/browser.php?api_key=" << FACEBOOK_API_KEY
		  << "&app_id=" << FACEBOOK_APP_ID
		  << "&fbconnect=true&v=1.0"
		  << "&connect_display=popup"
		  << "&extern=true"
		  << "&session_key_only=true"
		  << "&next=http://www.facebook.com/connect/login_success.html"
		  << "&cancel_url=http://www.facebook.com/connect/login_failure.html"
		  << "&req_perms=read_stream,publish_stream,offline_access";

	fbid2 << "/common/browser.php?api_id=" << FACEBOOK_API_KEY
		  << "&fbconnect=true&v=1.0"
		  << "&connect_display=popup&extern=true"
		  << "&session_key_only=true"
		  << "&next=http://www.facebook.com/connect/login_success.html"
		  << "&cancel_url=http://www.facebook.com/connect/login_failure.html"
		  << "&req_perms=read_stream,offline_access";

    string http_get = "GET " + fbid2.str() + "  HTTP/1.1\r\n"
        + "Host: www.facebook.com\r\n"
		+ "Authorization: Plaintext realm=\"http://facebook.com/\"\r\n"
        + "Content-Type: application/x-www-form-urlencoded\r\n"
//        + "Content-Type: application/xml\r\n"
        + "Accept: */*\r\n"
        + "\r\n";
    sock.write(http_get.c_str(), http_get.length());

    string strbuf;
    string headerbuf;
    string contentbuf;
    int crlfpos = -1;
    int inlen;
    while ((inlen = sock.read(buf, sizeof(buf))) > 0)
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
                inlen = sock.read(buf, ilen);
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
    }
    cout << "====== Read " << strbuf.length() << " bytes from socket:"
         << endl << strbuf << endl;

    sock.close();

    return(0);
}
