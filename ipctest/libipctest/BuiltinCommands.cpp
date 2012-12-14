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

// BuiltinCommands.cpp
//
#include <sockstr/IPC.h>
#include <sockstr/Socket.h>

#include "BuiltinCommands.h"
#include "Condition.h"
#include "Log.h"
#include "Message.h"
#include "Parser.h"
#include "RunContext.h"
#include "Serializer.h"
#include <sstream>

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace ipctest;
using namespace std;


// Comment
CommandComment::CommandComment(const std::string& comment)
    : Command("Comment", "Comment", comment)
{
//    params_->set("Comment", comment);
}


CommandComment::CommandComment(Params* params, Message* msg)
    : Command("Comment", params, msg)
{

}

Command* CommandComment::createCommand(Params* params, Message* msg)
{
    params->set("Comment", params->get("_cdata_line"));

    return new CommandComment(params, msg);
}

bool CommandComment::execute(RunContext& context)
{
    std::cout << toString() << std::endl;
    return true;
}

std::string CommandComment::toString() 
{
    std::string str;
    params_->get("Comment", str);
    return commandName_ + ": " + str; 
}

std::string CommandComment::toXml(int indent)
{
    std::string str = getXmlPart(indent, true);
    str += Serializer::encodeString(params_->get("Comment")) + "\n";
    str += getXmlPart(indent, false);

    return str;
}

// Connect
CommandConnect::CommandConnect(const std::string& url)
    : Command("Connect", "Url", url)
{

}

CommandConnect::CommandConnect(Params* params, Message* msg)
    : Command("Connect", params, msg)
{

}

Command* CommandConnect::createCommand(Params* params, Message* msg)
{
    params->set("Url", params->get("_cdata_line"));

    return new CommandConnect(params, msg);
}

bool CommandConnect::execute(RunContext& context)
{
    std::string url = params_->get("Url");
    sockstr::Socket* sock = new sockstr::Socket(url.c_str(), sockstr::Socket::modeReadWrite);
    if (sock->queryStatus() != sockstr::SC_OK)
        return false;

    data_ = sock;
    context.setSocket(sock);
    return true;
}

sockstr::Socket* CommandConnect::getSocket() const
{
    return (sockstr::Socket*) data_;
}

std::string CommandConnect::toString()
{
    return commandName_ + ": " + params_->get("Url");
}


std::string CommandConnect::toXml(int indent)
{
    std::string str(getXmlPart(indent, true));
    str += params_->get("Url") + "\n";
    str += getXmlPart(indent, false);

    return str;
}


//  Disconnect
CommandDisconnect::CommandDisconnect()
    : Command("Disconnect")
{

}

CommandDisconnect::CommandDisconnect(Params* params, Message* msg)
    : Command("Disconnect", params, msg)
{

}

Command* CommandDisconnect::createCommand(Params* params, Message* msg)
{
    return new CommandDisconnect(params, msg);
}

bool CommandDisconnect::execute(RunContext& context)
{
    sockstr::Socket* sock = context.getSocket();
    if (!sock)
        return false;

    sock->close();

    return true;
}


//  Function
CommandFunction::CommandFunction(const std::string& funcName, void* data, int delay)
    : Command("Function", "Function Name", funcName)
{

}

CommandFunction::CommandFunction(Params* params, Message* msg)
    : Command("Function", params, msg)
{

}

void CommandFunction::addCommand(Command* cmd)
{
    commands_.push_back(cmd);
}

Command* CommandFunction::createCommand(Params* params, Message* msg)
{
    params->set("Function Name", params->get("_name"));
    return new CommandFunction(params, msg);
}

bool CommandFunction::execute(RunContext& context)
{
    std::string funcName;
    params_->get("Function Name", funcName);
    std::cout << "exec function: " << funcName << std::endl;

    CommandIterator it = commands_.begin();
    for ( ; it != commands_.end(); ++it)
        if (!(*it)->execute(context))
            return false;       // stop on first error

    return true;
}

