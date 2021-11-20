// =============================================================================
//  ImageWindowGTK.hpp
//
//  MIT License
//
//  Copyright (c) 2021 Dairoku Sekiguchi
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
// =============================================================================
/*!
  \file     ImageWindowGTK.h
  \author   Dairoku Sekiguchi
  \version  1.0.0
  \date     2021/11/14
*/
#ifndef IMAGE_WINDOW_GTK_H_
#define IMAGE_WINDOW_GTK_H_
#define SHL_IMAGE_WINDOW_GTK_BASE_VERSION   1

#include <stdio.h>
#include <queue>
#include <mutex>
#include <thread>
#include <gtkmm.h>

// Namespace -------------------------------------------------------------------
namespace shl
{
// -----------------------------------------------------------------------------
//	shl base gtk classes
// -----------------------------------------------------------------------------
#ifndef SHL_BASE_GTK_CLASS_
#define SHL_BASE_GTK_CLASS_
#define SHL_BASE_GTK_CLASS_VERSION  SHL_IMAGE_WINDOW_GTK_BASE_VERSION

  // ---------------------------------------------------------------------------
  //	BaseAppGTK class
  // ---------------------------------------------------------------------------
  class GTK_BaseApp : public Gtk::Application
  {
  protected:
    // -------------------------------------------------------------------------
    // GTK_BaseApp
    // -------------------------------------------------------------------------
    GTK_BaseApp() :
            Gtk::Application("org.gtkmm.examples.application",
                             Gio::APPLICATION_NON_UNIQUE)
    {
      Glib::signal_idle().connect( sigc::mem_fun(*this, &GTK_BaseApp::on_idle) );
    }

    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // on_activate
    // -------------------------------------------------------------------------
    virtual void on_activate()
    {
      // The application has been started, so let's show a window.
      process_added_windows();
    }
    // -------------------------------------------------------------------------
    // add_app_window
    // -------------------------------------------------------------------------
    void add_app_window(Gtk::Window *in_window)
    {
      std::lock_guard<std::mutex> lock(m_add_win_queue_mutex);
      m_add_win_queue.push(in_window);
    }
    // -------------------------------------------------------------------------
    // on_hide_window
    // -------------------------------------------------------------------------
    void on_hide_window(Gtk::Window *in_window)
    {
      //delete in_window;
    }
    // -------------------------------------------------------------------------
    // on_idle
    // -------------------------------------------------------------------------
    bool on_idle()
    {
      process_added_windows();
      return false;
    }
    // -------------------------------------------------------------------------
    // process_added_windows
    // -------------------------------------------------------------------------
    void process_added_windows()
    {
      std::lock_guard<std::mutex> lock(m_add_win_queue_mutex);
      while (!m_add_win_queue.empty())
      {
        Gtk::Window *win = m_add_win_queue.front();
        add_window(*win);
        win->signal_hide().connect(sigc::bind<Gtk::Window*>(
                sigc::mem_fun(*this,
                              &GTK_BaseApp::on_hide_window), win));
        win->present();
        m_add_win_queue.pop();
      }
    }

  private:
    // member variables --------------------------------------------------------
    std::mutex m_add_win_queue_mutex;
    std::queue<Gtk::Window *> m_add_win_queue;

    // friend classes ----------------------------------------------------------
    friend class GTK_BaseAppRunner;
  };
  // ---------------------------------------------------------------------------
  //	BaseAppGTK class
  // ---------------------------------------------------------------------------
  class GTK_BaseAppRunner
  {
  public:
    // static functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // check_runner
    // -------------------------------------------------------------------------
    static bool check_runner()
    {
      if (get_runner() == nullptr)
        return false;
      return true;
    }

  protected:
    // constructors and destructor ----------------------------------------------
    // -------------------------------------------------------------------------
    // GTK_BaseAppRunner
    // -------------------------------------------------------------------------
    GTK_BaseAppRunner()
    {
      m_app = new GTK_BaseApp();
      //create_thread();
    }
    // -------------------------------------------------------------------------
    // GTK_BaseAppRunner
    // -------------------------------------------------------------------------
    virtual ~GTK_BaseAppRunner()
    {
      wait_thread();
      delete m_app;
printf("[DEBUG] I am deleted\n");
    }

    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // create_thread
    // -------------------------------------------------------------------------
    void create_thread()
    {
      if (m_thread != nullptr)
        return;
      m_thread = new std::thread(thread_func, this);
    }
    // -------------------------------------------------------------------------
    // wait_thread
    // -------------------------------------------------------------------------
    void wait_thread()
    {
      if (m_thread == nullptr)
        return;
      m_thread->join();
      delete m_thread;
      m_thread = nullptr;
    }
    // -------------------------------------------------------------------------
    // add_app_window
    // -------------------------------------------------------------------------
    void add_app_window(Gtk::Window *in_window)
    {
      m_app->add_app_window(in_window);
    }

    // static functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // get_runner
    // -------------------------------------------------------------------------
    static GTK_BaseAppRunner *get_runner()
    {
      static GTK_BaseAppRunner s_runner;
      return &s_runner;
    }
    // -------------------------------------------------------------------------
    // get_app
    // -------------------------------------------------------------------------
    static GTK_BaseApp *get_app()
    {
      return get_runner()->m_app;
    }
    // -------------------------------------------------------------------------
    // wait_app
    // -------------------------------------------------------------------------
    static void wait_app()
    {
      get_runner()->wait_thread();
    }

