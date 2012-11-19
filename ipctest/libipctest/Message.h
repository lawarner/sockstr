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

#ifndef _MESSAGE_H_INCLUDED_
#define _MESSAGE_H_INCLUDED_
//
//

//
// INCLUDE FILES
//
#include <string>
#include <vector>


namespace ipctest
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
class Field;
class Message;
class Params;


typedef std::vector<Field *>  FieldsArray;
typedef FieldsArray::iterator FieldsIterator;
typedef FieldsArray::const_iterator FieldsConstIterator;
typedef std::vector<Message *> MessageList;
//
// CLASS DEFINITIONS
//
class Message
{
public:
    Message(const std::string& name, int ordinal = -1);
    virtual ~Message();

    void addField(Field* field);
    int bumpSequence(bool incr = true);
    const FieldsArray& getFields() const;
    const std::string& getName() const;
    const int getOrdinal() const;
    const int getSize() const;

    /*! Take array of field values in string format and pack them into
     *  a buffer of internal representation, ready to be sent over the wire.
     *  @param vals array of field values in string format
     *  @param buf buffer in internal format (must be at least size of message).
     *  @return the total size packed.
     */
    int packFields(const std::vector<std::string>& vals, char* buf) const;

    /*! Take a set of Params containing field name/value pairs in string format and pack 
     *  them into a buffer of internal representation, ready to be sent over the wire.
     *  @param params dictionary of field name/values
     *  @param buf buffer in internal format (must be at least size of message).
     *  @return the total size packed.
     */
    int packParams(const Params& params, char* buf) const;

    /*! Take an internal buffer received and unpack into an array of field
     *  values in string format.
     *  @param buf buffer in internal format (must be at least size of message).
     *  @param vals array of field values in string format
     *  @return the total size unpacked.
     */
    int unpackFields(const char* buf, std::vector<std::string>& vals) const;

    int unpackParams(char* buf, Params& params) const;
    int unpackParams(const std::vector<std::string>& vals, Params& params) const;

    std::string toXml(int indent, const char* buf = 0) const;

private:
    std::string name_;
    int ordinal_;
    int totalSize_;

    FieldsArray fields_;

    static int ordinalSeq;
    static int messageSeq;
};

}  // namespace sockstr

#endif