std::string CommandFunction::toString() 
{
    std::string str;
    params_->get("Function Name", str);
    return commandName_ + ": " + str; 
}


std::string CommandFunction::toXml(int indent)
{
    std::string str(getXmlPart(indent, true));
    str += getXmlPart(indent, false);

    return str;
}


// If
CommandIf::CommandIf(Condition* condition, Command* cmd)
    : Command("If", "Condition", condition->toString())
    , condition_(condition)
{
    if (cmd) 
        commands_.push_back(cmd);
}

CommandIf::CommandIf(Params* params, Message* msg)
    : Command("If", params, msg)
    , condition_(Condition::createCondition(params->get("Condition")))
{

}

void CommandIf::addCondition(Condition* condition)
{
    condition_ = condition;
}

Command* CommandIf::createCommand(Params* params, Message* msg)
{
    params->set("Condition", params->get("_condition"));

    return new CommandIf(params, msg);
}

bool CommandIf::execute(RunContext& context)
{
    condition_->setParams(params_);
//    std::cout << "param is " << params_ << std::endl;

    std::cout << "exec if: " << condition_->toString() << std::endl;

    bool okStatus = true;
    bool condResult = (*condition_)(context);

    // Execute embedded commands
    if (condResult)
    {
        CommandIterator it = commands_.begin();
        for ( ; it != commands_.end(); ++it)
            if (!(*it)->execute(context))
            {
                okStatus = false;
                break;       // stop on first error
            }
    }

    // Execute next commands in run context that are higher nesting level
    CommandList* cmdList = context.getCommands();
    if (cmdList)
    {
        Command* cmd = 0;
        CommandIterator it;
        CommandIterator ij;
        it = context.getCommandIterator();	// cmdList->begin();
        ij = it;
        for (++it ; it != cmdList->end(); ++it)
        {
            cmd = *it;
            if (cmd->getLevel() <= level_)
                break;

            if (condResult)
            {
//                std::cout << "-if command (exec) " << cmd->getName() << std::endl;
                if (okStatus && !cmd->execute(context))
                    okStatus = false;
            }

            ij = it;
        }

        // check for Else and skip past commands
        if (cmd && cmd->getName() == "Else" && condResult)
        {
            ij = it;
            for (++it ; it != cmdList->end(); ++it)
            {
                cmd = *it;
                if (cmd->getLevel() <= level_)
                    break;
                ij = it;
            }
        }

        context.setCommandIterator(ij);
    }


    return okStatus;
}

std::string CommandIf::toString()
{
    std::string strcond;
    params_->get("Condition", strcond);
    return commandName_ + ": " + strcond;
}

std::string CommandIf::toXml(int indent)
{
    std::string strcond;
    params_->get("Condition", strcond);
    if (strcond.empty())
        strcond = "false";
    strcond = "condition=\"" + strcond + "\"";
    std::string str = getXmlPart(indent, strcond, true);
    str += getXmlPart(indent, false);
    return str;
}


// Else
CommandElse::CommandElse(Command* cmd)
    : Command("Else", (void *) 0, 0)
{
    if (cmd) 
        commands_.push_back(cmd);
}

CommandElse::CommandElse(Params* params, Message* msg)
    : Command("Else", params, msg)
{

}

Command* CommandElse::createCommand(Params* params, Message* msg)
{
    return new CommandElse(params, msg);
}

