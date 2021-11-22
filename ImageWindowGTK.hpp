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

#include <cstdio>
#include <algorithm>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
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
  //	GTK_BaseWindowFactoryInterface class
  // ---------------------------------------------------------------------------
  class GTK_BaseWindowFactoryInterface
  {
  public:
    virtual Gtk::Window *create_window() = 0;
    virtual void wait_new_window() = 0;
    virtual Gtk::Window *get_window() = 0;
    virtual void delete_window() = 0;
  };

  // ---------------------------------------------------------------------------
  //	BaseAppGTK class
  // ---------------------------------------------------------------------------
  class GTK_BaseApp : public Gtk::Application
  {
  protected:
    // constructors and destructor ---------------------------------------------
    // -------------------------------------------------------------------------
    // GTK_BaseApp
    // -------------------------------------------------------------------------
    GTK_BaseApp() :
            Gtk::Application("org.gtkmm.examples.application",
                             Gio::APPLICATION_NON_UNIQUE),
                             m_quit(false)
    {
      Glib::signal_idle().connect( sigc::mem_fun(*this, &GTK_BaseApp::on_idle) );
    }

    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // create_window
    // -------------------------------------------------------------------------
    // [Note] this function will be called from another thread
    //
    void create_window(GTK_BaseWindowFactoryInterface *in_interface)
    {
      std::lock_guard<std::mutex> lock(m_create_win_queue_mutex);
      m_create_win_queue.push(in_interface);
    }
    // -------------------------------------------------------------------------
    // delete_window
    // -------------------------------------------------------------------------
    // [Note] this function will be called from another thread
    //
    void delete_window(GTK_BaseWindowFactoryInterface *in_interface)
    {
      std::lock_guard<std::mutex> lock(m_delete_win_queue_mutex);
      m_delete_win_queue.push(in_interface);
    }
    // -------------------------------------------------------------------------
    // post_quit_app
    // -------------------------------------------------------------------------
    // [Note] this function will be called from another thread
    //
    void post_quit_app()
    {
      std::lock_guard<std::mutex> lock(m_create_win_queue_mutex);
      m_quit = true;
    }
    // -------------------------------------------------------------------------
    // get_window_num
    // -------------------------------------------------------------------------
    //
    size_t get_window_num()
    {
      return m_window_list.size();
    }
    // -------------------------------------------------------------------------
    // wait_window_all_closed
    // -------------------------------------------------------------------------
    //
    void wait_window_all_closed()
    {
      std::unique_lock<std::mutex> window_lock(m_window_mutex);
      if (get_window_num() == 0)
        return;
      m_window_cond.wait(window_lock);
    }
    // -------------------------------------------------------------------------
    // -------------------------------------------------------------------------
    // on_activate
    // -------------------------------------------------------------------------
    void on_activate() override
    {
      // The application has been started, so let's show a window.
      process_create_windows();
    }
    // -------------------------------------------------------------------------
    // on_hide_window
    // -------------------------------------------------------------------------
    void on_hide_window(Gtk::Window *in_window)
    {
      for (auto it = m_window_list.begin(); it != m_window_list.end(); it++)
      {
        if (in_window == (*it)->get_window())
        {
          (*it)->delete_window();
          m_window_list.erase(it);
          break;
        }
      }
      if (m_window_list.empty())
        m_window_cond.notify_all();
    }
    // -------------------------------------------------------------------------
    // on_idle
    // -------------------------------------------------------------------------
    bool on_idle()
    {
      process_create_windows();
      process_delete_windows();
      if (m_quit)
        quit();
      return true;
    }
    // -------------------------------------------------------------------------
    // process_create_windows
    // -------------------------------------------------------------------------
    void process_create_windows()
    {
      std::lock_guard<std::mutex> lock(m_create_win_queue_mutex);
      while (!m_create_win_queue.empty())
      {
        GTK_BaseWindowFactoryInterface *interface = m_create_win_queue.front();
        auto it = std::find(m_window_list.begin(), m_window_list.end(), interface);
        if (it == m_window_list.end())
        {
          Gtk::Window *win = interface->create_window();
          add_window(*win);
          win->signal_hide().connect(sigc::bind<Gtk::Window *>(
                        sigc::mem_fun(*this,
                                &GTK_BaseApp::on_hide_window), win));
          win->present();
          m_window_list.push_back(interface);
        }
        m_create_win_queue.pop();
      }
    }
    // -------------------------------------------------------------------------
    // process_delete_windows
    // -------------------------------------------------------------------------
    void process_delete_windows()
    {
      std::lock_guard<std::mutex> lock(m_delete_win_queue_mutex);
      while (!m_delete_win_queue.empty())
      {
        GTK_BaseWindowFactoryInterface *interface = m_delete_win_queue.front();
        auto it = std::find(m_window_list.begin(), m_window_list.end(), interface);
        if (it != m_window_list.end())
        {
          Gtk::Window *win = interface->get_window();
          win->close();
          remove_window(*win);
          interface->delete_window();
          m_window_list.erase(it);
        }
        m_delete_win_queue.pop();
      }
    }

  private:
    // member variables --------------------------------------------------------
    std::mutex m_create_win_queue_mutex;
    std::queue<GTK_BaseWindowFactoryInterface *> m_create_win_queue;
    std::mutex m_delete_win_queue_mutex;
    std::queue<GTK_BaseWindowFactoryInterface *> m_delete_win_queue;
    std::vector<GTK_BaseWindowFactoryInterface *> m_window_list;
    std::condition_variable m_window_cond;
    std::mutex  m_window_mutex;
    bool m_quit;

    // friend classes ----------------------------------------------------------
    friend class GTK_BaseAppRunner;
  };

  // ---------------------------------------------------------------------------
  //	GTK_BaseAppRunner class
  // ---------------------------------------------------------------------------
  class GTK_BaseAppRunner
  {
  public:
    // constructors and destructor ---------------------------------------------
    // -------------------------------------------------------------------------
    // ~GTK_BaseAppRunner
    // -------------------------------------------------------------------------
    virtual ~GTK_BaseAppRunner()
    {
      if (m_app != nullptr)
        m_app->post_quit_app();
      if (m_thread != nullptr)
      {
        m_thread->join();
        delete m_thread;
        delete m_app;
      }
      printf("[DEBUG] GTK_BaseAppRunner was deleted\n");
    }

  protected:
    // constructors and destructor ---------------------------------------------
    // -------------------------------------------------------------------------
    // GTK_BaseAppRunner
    // -------------------------------------------------------------------------
    GTK_BaseAppRunner() :
      m_app(nullptr), m_thread(nullptr)
    {
    }

    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // wait_window_all_closed
    // -------------------------------------------------------------------------
    void wait_window_all_closed()
    {
      std::lock_guard<std::mutex> lock(m_function_call_mutex);
      if (m_app == nullptr)
        return;
      m_app->wait_window_all_closed();
    }
    // -------------------------------------------------------------------------
    // is_window_opened
    // -------------------------------------------------------------------------
    bool is_window_close_all()
    {
      std::lock_guard<std::mutex> lock(m_function_call_mutex);
      if (m_app == nullptr)
        return true;
      if (m_app->get_window_num() == 0)
        return true;
      return false;
    }
    // -------------------------------------------------------------------------
    // create_window
    // -------------------------------------------------------------------------
    void create_window(GTK_BaseWindowFactoryInterface *in_interface)
    {
      std::lock_guard<std::mutex> lock(m_function_call_mutex);
      if (m_app == nullptr)
      {
        m_app = new GTK_BaseApp();
        m_app->hold();
      }
      m_app->create_window(in_interface);
      if (m_thread != nullptr)
        return;
      m_thread = new std::thread(thread_func, this);
    }
    // -------------------------------------------------------------------------
    // delete_window
    // -------------------------------------------------------------------------
    void delete_window(GTK_BaseWindowFactoryInterface *in_interface)
    {
      std::lock_guard<std::mutex> lock(m_function_call_mutex);
      if (m_app == nullptr)
        return;
      m_app->delete_window(in_interface);
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

  private:
    // member variables --------------------------------------------------------
    GTK_BaseApp *m_app;
    std::thread *m_thread;
    std::mutex m_function_call_mutex;

    // static functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // thread_func
    // -------------------------------------------------------------------------
    static void thread_func(GTK_BaseAppRunner *in_obj)
    {
      in_obj->m_app->run();
    }

    // friend classes ----------------------------------------------------------
    friend class GTK_BaseObject;
    friend class ImageWindowGTK;
  };

  // ---------------------------------------------------------------------------
  //	GTK_BaseObject class
  // ---------------------------------------------------------------------------
  class GTK_BaseObject : protected GTK_BaseWindowFactoryInterface
  {
  public:
    // constructors and destructor ---------------------------------------------
    // -------------------------------------------------------------------------
    // ~GTK_BaseObject
    // -------------------------------------------------------------------------
    virtual ~GTK_BaseObject()
    {
      m_app_runner->delete_window(this);
    }
    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // wait_window_close_all
    // -------------------------------------------------------------------------
    virtual void wait_window_close_all()
    {
      m_app_runner->wait_window_all_closed();
    }
    // -------------------------------------------------------------------------
    // is_window_close_all
    // -------------------------------------------------------------------------
    virtual bool is_window_close_all()
    {
      return m_app_runner->is_window_close_all();
    }
    // -------------------------------------------------------------------------
    // show_window
    // -------------------------------------------------------------------------
    virtual void show_window()
    {
      if (get_window() != nullptr)
        return;
      m_app_runner->create_window(this);
      wait_new_window();
    }

  protected:
    // constructors and destructor ---------------------------------------------
    // -------------------------------------------------------------------------
    // GTK_BaseObject
    // -------------------------------------------------------------------------
    GTK_BaseObject()
    {
      m_app_runner = GTK_BaseAppRunner::get_runner();
    }

  private:
    GTK_BaseAppRunner *m_app_runner;
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
  class GTK_ImageMainWindow : public Gtk::Window
  {
  public:
    // constructors and destructor ---------------------------------------------
    // -------------------------------------------------------------------------
    // GTK_ImageMainWindow
    // -------------------------------------------------------------------------
    GTK_ImageMainWindow()
            //m_box(Gtk::Orientation::ORIENTATION_VERTICAL , 0),
            //m_status_bar("Statusbar")
    = default;
    // -------------------------------------------------------------------------
    // ~GTK_ImageMainWindow
    // -------------------------------------------------------------------------
    ~GTK_ImageMainWindow() override
    {
      printf("[DEBUG] GTK_ImageMainWindow was deleted\n");
    }

    friend class ImageWindowGTK;
    //Gtk::Box            m_box;
    //Gtk::ScrolledWindow m_scr_win;
    //Gtk::Label          m_status_bar;
    //ibc::gtkmm::ImageView   mImageView;
    //ibc::gtkmm::ImageData   *mImageDataPtr;
  };

  // ---------------------------------------------------------------------------
  //	ImageWindowGTK class
  // ---------------------------------------------------------------------------
  class ImageWindowGTK : public GTK_BaseObject
  {
  public:
    // constructors and destructor ---------------------------------------------
    // -------------------------------------------------------------------------
    // ImageWindowGTK
    // -------------------------------------------------------------------------
    ImageWindowGTK() :
            m_window(nullptr)
    {
    }
    // -------------------------------------------------------------------------
    // ~ImageWindowGTK
    // -------------------------------------------------------------------------
    ~ImageWindowGTK() override
    {
      printf("[DEBUG] ImageWindowGTK was deleted\n");
    }
    // Member functions --------------------------------------------------------
    bool is_window_closed()
    {
      if (m_window == nullptr)
        return true;
      return false;
    }

  protected:
    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // create_window
    // -------------------------------------------------------------------------
    Gtk::Window *create_window() override
    {
      m_window = new GTK_ImageMainWindow();
      m_window_cond.notify_all();
      return m_window;
    }
    void wait_new_window() override
    {
      std::unique_lock<std::mutex> lock(m_window_mutex);
      m_window_cond.wait(lock);
    }
    Gtk::Window *get_window() override
    {
      return m_window;
    }
    void delete_window() override
    {
      delete m_window;
      m_window = nullptr;
    }

  private:
    GTK_ImageMainWindow *m_window;
    std::condition_variable m_window_cond;
    std::mutex  m_window_mutex;
  };
}

#endif	// #ifdef IMAGE_WINDOW_GTK_H_