  private:
    // member variables --------------------------------------------------------
    GTK_BaseApp *m_app;
    std::thread *m_thread;

    // static functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // thread_func
    // -------------------------------------------------------------------------
    static void thread_func(GTK_BaseAppRunner *in_obj)
    {
      in_obj->m_app->run();
    }

    // friend classes ----------------------------------------------------------
    friend class GTK_BaseWindow;
  };
  // ---------------------------------------------------------------------------
  //	GTK_BaseWindow class
  // ---------------------------------------------------------------------------
  class GTK_BaseWindow : public Gtk::Window
  {
  public:
    // -------------------------------------------------------------------------
    // GTK_BaseWindow
    // -------------------------------------------------------------------------
    GTK_BaseWindow()
    {
      m_app_base_runner = GTK_BaseAppRunner::get_runner();
    }
    // -------------------------------------------------------------------------
    // GTK_ImageMainWindow
    // -------------------------------------------------------------------------
    virtual ~GTK_BaseWindow()
    {
    }
    // -------------------------------------------------------------------------
    // add_to_app
    // -------------------------------------------------------------------------
    void add_to_app()
    {
      m_app_base_runner->add_app_window(this);
      m_app_base_runner->create_thread();
    }
    // -------------------------------------------------------------------------
    // wait_app
    // -------------------------------------------------------------------------
    void wait_app()
    {
      m_app_base_runner->wait_thread();
    }

  private:
    GTK_BaseAppRunner *m_app_base_runner;
  };

  // ---------------------------------------------------------------------------
  //	GTK_BaseObject class
  // ---------------------------------------------------------------------------
  class GTK_BaseObject
  {
  public:
    // -------------------------------------------------------------------------
    // ~GTK_BaseObject
    // -------------------------------------------------------------------------
    virtual ~GTK_BaseObject()
    {
    }
    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // ShowWindow
    // -------------------------------------------------------------------------
    void ShowWindow()
    {
      if (m_window != nullptr)
        return;

      // The following is a sanity check
      // And also this call will create a runner instance (a bit tricky)
      if (GTK_BaseAppRunner::check_runner() == false)
        return;

      CreateWindow();
    }
    // -------------------------------------------------------------------------
    // WaitForWindowCloseAll
    // -------------------------------------------------------------------------
    void WaitForWindowCloseAll()
    {
      if (m_window == nullptr)
        return;
      m_window->wait_app();
    }

  protected:
    // -------------------------------------------------------------------------
    // GTK_BaseObject
    // -------------------------------------------------------------------------
    GTK_BaseObject() :
            m_window(nullptr)
    {
    }

    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // CreateWindow
    // -------------------------------------------------------------------------
    virtual GTK_BaseWindow *CreateWindow() = 0;

  private:
    GTK_BaseWindow *m_window;
  };

#else
  // If another shl file is already included, we need to check the base gtk class version
  #if SHL_BASE_GTK_CLASS_VERSION != SHL_IMAGE_WINDOW_GTK_BASE_VERSION
    #error invalid shl base class version (There is a version inconsistency between included shl files)
  #endif
  //
#endif  // SHL_BASE_GTK_CLASS_

  // ---------------------------------------------------------------------------
  //	GTK_ImageMainWindow class
  // ---------------------------------------------------------------------------
  class GTK_ImageMainWindow : public GTK_BaseWindow
  {
  public:
    // -------------------------------------------------------------------------
    // GTK_ImageMainWindow
    // -------------------------------------------------------------------------
    GTK_ImageMainWindow() :
            GTK_BaseWindow(),
            m_box(Gtk::Orientation::ORIENTATION_VERTICAL , 0),
            m_status_bar("Statusbar")
    {
    }
    // -------------------------------------------------------------------------
    // GTK_ImageMainWindow
    // -------------------------------------------------------------------------
    virtual ~GTK_ImageMainWindow()
    {
    }

    friend class ImageWindowGTK;
    Gtk::Box            m_box;
    Gtk::ScrolledWindow m_scr_win;
    Gtk::Label          m_status_bar;
    //ibc::gtkmm::ImageView   mImageView;
    //ibc::gtkmm::ImageData   *mImageDataPtr;
  };

  // ---------------------------------------------------------------------------
  //	ImageWindowGTK class
  // ---------------------------------------------------------------------------
  class ImageWindowGTK : public GTK_BaseObject
  {
  public:
    // -------------------------------------------------------------------------
    // ImageWindowGTK
    // -------------------------------------------------------------------------
    ImageWindowGTK()
    {
    }
    // -------------------------------------------------------------------------
    // ~ImageWindowGTK
    // -------------------------------------------------------------------------
    virtual ~ImageWindowGTK()
    {
    }

  protected:
    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // ShowWindow
    // -------------------------------------------------------------------------
    virtual GTK_BaseWindow *CreateWindow()
    {
      GTK_ImageMainWindow *window = new GTK_ImageMainWindow();
      if (window == nullptr)
        return nullptr;
      window->add_to_app();
      return window;
    }
  };
};

#endif	// #ifdef IMAGE_WINDOW_GTK_H_