bool CommandElse::execute(RunContext& context)
{
    bool condition = true;
    std::cout << "exec else: " << std::endl;

    bool okStatus = true;

    // Execute embedded commands
    if (condition)
    {
        CommandIterator it = commands_.begin();
        for ( ; it != commands_.end(); ++it)
            if (!(*it)->execute(context))
            {
                okStatus = false;
                break;       // stop on first error
            }
    }

    // Execute next commands in run context that are higher nesting level
    CommandList* cmdList = context.getCommands();
    if (cmdList)
    {
        CommandIterator it;
        CommandIterator ij;
        it = context.getCommandIterator();
        ij = it;
        for (++it ; it != cmdList->end(); ++it)
        {
            Command* cmd = *it;
            if (cmd->getLevel() <= level_)
                break;

            if (condition)
            {
//                std::cout << "-else command (exec) " << cmd->getName() << std::endl;
                if (okStatus && !cmd->execute(context))
                    okStatus = false;
            }

            ij = it;
        }
        context.setCommandIterator(ij);
    }

    return okStatus;
}


std::string CommandElse::toXml(int indent)
{
    std::string str;
    str = getXmlPart(indent, true) + getXmlPart(indent, false);
    return str;
}


// Exec
CommandExec::CommandExec(const std::string& cmdLine, bool waitFor, void* data, int delay)
    : Command("Exec", "Command Line", cmdLine)
    , commandLine_(cmdLine)
    , waitFor_(waitFor)
    , pid_(-1)
{
    params_->set("Wait for command", waitFor_);
}

CommandExec::CommandExec(Params* params, Message* msg)
    : Command("Exec", params, msg)
    , waitFor_(false)
    , pid_(-1)
{
    initParams();
}

Command* CommandExec::createCommand(Params* params, Message* msg)
{
    params->set("Command Line", params->get("_cdata_line"));
    params->set("Wait for command", params->get("_wait"));

    return new CommandExec(params, msg);
}

bool CommandExec::execute(RunContext& context)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        LOG << "Error: could not fork child process." << std::endl;
        return false;
    }

    if (pid > 0)
    {
        pid_ = pid;
        if (waitFor_)
        {
            LOG << "Waiting for child pid=" << pid_ << endl;
            int child_stat;
            waitpid(pid_, &child_stat, 0);
        }
        return true;
    }

    std::string cmdline = params_->get("Command Line");
    vector<string> cmdv;
    Parser::splitTokens(cmdline, cmdv, " \t\n\r");

    vector<const char*> cmdpv;
    vector<string>::iterator it = cmdv.begin();
    for ( ; it != cmdv.end(); ++it)
    {
        cmdpv.push_back((*it).c_str());
    }
    cmdpv.push_back(0);

    pid = 0;	// Mark as child
//    execlp(cmdv[0].c_str(), cmdv[0].c_str(), (char *)0);
    execvp(cmdpv[0], (char* const *)cmdpv.data());

    // We only fall thru on error
    LOG << "Could not exec: " << cmdv[0] << endl;
    _exit(1);
    return false;	// NOT REACHED
}

void CommandExec::initParams()
{
    params_->get("Command Line", commandLine_);
    params_->get("Wait for command", waitFor_);
}

std::string CommandExec::toString()
{
    return commandName_ + ": " + commandLine_;
}

std::string CommandExec::toXml(int indent)
{
    std::string strattr = " wait=\"" + (waitFor_ ? string("true") : string("false")) + "\"";
    std::string str = getXmlPart(indent, strattr, true);
    str += Serializer::encodeString(params_->get("Command Line")) + "\n";
    str += getXmlPart(indent, false);

    return str;
}


// Loop
CommandLoop::CommandLoop(int iters, Command* cmd)
    : Command("Loop", "Iterations", iters)
    , curr_(0)
{
    addCommand(cmd);
}

CommandLoop::CommandLoop(Params* params, Message* msg)
    : Command("Loop", params, msg)
    , curr_(0)
{

}

void CommandLoop::addCommand(Command* cmd)
{
    currLevel_++;
    if (cmd)
        commands_.push_back(cmd);
    currLevel_--;
}

//   virtual
Command* CommandLoop::createCommand(Params* params, Message* msg)
{
    params->set("Iterations", params->get("_iterations"));

    return new CommandLoop(params, msg);
}

