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

// ipctest-gtk.cpp
//
// This is a testing tool used to create and execute various collections
// of tests.  The underlying IPC mechanism uses the sockstr library.

#include <iostream>
#include <gtkmm.h>

#include "MainWindow.h"

#ifndef GTK_VERSION_GE
#define GTK_VERSION_GE(x,y) 1
#endif


int main(int argc, char* argv[])
{
    std::cout << "Program ipctest-gtk start." << std::endl;

#if GTK_VERSION_GE(3,4)
    int fakeargc = 1;
    Glib::RefPtr<Gtk::Application> app
        = Gtk::Application::create(fakeargc, argv, "sockstr.ipctest.gtk");
#else
    Gtk::Main kit(argc, argv);
#endif
    std::string defFilename =  "t";
    std::string testFilename = "t.its";
#if 0
    if (argc < 2)
    {
        std::cerr << "Usage: ipctest-gtk <ipcdef_filename> <testcase>" << std::endl;
        return 1;
    }
#else
    if (argc > 1)
        defFilename = argv[1];
    if (argc > 2)
        testFilename = argv[2];
#endif

    std::cout << "Defs=" << defFilename << ", testcase=" << testFilename << std::endl;

    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file("ipctest-gtk.glade");

    ipctest::uigtk::MainWindow* mainWindow;
    builder->get_widget_derived("IpcTestMainWindow", mainWindow);
    if (mainWindow)
    {
        mainWindow->setup(defFilename, testFilename);
#if GTK_VERSION_GE(3,4)
        return app->run(*mainWindow);
#else
        kit.run(*mainWindow);
        return 0;
#endif
    }

    std::cerr << "Could not initialize main Gtk window" << std::endl;
    return 2;
}
