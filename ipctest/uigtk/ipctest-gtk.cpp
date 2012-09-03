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



int main(int argc, char* argv[])
{
    std::cout << "Program ipctest-gtk start." << std::endl;

    Glib::RefPtr<Gtk::Application> app
        = Gtk::Application::create(argc, argv, "sockstr.ipctest.gtk");

    if (argc < 2)
    {
        std::cerr << "Usage: ipctest-gtk <ipcdef_filename>" << std::endl;
        return 1;
    }
    std::string defFilename = argv[1];

    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file("ipctest-gtk.glade");

    ipctest::uigtk::MainWindow* mainWindow;
    builder->get_widget_derived("IpcTestMainWindow", mainWindow);
    if (mainWindow)
    {
        mainWindow->setup(defFilename);
        return app->run(*mainWindow);
    }

    std::cerr << "Could not initialize main Gtk window" << std::endl;
    return 2;
}