bool CommandLoop::execute(RunContext& context)
{
    int iters = 1;
    params_->get("Iterations", iters);
    std::cout << "exec loop: " << iters << std::endl;

    bool okStatus = true;

    // Execute embedded commands
    for (curr_ = 0; okStatus && curr_ < iters; curr_++)
    {
        CommandIterator it = commands_.begin();
        for ( ; it != commands_.end(); ++it)
            if (!(*it)->execute(context))
            {
                okStatus = false;
                break;       // stop on first error
            }
    }

    // Execute next commands in run context that are higher nesting level
    CommandList* cmdList = context.getCommands();
    if (cmdList)
    {
        CommandIterator it;
        CommandIterator ij;
        for (curr_ = 0; curr_ < iters; curr_++)
        {
            it = context.getCommandIterator();	// cmdList->begin();
            ij = it;
            for (++it ; it != cmdList->end(); ++it)
            {
                Command* cmd = *it;
//                std::cout << "-loop command (exec) " << cmd->getName() << std::endl;
                if (cmd->getLevel() <= level_)
                    break;

                if (okStatus && !cmd->execute(context))
                    okStatus = false;

                ij = it;
            }
        }
        if (iters > 0)
            context.setCommandIterator(ij);
    }

    return okStatus;
}

std::string CommandLoop::toString()
{
//    std::ostringstream oss;
//    oss << iters_;
//    return commandName_ + ": " + oss.str(); 
    std::string striter;
    params_->get("Iterations", striter);
    return commandName_ + ": " + striter;
}

std::string CommandLoop::toXml(int indent)
{
    std::string striter;
    params_->get("Iterations", striter);
    if (striter.empty())
        striter = "1";
    striter = "iterations=\"" + striter + "\"";

    std::string str = getXmlPart(indent, striter, true) + getXmlPart(indent, false);
    return str;
}


// Receive
CommandReceive::CommandReceive(Message* msg)
    : Command("Receive")
{
    message_ = msg ? msg : emptyMessage_;
}

CommandReceive::CommandReceive(Params* params, Message* msg)
    : Command("Receive", params, msg)
{

}

Command* CommandReceive::createCommand(Params* params, Message* msg)
{
    return new CommandReceive(params, msg);
}

bool CommandReceive::execute(RunContext& context)
{
    Message* msg = context.getMessage();
    sockstr::Socket* sock = context.getSocket();

    if (msg && sock)
    {
        message_ = msg;
        if (message_->getName().empty())
            return false;
        std::cout << "exec: " << toString() << std::endl;

        sockstr::GenericReply reply;
        sock->setAsyncMode(true); 	// Don't block if no data available
        UINT sz = sock->read(&reply, sizeof(reply));
        if (sz == 0)
            return false;

        std::cout << "Got " << sz << " bytes, function=" << reply.wFunction_
                  << ", seq=" << reply.dwSequence_ << std::endl;
        if (sz > 12)
        {
            std::vector<std::string> fldValues;
            int szdata = msg->unpackFields(reply.filler_, fldValues);
            if (szdata)
            {
                char* databuf = new char[szdata];
                memcpy(databuf, reply.filler_, szdata);
                setData(databuf);
                context.setMessage(msg);
                context.setFieldValues(fldValues);
            }
        }
    }

    return true;
}

std::string CommandReceive::toString()
{
    return commandName_ + ": " + message_->getName();
}


std::string CommandReceive::toXml(int indent)
{
    std::string strmsg = "message=\"" + message_->getName() + "\"";
    std::string str = getXmlPart(indent, strmsg, true);

    // get message and field names/values
    str += message_->toXml(indent + 4, (const char *) getData());
    str += getXmlPart(indent, false);

    return str;
}


// Send
CommandSend::CommandSend(Message* msg, void* msgData)
    : Command("Send", msgData)
{
    message_ = msg ? msg : emptyMessage_;
}

CommandSend::CommandSend(Params* params, Message* msg)
    : Command("Send", params, msg)
{

}

Command* CommandSend::createCommand(Params* params, Message* msg)
{
    return new CommandSend(params, msg);
}

bool CommandSend::execute(RunContext& context)
{
    Message* msg = context.getMessage();
    sockstr::Socket* sock = context.getSocket();

    if (msg && sock)
    {
        std::cout << " Yes, we rock with message and socks!" << std::endl;
        message_ = msg;
        if (message_->getName().empty())
            return false;

        std::cout << "exec: " << toString() << std::endl;
        sockstr::GenericReply basicMsg;
        // Write header and payload together
        basicMsg.wFunction_   = msg->getOrdinal();
        basicMsg.wPacketSize_ = msg->getSize();
        basicMsg.dwSequence_  = msg->bumpSequence();	// cookie
        int szdata = msg->packFields(context.getFieldValues(), basicMsg.filler_);

        if (szdata > 0)
        {
            char* databuf = new char[szdata];
            memcpy(databuf, basicMsg.filler_, szdata);
            setData(databuf);
        }

        sock->write(&basicMsg, sizeof(sockstr::IpcStruct) + basicMsg.wPacketSize_);
    }
    else
        return false;

    return true;
}


std::string CommandSend::toString()
{
    return commandName_ + ": " + message_->getName();
}

std::string CommandSend::toXml(int indent)
{
    std::string str(indent, ' ');
    str += "<" + commandName_ + " message=\"" + message_->getName() + "\">\n";
    // get message and field names/values
    str += message_->toXml(indent + 4, (const char*) getData());
    str += getXmlPart(indent, false);

    return str;
}


// While
CommandWhile::CommandWhile(Condition* condition, Command* cmd)
    : Command("While", "Condition", condition->toString())
    , condition_(condition)
{
    if (cmd) 
        commands_.push_back(cmd);
}

CommandWhile::CommandWhile(Params* params, Message* msg)
    : Command("While", params, msg)
    , condition_(Condition::createCondition(params->get("Condition")))
{

}

void CommandWhile::addCondition(Condition* condition)
{
    condition_ = condition;
}

Command* CommandWhile::createCommand(Params* params, Message* msg)
{
    params->set("Condition", params->get("_condition"));

    return new CommandWhile(params, msg);
}

bool CommandWhile::execute(RunContext& context)
{
    condition_->setParams(params_);
//    std::cout << "param is " << params_ << std::endl;

    LOG << "exec while: " << condition_->toString() << std::endl;

    bool okStatus = true;

    while (okStatus && (*condition_)(context))
    {
        // Execute embedded commands
        CommandIterator it = commands_.begin();
        for ( ; it != commands_.end(); ++it)
            if (!(*it)->execute(context))
            {
                okStatus = false;
                break;       // stop on first error
            }

        // Execute next commands in run context that are higher nesting level
        if (okStatus)
        {
            CommandIterator it = context.getCommandIterator();
            CommandIterator ij = context.getCommandIterator(level_);
            for (++it, ++ij ; it != ij; ++it)
            {
                Command* cmd = *it;
                // stop executing on first error, but still have to skip to end
                LOG << "Executing from While:" << std::endl;
                if (!cmd->execute(context))
                {
                    okStatus = false;
                    break;
                }
            }
        }
//        condStatus = (*condition_)(context);
//        LOG << "condStatus is " << condStatus << " okStatus is " << okStatus << endl;
    }

    // Update command to run after our nested commands
    context.setCommandIterator(context.getCommandIterator(level_));

    return okStatus;
}


std::string CommandWhile::toString()
{
    std::string strcond;
    params_->get("Condition", strcond);
    return commandName_ + ": " + strcond;
}

std::string CommandWhile::toXml(int indent)
{
    std::string strcond;
    params_->get("Condition", strcond);
    if (strcond.empty())
        strcond = "false";
    strcond = " condition=\"" + strcond + "\"";
    std::string str = getXmlPart(indent, strcond, true);
    str += getXmlPart(indent, false);
    return str;
}

