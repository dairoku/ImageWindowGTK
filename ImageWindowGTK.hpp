// =============================================================================
//  ImageWindow.hpp
//
//  MIT License
//
//  Copyright (c) 2021-2024 Dairoku Sekiguchi
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
  \file     ImageWindow.h
  \author   Dairoku Sekiguchi
  \version  3.0.0
  \date     2024/08/25
*/
#ifndef IMAGE_WINDOW_GTK_H_
#define IMAGE_WINDOW_GTK_H_
#define SHL_IMAGE_WINDOW_GTK_BASE_VERSION   "3.0.0"

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <gtkmm.h>
#include <gtkmm/switch.h>


// Namespace -------------------------------------------------------------------
namespace shl::gtk
{
//
// =============================================================================
//  shl base gtk classes
// =============================================================================
// Macros ----------------------------------------------------------------------
#ifndef SHL_BASE_GTK_CLASS_
#define SHL_BASE_GTK_CLASS_
#define SHL_BASE_GTK_CLASS_VERSION  SHL_CONTROLS_WINDOW_GTK_BASE_VERSION
//
// Namespace -------------------------------------------------------------------
namespace base  // shl::gtk::base
{
// Log macros for debugging ----------------------------------------------------
#define SHL_LOG_STRINGIFY(x)    #x
#define SHL_LOG_TOSTRING(x)     SHL_LOG_STRINGIFY(x)
#define SHL_LOG_AT              SHL_LOG_TOSTRING(__FILE__) ":" SHL_LOG_TOSTRING(__LINE__)
#define SHL_LOG_LOCATION_MACRO  "(" SHL_LOG_AT ")"
#ifdef _MSC_VER
 #define __PRETTY_FUNCTION__  __FUNCTION__
#endif
#define SHL_FUNC_NAME_MACRO  __PRETTY_FUNCTION__
#if defined(SHL_LOG_LEVEL_ERR)
 #define SHL_LOG_LEVEL 1
#elif defined(SHL_LOG_LEVEL_WARN)
 #define SHL_LOG_LEVEL 2
#elif defined(SHL_LOG_LEVEL_INFO)
 #define SHL_LOG_LEVEL 3
#elif defined(SHL_LOG_LEVEL_DBG)
 #define SHL_LOG_LEVEL 4
#elif defined(SHL_LOG_LEVEL_TRACE)
 #define SHL_LOG_LEVEL 5
#endif
#ifdef SHL_LOG_LEVEL
 #define SHL_LOG_OUT(type, loc_str, func_str, out_str, ...) \
  printf(type " %s@" loc_str  " " out_str "\n", func_str , ##__VA_ARGS__)
 #if SHL_LOG_LEVEL > 0
  #define SHL_ERROR_OUT(out_str, ...) SHL_LOG_OUT("[ERROR]",\
       SHL_LOG_LOCATION_MACRO, SHL_FUNC_NAME_MACRO, out_str, ##__VA_ARGS__)
 #else
  #define SHL_ERROR_OUT(out_str, ...)
 #endif
 #if SHL_LOG_LEVEL > 1
  #define SHL_WARNING_OUT(out_str, ...) SHL_LOG_OUT("[WARN ]",\
       SHL_LOG_LOCATION_MACRO, SHL_FUNC_NAME_MACRO, out_str, ##__VA_ARGS__)
 #else
  #define SHL_WARNING_OUT(out_str, ...)
 #endif
 #if SHL_LOG_LEVEL > 2
  #define SHL_INFO_OUT(out_str, ...) SHL_LOG_OUT("[INFO ]",\
       SHL_LOG_LOCATION_MACRO, SHL_FUNC_NAME_MACRO, out_str, ##__VA_ARGS__)
 #else
  #define SHL_INFO_OUT(out_str, ...)
 #endif
 #if SHL_LOG_LEVEL > 3
  #define SHL_DBG_OUT(out_str, ...) SHL_LOG_OUT("[DEBUG]",\
   SHL_LOG_LOCATION_MACRO, SHL_FUNC_NAME_MACRO, out_str, ##__VA_ARGS__)
 #else
  #define SHL_DBG_OUT(out_str, ...)
 #endif
 #if SHL_LOG_LEVEL > 4
  #define SHL_TRACE_OUT(out_str, ...) SHL_LOG_OUT("[TRACE]",\
   SHL_LOG_LOCATION_MACRO, SHL_FUNC_NAME_MACRO, out_str, ##__VA_ARGS__)
 #else
  #define SHL_TRACE_OUT(out_str, ...)
 #endif
#else
 #define SHL_LOG_OUT(type, loc_str, func_str, out_str, ...)
 #define SHL_ERROR_OUT(out_str, ...)
 #define SHL_WARNING_OUT(out_str, ...)
 #define SHL_INFO_OUT(out_str, ...)
 #define SHL_DBG_OUT(out_str, ...)
 #define SHL_TRACE_OUT(out_str, ...)
#endif

  // ===========================================================================
  //  BackgroundAppWindowInterface class
  // ===========================================================================
  class BackgroundAppWindowInterface
  {
  public:
    virtual Gtk::Window *back_app_create_window(const char *in_title) = 0;
    virtual void back_app_wait_new_window() = 0;
    virtual Gtk::Window *back_app_get_window() = 0;
    virtual bool back_app_delete_request() = 0;
    virtual void back_app_delete_window() = 0;
    virtual bool back_app_is_window_deleted() = 0;
    virtual void back_app_wait_delete_window() = 0;
    virtual void back_app_update_window() = 0;
  };

  // ===========================================================================
  //  EventData class
  // ===========================================================================
  class EventData
  {
  public:
    // -------------------------------------------------------------------------
    // EventData destructor
    // -------------------------------------------------------------------------
    virtual ~EventData() = default;
    // -------------------------------------------------------------------------
    // get_source
    // -------------------------------------------------------------------------
    virtual void *get_source()
    {
      return m_source;
    }

  protected:
    // -------------------------------------------------------------------------
    // EventData constructor
    // -------------------------------------------------------------------------
    EventData(void *in_source, void (*in_handler)(EventData *)) :
        m_source(in_source),
        m_handler(in_handler)
    {
    }
    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // is_same_source
    // -------------------------------------------------------------------------
    virtual bool is_same_source(EventData *in_event_data)
    {
      if (m_source == in_event_data->m_source &&
          m_handler       == in_event_data->m_handler)
        return true;
      return false;
    }
    // -------------------------------------------------------------------------
    // invoke_handler
    // -------------------------------------------------------------------------
    virtual void invoke_handler()
    {
      if (m_handler != nullptr)
        m_handler(this);
    }

  private:
    // member variables --------------------------------------------------------
    void  *m_source;
    void (*m_handler)(EventData *);

    // friend classes ----------------------------------------------------------
    friend class EventQueue;
    friend class WindowBase;
  };

  // ===========================================================================
  //  EventQueue class
  // ===========================================================================
  class EventQueue
  {
  public:
    // -------------------------------------------------------------------------
    // EventQueue constructor
    // -------------------------------------------------------------------------
    EventQueue() = default;
    // -------------------------------------------------------------------------
    // EventQueue destructor
    // -------------------------------------------------------------------------
    virtual ~EventQueue() = default;
    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // push
    // -------------------------------------------------------------------------
    void push(void *in_source, void (*in_handler)(EventData *))
    {
      auto *event = new EventData(in_source, in_handler);
      push(event);
    }
    // -------------------------------------------------------------------------
    // push
    // -------------------------------------------------------------------------
    void push(EventData *in_event)
    {
      std::lock_guard<std::mutex> lock(m_event_queue_mutex);
      m_event_data_queue.push_back(in_event);
      m_new_event_cond.notify_all();
    }
    // -------------------------------------------------------------------------
    // notify
    // -------------------------------------------------------------------------
    void notify()
    {
      std::lock_guard<std::mutex> lock(m_event_queue_mutex);
      m_new_event_cond.notify_all();
    }
    // -------------------------------------------------------------------------
    // wait
    // -------------------------------------------------------------------------
    void wait()
    {
      std::unique_lock<std::mutex> lock(m_event_queue_mutex);
      if (m_event_data_queue.empty() == false)
        return;
      m_new_event_cond.wait(lock);
    }
    // -------------------------------------------------------------------------
    // process_events
    // -------------------------------------------------------------------------
    void process_events(bool in_last_only = false)
    {
      std::lock_guard<std::mutex> lock(m_event_queue_mutex);
      while (m_event_data_queue.empty() == false)
      {
        bool skip = false;
        EventData *event_data = m_event_data_queue.front();
        if (in_last_only)
        {
          auto it = m_event_data_queue.begin();
          it++;
          while (it != m_event_data_queue.end())
          {
            if (event_data->is_same_source(*it))
            {
              skip = true;
              break;
            }
            it++;
          }
        }
        if (skip == false)
          event_data->invoke_handler();
        m_event_data_queue.pop_front();
        delete event_data;
      }
    }

  private:
    // member variables --------------------------------------------------------
    std::list<EventData *>  m_event_data_queue;
    std::condition_variable m_new_event_cond;
    std::mutex  m_event_queue_mutex;
  };

  // ===========================================================================
  //  TimerData class
  // ===========================================================================
  class TimerData
  {
  protected:
    // -------------------------------------------------------------------------
    // TimerData constructor
    // -------------------------------------------------------------------------
    TimerData( unsigned int in_interval_ms,
                EventQueue *in_user_event_queue,
                void (*in_timer_event_func)(void *in_user_data),
                void *in_user_data = nullptr) :
        m_interval_ms(in_interval_ms),
        m_user_event_queue(in_user_event_queue),
        m_timer_event_func(in_timer_event_func),
        m_user_data(in_user_data),
        m_is_running(false)
    {
    }
    // -------------------------------------------------------------------------
    // WidgetData destructor
    // -------------------------------------------------------------------------
    virtual ~TimerData() = default;
    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // connect (needs to be called from the UI thread)
    // -------------------------------------------------------------------------
    void connect()
    {
      mTimerConnection = Glib::signal_timeout().connect(
              sigc::mem_fun(*this, &TimerData::queue_timer_event), m_interval_ms);
      m_is_running = true;
      // todo : add connect_seconds() path also
    }
    // -------------------------------------------------------------------------
    // disconnect (needs to be called from the UI thread)
    // -------------------------------------------------------------------------
    void disconnect()
    {
      if (m_is_running == false)
        return;
      mTimerConnection.disconnect();
      m_is_running = false;
    }
    // -------------------------------------------------------------------------
    // queue_timer_event
    // -------------------------------------------------------------------------
    bool queue_timer_event()
    {
      if (m_user_event_queue == nullptr || m_timer_event_func == nullptr)
        return false; // return false = disconnect

      m_user_event_queue->push(this, process_timer_event);
      return true;  
    }
    // -------------------------------------------------------------------------
    // process_timer_event
    // -------------------------------------------------------------------------
    static void process_timer_event(base::EventData *in_event)
    {
      auto  *timer = (TimerData *)in_event->get_source();
      timer->m_timer_event_func(timer->m_user_data);
    }

  private:
    // member variables --------------------------------------------------------
    unsigned int  m_interval_ms;
    void (*m_timer_event_func)(void *in_user_data);
    void *m_user_data;
    EventQueue *m_user_event_queue;
    sigc::connection  mTimerConnection;
    bool  m_is_running;

    friend class BackgroundApp;
    friend class WindowBase;
  };

  // ===========================================================================
  //  BackgroundApp class
  // ===========================================================================
  class BackgroundApp : public Gtk::Application
  {
  public:
    // -------------------------------------------------------------------------
    // BackgroundApp destructor
    // -------------------------------------------------------------------------
    ~BackgroundApp() override
    {
      SHL_DBG_OUT("BackgroundApp was deleted");
    }

  protected:
    // -------------------------------------------------------------------------
    // BackgroundApp constructor
    // -------------------------------------------------------------------------
    BackgroundApp() :
            Gtk::Application("org.gtkmm.examples.application",
                             Gio::APPLICATION_NON_UNIQUE),
                             m_quit(false)
    {
    }

    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // post_create_window
    // -------------------------------------------------------------------------
    // [Note] this function will be called from another thread
    //
    void post_create_window(BackgroundAppWindowInterface *in_interface, const char *in_title)
    {
      std::lock_guard<std::mutex> lock(m_create_win_queue_mutex);
      m_create_win_queue.push(in_interface);
      m_win_title_queue.push(in_title);
      // Invoke one time on_idle call (by returning false from the signal handler)
      Glib::signal_idle().connect(sigc::mem_fun(*this, &BackgroundApp::on_idle));
    }
    // -------------------------------------------------------------------------
    // post_delete_window
    // -------------------------------------------------------------------------
    // [Note] this function will be called from another thread
    //
    void post_delete_window(BackgroundAppWindowInterface *in_interface)
    {
      std::lock_guard<std::mutex> lock(m_delete_win_queue_mutex);
      m_delete_win_queue.push(in_interface);
      // Invoke one time on_idle call (by returning false from the signal handler)
      Glib::signal_idle().connect(sigc::mem_fun(*this, &BackgroundApp::on_idle));
    }
    // -------------------------------------------------------------------------
    // post_update_window
    // -------------------------------------------------------------------------
    // [Note] this function will be called from another thread
    //
    void post_update_window(BackgroundAppWindowInterface *in_interface)
    {
      std::lock_guard<std::mutex> lock(m_update_win_queue_mutex);
      m_update_win_queue.push(in_interface);
      // Invoke one time on_idle call (by returning false from the signal handler)
      Glib::signal_idle().connect(sigc::mem_fun(*this, &BackgroundApp::on_idle));
    }
    // -------------------------------------------------------------------------
    // post_connect_timer
    // -------------------------------------------------------------------------
    // [Note] this function will be called from another thread
    //
    void post_connect_timer(TimerData *inTimerData)
    {
      std::lock_guard<std::mutex> lock(m_connect_timer_queue_mutex);
      m_connect_timer_queue.push(inTimerData);
      // Invoke one time on_idle call (by returning false from the signal handler)
      Glib::signal_idle().connect(sigc::mem_fun(*this, &BackgroundApp::on_idle));
    }
    // -------------------------------------------------------------------------
    // post_disconnect_timer
    // -------------------------------------------------------------------------
    // [Note] this function will be called from another thread
    //
    void post_disconnect_timer(TimerData *inTimerData)
    {
      std::lock_guard<std::mutex> lock(m_disconnect_timer_queue_mutex);
      m_disconnect_timer_queue.push(inTimerData);
      // Invoke one time on_idle call (by returning false from the signal handler)
      Glib::signal_idle().connect(sigc::mem_fun(*this, &BackgroundApp::on_idle));
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
      // Invoke one time on_idle call (by returning false from the signal handler)
      Glib::signal_idle().connect(sigc::mem_fun(*this, &BackgroundApp::on_idle));
    }
    // -------------------------------------------------------------------------
    // get_window_num
    // -------------------------------------------------------------------------
    size_t get_window_num()
    {
      return m_window_list.size();
    }
    // -------------------------------------------------------------------------
    // wait_window_all_closed
    // -------------------------------------------------------------------------
    void wait_window_all_closed()
    {
      std::unique_lock<std::mutex> window_lock(m_window_mutex);
      if (get_window_num() == 0)
        return;
      m_window_cond.wait(window_lock);
    }
    // -------------------------------------------------------------------------
    // on_activate
    // -------------------------------------------------------------------------
    void on_activate() override
    {
      // The application has been started, so let's show a window.
      process_create_windows();
    }
    // -------------------------------------------------------------------------
    // on_delete_event
    // -------------------------------------------------------------------------
    bool on_delete_event(GdkEventAny *in_event)
    {
      for (auto it = m_window_list.begin(); it != m_window_list.end(); it++)
      {
        //if (in_window == (*it)->back_app_get_window())
        if (in_event->window == (*it)->back_app_get_window()->get_window()->gobj())
          return (*it)->back_app_delete_request();
      }
      return false;
    }
    // -------------------------------------------------------------------------
    // on_hide_window
    // -------------------------------------------------------------------------
    void on_hide_window(Gtk::Window *in_window)
    {
      for (auto it = m_window_list.begin(); it != m_window_list.end(); it++)
      {
        if (in_window == (*it)->back_app_get_window())
        {
          (*it)->back_app_delete_window();
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
      SHL_DBG_OUT("on_idle() was called");
      process_create_windows();
      process_update_windows();
      process_delete_windows();
      process_connect_timers();
      process_disconnect_timers();
      if (m_quit)
        quit();
      // by returning false here, this signal handler will be disconnected
      // from Glib::signal_idle()
      // https://gnome.pages.gitlab.gnome.org/gtkmm-documentation/sec-idle-functions.html
      return false;
    }
    // -------------------------------------------------------------------------
    // process_create_windows
    // -------------------------------------------------------------------------
    void process_create_windows()
    {
      std::lock_guard<std::mutex> lock(m_create_win_queue_mutex);
      while (!m_create_win_queue.empty())
      {
        BackgroundAppWindowInterface *interface = m_create_win_queue.front();
        const char *title = m_win_title_queue.front();
        auto it = std::find(m_window_list.begin(), m_window_list.end(), interface);
        if (it == m_window_list.end())
        {
          Gtk::Window *win = interface->back_app_create_window(title);
          add_window(*win);
          win->signal_delete_event().connect(sigc::mem_fun(*this,
                            &BackgroundApp::on_delete_event));
          win->signal_hide().connect(sigc::bind<Gtk::Window *>(
                  sigc::mem_fun(*this,
                                &BackgroundApp::on_hide_window), win));
          win->present();
          m_window_list.push_back(interface);
        }
        m_create_win_queue.pop();
        m_win_title_queue.pop();
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
        BackgroundAppWindowInterface *interface = m_delete_win_queue.front();
        auto it = std::find(m_window_list.begin(), m_window_list.end(), interface);
        if (it != m_window_list.end())
        {
          Gtk::Window *win = interface->back_app_get_window();
          win->close();
          remove_window(*win);
          interface->back_app_delete_window();
          m_window_list.erase(it);
        }
        m_delete_win_queue.pop();
      }
    }
    // -------------------------------------------------------------------------
    // process_update_windows
    // -------------------------------------------------------------------------
    void process_update_windows()
    {
      std::lock_guard<std::mutex> lock(m_update_win_queue_mutex);
      while (!m_update_win_queue.empty())
      {
        BackgroundAppWindowInterface *interface = m_update_win_queue.front();
        auto it = std::find(m_window_list.begin(), m_window_list.end(), interface);
        if (it != m_window_list.end())
          interface->back_app_update_window();
        m_update_win_queue.pop();
      }
    }
    // -------------------------------------------------------------------------
    // process_connect_timers
    // -------------------------------------------------------------------------
    void process_connect_timers()
    {
      std::lock_guard<std::mutex> lock(m_connect_timer_queue_mutex);
      while (!m_connect_timer_queue.empty())
      {
        TimerData *timer = m_connect_timer_queue.front();
        timer->connect();
        m_connect_timer_queue.pop();
      }
    }
    // -------------------------------------------------------------------------
    // process_disconnect_timers
    // -------------------------------------------------------------------------
    void process_disconnect_timers()
    {
      std::lock_guard<std::mutex> lock(m_disconnect_timer_queue_mutex);
      while (!m_disconnect_timer_queue.empty())
      {
        TimerData *timer = m_disconnect_timer_queue.front();
        timer->disconnect();
        m_disconnect_timer_queue.pop();
      }
    }

  private:
    // member variables --------------------------------------------------------
    std::mutex m_create_win_queue_mutex;
    std::queue<BackgroundAppWindowInterface *> m_create_win_queue;
    std::queue<const char *> m_win_title_queue;
    std::mutex m_delete_win_queue_mutex;
    std::queue<BackgroundAppWindowInterface *> m_delete_win_queue;
    std::mutex m_update_win_queue_mutex;
    std::queue<BackgroundAppWindowInterface *> m_update_win_queue;
    //
    std::mutex m_connect_timer_queue_mutex;
    std::queue<TimerData *> m_connect_timer_queue;
    std::mutex m_disconnect_timer_queue_mutex;
    std::queue<TimerData *> m_disconnect_timer_queue;
    //
    std::vector<BackgroundAppWindowInterface *> m_window_list;
    std::condition_variable m_window_cond;
    std::mutex  m_window_mutex;
    bool m_quit;

    // friend classes ----------------------------------------------------------
    friend class BackgroundAppRunner;
  };

  // ===========================================================================
  //  BackgroundAppRunner class
  // ===========================================================================
  class BackgroundAppRunner
  {
  public:
    // -------------------------------------------------------------------------
    // BackgroundAppRunner destructor
    // -------------------------------------------------------------------------
    virtual ~BackgroundAppRunner()
    {
      if (m_app != nullptr)
        m_app->post_quit_app();
      if (m_thread != nullptr)
      {
        m_thread->join();
        delete m_thread;
        delete m_app;
      }
      SHL_DBG_OUT("BackgroundAppRunner was deleted");
    }

  protected:
    // -------------------------------------------------------------------------
    // BackgroundAppRunner constructor
    // -------------------------------------------------------------------------
    BackgroundAppRunner() :
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
    // get_window_num
    // -------------------------------------------------------------------------
    size_t get_window_num()
    {
      std::lock_guard<std::mutex> lock(m_function_call_mutex);
      if (m_app == nullptr)
        return 0;
      return m_app->get_window_num();
    }
    // -------------------------------------------------------------------------
    // is_window_close_all
    // -------------------------------------------------------------------------
    bool is_window_close_all()
    {
      if (get_window_num() == 0)
        return true;
      return false;
    }
    // -------------------------------------------------------------------------
    // create_window
    // -------------------------------------------------------------------------
    void create_window(BackgroundAppWindowInterface *in_interface, const char *in_title)
    {
      std::lock_guard<std::mutex> lock(m_function_call_mutex);
      if (m_app == nullptr)
      {
        m_app = new BackgroundApp();
        m_app->hold();
      }
      m_app->post_create_window(in_interface, in_title);
      if (m_thread != nullptr)
        return;
      m_thread = new std::thread(thread_func, this);
    }
    // -------------------------------------------------------------------------
    // delete_window
    // -------------------------------------------------------------------------
    void delete_window(BackgroundAppWindowInterface *in_interface)
    {
      std::lock_guard<std::mutex> lock(m_function_call_mutex);
      if (m_app == nullptr)
        return;
      m_app->post_delete_window(in_interface);
    }
    // -------------------------------------------------------------------------
    // update_window
    // -------------------------------------------------------------------------
    void update_window(BackgroundAppWindowInterface *in_interface)
    {
      std::lock_guard<std::mutex> lock(m_function_call_mutex);
      if (m_app == nullptr)
        return;
      m_app->post_update_window(in_interface);
    }
    // -------------------------------------------------------------------------
    // connect_timer
    // -------------------------------------------------------------------------
    void connect_timer(TimerData *inTimerData)
    {
      std::lock_guard<std::mutex> lock(m_function_call_mutex);
      if (m_app == nullptr)
        return;
      m_app->post_connect_timer(inTimerData);
    }
    // -------------------------------------------------------------------------
    // disconnect_timer
    // -------------------------------------------------------------------------
    void disconnect_timer(TimerData *inTimerData)
    {
      std::lock_guard<std::mutex> lock(m_function_call_mutex);
      if (m_app == nullptr)
        return;
      m_app->post_disconnect_timer(inTimerData);
    }

    // static functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // get_runner
    // -------------------------------------------------------------------------
    static BackgroundAppRunner *get_runner()
    {
      static BackgroundAppRunner s_runner;
      return &s_runner;
    }

  private:
    // member variables --------------------------------------------------------
    BackgroundApp *m_app;
    std::thread *m_thread;
    std::mutex m_function_call_mutex;

    // static functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // thread_func
    // -------------------------------------------------------------------------
    static void thread_func(BackgroundAppRunner *in_obj)
    {
      SHL_TRACE_OUT("thread started");
      in_obj->m_app->run();
      SHL_TRACE_OUT("thread ended");
    }

    // friend classes ----------------------------------------------------------
    friend class WindowBase;
  };

  // ===========================================================================
  //  WindowBase class
  // ===========================================================================
  class WindowBase : private BackgroundAppWindowInterface
  {
  public:
    // -------------------------------------------------------------------------
    // WindowBase destructor
    // -------------------------------------------------------------------------
    virtual ~WindowBase()
    {
      kill_timer(nullptr);  // stop and delete all timers (just in case)
      m_app_runner->delete_window(this);
    }

    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // wait_window_closed
    // -------------------------------------------------------------------------
    /**
     * Waits until the window associated to the object is closed.
     * @note This is a blocking function.
     *
     */
    void wait_window_closed()
    {
      back_app_wait_delete_window();
    }
    // -------------------------------------------------------------------------
    // is_window_closed
    // -------------------------------------------------------------------------
    /**
     * Checks whether the opened window is closed or not.
     * @ The window associated to this object
     *
     * @return Whether the window is closed or not
     *  - true : The window was closed
     *  - false : The window is not closed
     */
    bool is_window_closed()
    {
      return back_app_is_window_deleted();
    }
    // -------------------------------------------------------------------------
    // wait_window_close_all
    // -------------------------------------------------------------------------
    /**
     * Waits until all of the windows are closed.
     * @note The windows including opened by the other objects
     */
    void wait_window_close_all()
    {
      m_app_runner->wait_window_all_closed();
    }
    // -------------------------------------------------------------------------
    // is_window_close_all
    // -------------------------------------------------------------------------
    /**
     * Checks whether the all windows are closed or not.
     * @note The windows including opened by the other objects
     *
     * @return Whether the window is closed or not
     *  - true : The windows were all closed
     *  - false : The all of the windows are still not closed
     */
    bool is_window_close_all()
    {
      return m_app_runner->is_window_close_all();
    }
    // -------------------------------------------------------------------------
    // get_window_num
    // -------------------------------------------------------------------------
    /**
     * Retrieves the number of opened windows.
     * @note The windows including opened by the other objects
     *
     * @return the number of opened windows
     */
    size_t get_window_num()
    {
      return m_app_runner->get_window_num();
    }
    // -------------------------------------------------------------------------
    // show_window
    // -------------------------------------------------------------------------
    /**
     * Shows a window.
     * @note if in_title is nullptr, the title will be automatically generated.
     *
     * @param in_title  The title of the window
     */
    void show_window(const char *in_title = nullptr)
    {
      if (back_app_get_window() != nullptr)
        return;
      m_app_runner->create_window(this, in_title);
      back_app_wait_new_window();
      start_all_timers();
    }
    // -------------------------------------------------------------------------
    // update
    // -------------------------------------------------------------------------
    /**
     * Tells the library to updates the window.
     * @note This function needs to be called when the content of the object is
     * updated
     */
    void update()
    {
      if (back_app_get_window() == nullptr)
        return;
      m_app_runner->update_window(this);
    }
    // -------------------------------------------------------------------------
    // get_user_event_queue()
    // -------------------------------------------------------------------------
    EventQueue *get_user_event_queue()
    {
      if (m_user_event_queue != nullptr)
        return m_user_event_queue;
      return get_user_global_queue();
    }
    // -------------------------------------------------------------------------
    // push_update_event()
    // -------------------------------------------------------------------------
    void push_update_event(void *inSource, void (*in_func)(EventData *))
    {
      m_background_queue.push(inSource, in_func);
    }
    // -------------------------------------------------------------------------
    // push_update_event()
    // -------------------------------------------------------------------------
    void push_update_event(EventData *in_update)
    {
      m_background_queue.push(in_update);
    }
    // -------------------------------------------------------------------------
    // add_close_event_listener
    // -------------------------------------------------------------------------
    void add_close_event_listener(base::EventQueue *in_event_queue)
    {
      m_close_notify_list.push_back(in_event_queue);
    }
    // -------------------------------------------------------------------------
    // connect_timer
    // -------------------------------------------------------------------------
    TimerData *add_timer(
                  unsigned int in_interval_ms,
                  void (*in_timer_event_func)(void *in_user_data) = nullptr,
                  void *in_user_data = nullptr,
                  base::EventQueue *in_user_event_queue = nullptr)
    {
      if (in_user_event_queue == nullptr)
        in_user_event_queue = get_user_event_queue();
      TimerData *timer;
      timer = new TimerData(
              in_interval_ms,
              in_user_event_queue,
              in_timer_event_func,
              in_user_data);
      m_timer_list.push_back(timer);
      return timer;
    }
    // -------------------------------------------------------------------------
    // kill_timer
    // -------------------------------------------------------------------------
    // [note]
    // after calling this function, passed inTimeData will be no
    // longer valid (the linked object will be deleted internally)
    // specifying nullptr will remove all timers associated with this window
    //
    void kill_timer(TimerData *inTimerData = nullptr)
    {
      if (inTimerData == nullptr)
        return;
      
      for (auto it = m_timer_list.begin(); it != m_timer_list.end(); it++)
        if ((*it) == inTimerData || inTimerData == nullptr)
        {
          m_app_runner->disconnect_timer((*it));
          delete (*it);
          if (inTimerData != nullptr)
          {
            m_timer_list.erase(it);
            break;
          }
        }
      if (inTimerData == nullptr)
        m_timer_list.clear();
    }

  protected:
    // Virtual Member functions --------------------------------------------------------
    virtual Gtk::Window *create_window_object(const char *in_title) = 0;
    virtual Gtk::Window *get_window_object() = 0;
    virtual void delete_window_object() = 0;
    virtual bool is_window_object_null() = 0;
    virtual void update_window() = 0;
    virtual const char *get_default_window_title() = 0;

    // -------------------------------------------------------------------------
    // WindowBase constructor
    // -------------------------------------------------------------------------
    WindowBase(EventQueue *in_user_event_queue = nullptr) :
      m_user_event_queue(in_user_event_queue)
    {
      m_app_runner = BackgroundAppRunner::get_runner();
    }
    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // process_update_events()
    // -------------------------------------------------------------------------
    void process_update_events(bool in_last_only = false)
    {
      m_background_queue.process_events(in_last_only);
    }
    // -------------------------------------------------------------------------
    // start_all_timers
    // -------------------------------------------------------------------------
    void start_all_timers()
    {
      for (auto it = m_timer_list.begin(); it != m_timer_list.end(); it++)
        m_app_runner->connect_timer((*it));
    }

  private:
    // member variables --------------------------------------------------------
    BackgroundAppRunner *m_app_runner;
    EventQueue  m_background_queue;
    EventQueue  *m_user_event_queue;
    std::condition_variable m_new_window_cond;
    std::mutex  m_new_window_mutex;
    std::condition_variable m_delete_window_cond;
    std::mutex  m_delete_window_mutex;
    std::vector<base::EventQueue *> m_close_notify_list;
    std::vector<TimerData *>   m_timer_list;

    // BackgroundAppWindowInterface functions ----------------------------------
    // -------------------------------------------------------------------------
    // back_app_create_window (called from the UI thread)
    // -------------------------------------------------------------------------
    Gtk::Window *back_app_create_window(const char *in_title) override
    {
      static int window_num = 0;
      char buf[256];
      if (in_title != nullptr)
        if (*in_title == 0)
          in_title = nullptr;
      if (in_title == nullptr)
      {
        const char *title = get_default_window_title();
        if (window_num == 0)
          sprintf(buf, "%s", title);
        else
          sprintf(buf, "%s_%d", title, window_num);
        in_title = buf;
      }
      Gtk::Window *window = create_window_object(in_title);
      m_new_window_cond.notify_all();
      window_num++;
      return window;
    }
    // -------------------------------------------------------------------------
    // back_app_wait_new_window (called from the UI thread)
    // -------------------------------------------------------------------------
    void back_app_wait_new_window() override
    {
      std::unique_lock<std::mutex> lock(m_new_window_mutex);
      m_new_window_cond.wait(lock);
    }
    // -------------------------------------------------------------------------
    // back_app_delete_request (called from the UI thread)
    // -------------------------------------------------------------------------
    Gtk::Window *back_app_get_window() override
    {
      return get_window_object();
    }
    // -------------------------------------------------------------------------
    // back_app_delete_request (called from the UI thread)
    // -------------------------------------------------------------------------
    bool back_app_delete_request() override
    {
      return false; // by returning true, we can cancel window close
    }
    // -------------------------------------------------------------------------
    // back_app_delete_window (called from the UI thread)
    // -------------------------------------------------------------------------
    void back_app_delete_window() override
    {
      // we need to disconnect all timers first
      for (auto it = m_timer_list.begin(); it != m_timer_list.end(); it++)
        (*it)->disconnect();
      delete_window_object();
      m_delete_window_cond.notify_all();
      // We need to notify all event queues to un-block event queue's wait()
      for (auto it = m_close_notify_list.begin(); it != m_close_notify_list.end(); it++)
        (*it)->notify();
    }
    // -------------------------------------------------------------------------
    // back_app_is_window_deleted (called from the UI thread)
    // -------------------------------------------------------------------------
    bool back_app_is_window_deleted() override
    {
      return is_window_object_null();
    }
    // -------------------------------------------------------------------------
    // back_app_wait_delete_window (called from the UI thread)
    // -------------------------------------------------------------------------
    void back_app_wait_delete_window() override
    {
      std::unique_lock<std::mutex> lock(m_delete_window_mutex);
      m_delete_window_cond.wait(lock);
    }
    // -------------------------------------------------------------------------
    // back_app_update_window
    // -------------------------------------------------------------------------
    void back_app_update_window() override
    {
      update_window();
    }
    // static functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // get_user_global_queue
    // -------------------------------------------------------------------------
    static EventQueue *get_user_global_queue()
    {
      static EventQueue s_user_global_queue;
      return &s_user_global_queue;
    }
  };
} // namespace shl::gtk::base
#else
  // If another shl file is already included, we need to is_valid the base gtk class version
  #if SHL_BASE_GTK_CLASS_VERSION != SHL_CONTROLS_WINDOW_GTK_BASE_VERSION
    #error invalid shl base class version (There is a version inconsistency between included shl files)
  #endif
  //
#endif  // SHL_BASE_GTK_CLASS_

// Namespace -------------------------------------------------------------------
class ImageWindow;  // This is for the friend class definition

namespace image     // shl::gtk::image
{
  // ===========================================================================
  // Colormap class - colormap related utility class
  // ===========================================================================
  class Colormap
  {
  public:
    // Constants ---------------------------------------------------------------
    enum ColormapIndex
    {
      COLORMAP_NOT_SPECIFIED = 0,

      // Linear
      COLORMAP_GrayScale = 1,
      COLORMAP_Jet,
      COLORMAP_Rainbow,
      COLORMAP_RainbowWide,
      COLORMAP_Spectrum,
      COLORMAP_SpectrumWide,
      COLORMAP_Thermal,
      COLORMAP_ThermalWide,

      // Diverging
      COLORMAP_CoolWarm,
      COLORMAP_PurpleOrange,
      COLORMAP_GreenPurple,
      COLORMAP_BlueDarkYellow,
      COLORMAP_GreenRed,

      COLORMAP_ANY = 32765,

      // For Internal use only
      COLORMAP_END = -1
    };

    // Static Functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // get_colormap
    // -------------------------------------------------------------------------
    static void get_colormap(ColormapIndex in_index,
                             unsigned int in_color_num,
                             uint8_t *out_colormap,
                             unsigned int in_multi_num = 1,
                             double in_gain = 1.0, int in_offset = 0)
    {
      unsigned int i, index, num, total, offset, num_all;
      std::vector<ColormapData> colormap_data;
      double ratio0, ratio1, offset_ratio;

      get_multi_colormap_data(in_index, in_multi_num, colormap_data);
      if (colormap_data.size() < 2 || in_gain <= 0.0 || in_color_num == 0)
      {
        clear_colormap(in_color_num, out_colormap);
        return;
      }

      total = 0;
      index = 0;
      num = 0;
      offset_ratio = (double) in_offset / (double) in_color_num;
      ratio0 = colormap_data[index].ratio / in_gain - offset_ratio;
      if (ratio0 > 0)
      {
        num = (int) ((double) in_color_num * ratio0);
        for (i = 0; i < num; i++)
        {
          out_colormap[0] = colormap_data[index].rgb.R;
          out_colormap[1] = colormap_data[index].rgb.G;
          out_colormap[2] = colormap_data[index].rgb.B;
          out_colormap += 3;
          total++;
        }
      }

      const uint8_t *rgb0;
      const uint8_t *rgb1 = nullptr;
      while (index + 1 < colormap_data.size())
      {
        ratio1 = colormap_data[index + 1].ratio / in_gain - offset_ratio;
        rgb0 = (const uint8_t *) &(colormap_data[index].rgb);
        rgb1 = (const uint8_t *) &(colormap_data[index + 1].rgb);
        if (ratio1 > 0)
        {
          if (ratio1 == 1.0)  // <- this is to absorb calculation error
            num_all = in_color_num - total;
          else
            num_all = (int) (ratio1 * in_color_num) -
                      (int) (ratio0 * in_color_num);  // Do not use (ratio1 - ratio0) * in_color_num
          if (ratio0 < 0.0)
          {
            if (ratio1 < 1.0)
              num = (unsigned int) ((double) in_color_num * ratio1);
            else
              num = in_color_num;
            offset = (int) ((0.0 - ratio0) * (double) in_color_num);
          } else
          {
            num = num_all;
            offset = 0;
          }
          if (num > in_color_num - total)
            num = in_color_num - total;
          switch (colormap_data[index].type)
          {
            case CMType_Linear:
              calc_linear_colormap(rgb0, rgb1, offset, num_all,
                                   num, out_colormap);
              break;
            case CMType_Diverging:
              calc_diverging_colormap(rgb0, rgb1, offset, num_all,
                                      num, out_colormap);
              break;
          }
        }
        ratio0 = ratio1;
        out_colormap += num * 3;
        total += num;
        index++;
        if (total == in_color_num)
          break;
      }

      if (total < in_color_num)
      {
        num = in_color_num - total;
        for (i = 0; i < num; i++)
        {
          out_colormap[0] = rgb1[0];
          out_colormap[1] = rgb1[1];
          out_colormap[2] = rgb1[2];
          out_colormap += 3;
        }
      }
    }
    // -------------------------------------------------------------------------
    // get_monomap
    // -------------------------------------------------------------------------
    static void get_monomap(unsigned int in_color_num,
                            uint8_t *out_colormap,
                            double in_gamma = 1.0,
                            double in_gain = 1.0, int in_offset = 0.0)
    {
      if (in_gamma <= 0.0 || in_color_num == 0 || in_gain <= 0.0)
      {
        clear_colormap(in_color_num, out_colormap);
        return;
      }

      double pitch = 1.0 / (double) (in_color_num - 1.0);
      in_gamma = 1.0 / in_gamma;
      for (int i = 0; i < in_color_num; i++)
      {
        double v = pitch * (i + in_offset) * in_gain;
        if (v < 0.0)
          v = 0.0;
        if (v > 1.0)
          v = 1.0;
        v = pow(v, in_gamma);
        v = v * 255.0;
        if (v >= 255)
          v = 255;
        out_colormap[0] = (uint8_t) v;
        out_colormap[1] = (uint8_t) v;
        out_colormap[2] = (uint8_t) v;
        out_colormap += 3;
      }
    }
    // -------------------------------------------------------------------------
    // clear_colormap
    // -------------------------------------------------------------------------
    static void clear_colormap(unsigned int in_color_num, uint8_t *out_colormap)
    {
      if (in_color_num == 0)
        return;
      std::memset(out_colormap, 0, in_color_num * 3);
    }
    // -------------------------------------------------------------------------
    // calc_linear_colormap
    // -------------------------------------------------------------------------
    static void calc_linear_colormap(const uint8_t *in_rgb0, const uint8_t *in_rgb1,
                                     unsigned int in_offset, unsigned int in_color_num_all,
                                     unsigned int in_map_num, uint8_t *out_colormap)
    {
      double interp, k, v;

      k = 1.0 / (double) (in_color_num_all - 1.0);
      for (unsigned int i = 0; i < in_map_num; i++)
      {
        unsigned int t = i + in_offset;
        if (t >= in_color_num_all) // Sanity is_valid
          t = in_color_num_all - 1;
        interp = (double) t * k;
        for (int j = 0; j < 3; j++)
        {
          v = (1.0 - interp) * in_rgb0[j] + interp * in_rgb1[j];
          if (v > 255)
            v = 255;
          out_colormap[i * 3 + j] = (uint8_t) v;
        }
      }
    }
    // -------------------------------------------------------------------------
    // calc_diverging_colormap
    // -------------------------------------------------------------------------
    static void calc_diverging_colormap(const uint8_t *in_rgb0, const uint8_t *in_rgb1,
                                        unsigned int in_offset, unsigned int in_color_num_all,
                                        unsigned int in_map_num, uint8_t *out_color_map)
    {
      double interp, k;

      k = 1.0 / (double) (in_color_num_all - 1.0);
      for (unsigned int i = 0; i < in_map_num; i++)
      {
        unsigned int t = i + in_offset;
        if (t >= in_color_num_all) // Sanity is_valid
          t = in_color_num_all - 1;
        interp = (double) t * k;
        interpolate_color(in_rgb0, in_rgb1, interp, &(out_color_map[i * 3]));
      }
    }
    // -------------------------------------------------------------------------
    // interpolate_color
    // -------------------------------------------------------------------------
    static void interpolate_color(const uint8_t *in_rgb0, const uint8_t *in_rgb1,
                                  double in_interp, uint8_t *out_rgb)
    {
      double msh0[3], msh1[3], msh[3], m;

      conv_rgb_to_msh(in_rgb0, msh0);
      conv_rgb_to_msh(in_rgb1, msh1);

      if ((msh0[1] > 0.05 && msh1[1] > 0.05) && fabs(msh0[2] - msh1[2]) > 1.0472)
      {
        if (msh0[0] > msh1[0])
          m = msh0[0];
        else
          m = msh1[0];
        if (m < 88)
          m = 88;
        if (in_interp < 0.5)
        {
          msh1[0] = m;
          msh1[1] = 0;
          msh1[2] = 0;
          in_interp = 2 * in_interp;
        } else
        {
          msh0[0] = m;
          msh0[1] = 0;
          msh0[2] = 0;
          in_interp = 2 * in_interp - 1;
        }
      }

      if (msh0[1] < 0.05 && msh1[1] > 0.05)
        msh0[2] = adjust_hue(msh1, msh0[0]);
      else if (msh0[1] > 0.05 && msh1[1] < 0.05)
        msh1[2] = adjust_hue(msh0, msh1[0]);

      for (int i = 0; i < 3; i++)
        msh[i] = (1 - in_interp) * msh0[i] + in_interp * msh1[i];

      conv_msh_to_rgb(msh, out_rgb);
    }
    // -------------------------------------------------------------------------
    // adjust_hue
    // -------------------------------------------------------------------------
    static double adjust_hue(const double *in_msh, double in_munsat)
    {
      if (in_msh[0] >= in_munsat)
        return in_msh[2];

      double hSpin = in_msh[1] * sqrt(in_munsat * in_munsat - in_msh[0] * in_msh[0]) /
                     (in_msh[0] * sin(in_msh[1]));

      if (in_msh[2] > -1.0472)
        return in_msh[2] + hSpin;
      return in_msh[2] - hSpin;
    }
    // -------------------------------------------------------------------------
    // conv_rgb_to_msh
    // -------------------------------------------------------------------------
    static void conv_rgb_to_msh(const uint8_t *in_rgb, double *out_msh)
    {
      double rgbL[3];
      double xyz[3];
      double lab[3];

      conv_rgb_to_lin_rgb(in_rgb, rgbL);
#ifdef SHL_IMAGE_COLORMAP_USE_D50
      conv_lin_rgb_to_xyz_d50(rgbL, xyz);
      conv_xyz_d50_to_lab(xyz, lab);
#else
      conv_lin_rgb_to_xyz(rgbL, xyz);
      conv_xyz_d65_to_lab(xyz, lab);
#endif
      conv_lab_to_msh(lab, out_msh);
    }
    // -------------------------------------------------------------------------
    // conv_msh_to_rgb
    // -------------------------------------------------------------------------
    static void conv_msh_to_rgb(const double *in_msh, uint8_t *out_rgb)
    {
      double lab[3];
      double xyz[3];
      double rgbL[3];

      conv_msh_to_Lab(in_msh, lab);
#ifdef SHL_IMAGE_COLORMAP_USE_D50
      conv_lab_to_xyz_d50(lab, xyz);
      conv_xyz_d50_to_lin_rgb(xyz, rgbL);
#else
      conv_lab_to_xyz_d65(lab, xyz);
      conv_xyz_to_lin_rgb(xyz, rgbL);
#endif
      conv_lin_rgb_to_rgb(rgbL, out_rgb);
    }
    // -------------------------------------------------------------------------
    // conv_lab_to_msh
    // -------------------------------------------------------------------------
    static void conv_lab_to_msh(const double *in_lab, double *out_msh)
    {
      out_msh[0] = sqrt(in_lab[0] * in_lab[0] + in_lab[1] * in_lab[1] + in_lab[2] * in_lab[2]);
      out_msh[1] = acos(in_lab[0] / out_msh[0]);
      out_msh[2] = atan2(in_lab[2], in_lab[1]);
    }
    // -------------------------------------------------------------------------
    // conv_msh_to_Lab
    // -------------------------------------------------------------------------
    static void conv_msh_to_Lab(const double *in_msh, double *out_lab)
    {
      out_lab[0] = in_msh[0] * cos(in_msh[1]);
      out_lab[1] = in_msh[0] * sin(in_msh[1]) * cos(in_msh[2]);
      out_lab[2] = in_msh[0] * sin(in_msh[1]) * sin(in_msh[2]);
    }
    // -------------------------------------------------------------------------
    // conv_xyz_d50_to_lab
    // -------------------------------------------------------------------------
    static void conv_xyz_d50_to_lab(const double *in_xyz, double *out_lab)
    {
      const double *wpXyz = get_d50_whitepoint_in_xyz();

      out_lab[0] = 116 * lab_sub_func(in_xyz[1] / wpXyz[1]) - 16.0;
      out_lab[1] = 500 * (lab_sub_func(in_xyz[0] / wpXyz[0]) - lab_sub_func(in_xyz[1] / wpXyz[1]));
      out_lab[2] = 200 * (lab_sub_func(in_xyz[1] / wpXyz[1]) - lab_sub_func(in_xyz[2] / wpXyz[2]));
    }
    // -------------------------------------------------------------------------
    // conv_lab_to_xyz_d50
    // ------------------------------------------------------------------------
    static void conv_lab_to_xyz_d50(const double *in_lab, double *out_xyz)
    {
      const double *wpXyz = get_d50_whitepoint_in_xyz();

      out_xyz[0] = lab_sub_inv_func((in_lab[0] + 16) / 116.0 + (in_lab[1] / 500.0)) * wpXyz[0];
      out_xyz[1] = lab_sub_inv_func((in_lab[0] + 16) / 116.0) * wpXyz[1];
      out_xyz[2] = lab_sub_inv_func((in_lab[0] + 16) / 116.0 - (in_lab[2] / 200.0)) * wpXyz[2];
    }
    // -------------------------------------------------------------------------
    // conv_xyz_d65_to_lab
    // -------------------------------------------------------------------------
    static void conv_xyz_d65_to_lab(const double *in_xyz, double *out_lab)
    {
      const double *wpXyz = get_d65_whitepoint_in_xyz();

      out_lab[0] = 116 * lab_sub_func(in_xyz[1] / wpXyz[1]) - 16.0;
      out_lab[1] = 500 * (lab_sub_func(in_xyz[0] / wpXyz[0]) - lab_sub_func(in_xyz[1] / wpXyz[1]));
      out_lab[2] = 200 * (lab_sub_func(in_xyz[1] / wpXyz[1]) - lab_sub_func(in_xyz[2] / wpXyz[2]));
    }
    // -------------------------------------------------------------------------
    // conv_lab_to_xyz_d65
    // -------------------------------------------------------------------------
    static void conv_lab_to_xyz_d65(const double *in_lab, double *out_xyz)
    {
      const double *wpXyz = get_d65_whitepoint_in_xyz();

      out_xyz[0] = lab_sub_inv_func((in_lab[0] + 16) / 116.0 + (in_lab[1] / 500.0)) * wpXyz[0];
      out_xyz[1] = lab_sub_inv_func((in_lab[0] + 16) / 116.0) * wpXyz[1];
      out_xyz[2] = lab_sub_inv_func((in_lab[0] + 16) / 116.0 - (in_lab[2] / 200.0)) * wpXyz[2];
    }
    // -------------------------------------------------------------------------
    // conv_lin_rgb_to_xyz (from sRGB linear (D65) to XYZ (D65) color space)
    // -------------------------------------------------------------------------
    static void conv_lin_rgb_to_xyz(const double *in_rgb_l, double *out_xyz)
    {
      out_xyz[0] = 0.412391 * in_rgb_l[0] + 0.357584 * in_rgb_l[1] + 0.180481 * in_rgb_l[2];
      out_xyz[1] = 0.212639 * in_rgb_l[0] + 0.715169 * in_rgb_l[1] + 0.072192 * in_rgb_l[2];
      out_xyz[2] = 0.019331 * in_rgb_l[0] + 0.119195 * in_rgb_l[1] + 0.950532 * in_rgb_l[2];
    }
    // -------------------------------------------------------------------------
    // conv_xyz_to_lin_rgb (from XYZ (D65) to sRGB linear (D65) color space)
    // -------------------------------------------------------------------------
    static void conv_xyz_to_lin_rgb(const double *in_xyz, double *out_rgb_l)
    {
      out_rgb_l[0] = 3.240970 * in_xyz[0] - 1.537383 * in_xyz[1] - 0.498611 * in_xyz[2];
      out_rgb_l[1] = -0.969244 * in_xyz[0] + 1.875968 * in_xyz[1] + 0.041555 * in_xyz[2];
      out_rgb_l[2] = 0.055630 * in_xyz[0] - 0.203977 * in_xyz[1] + 1.056972 * in_xyz[2];
    }
    // -------------------------------------------------------------------------
    // conv_lin_rgb_to_xyz_d50 (from sRGB linear (D65) to XYZ (D50) color space)
    // -------------------------------------------------------------------------
    static void conv_lin_rgb_to_xyz_d50(const double *in_rgb_l, double *out_xyz)
    {
      out_xyz[0] = 0.436041 * in_rgb_l[0] + 0.385113 * in_rgb_l[1] + 0.143046 * in_rgb_l[2];
      out_xyz[1] = 0.222485 * in_rgb_l[0] + 0.716905 * in_rgb_l[1] + 0.060610 * in_rgb_l[2];
      out_xyz[2] = 0.013920 * in_rgb_l[0] + 0.097067 * in_rgb_l[1] + 0.713913 * in_rgb_l[2];
    }
    // -------------------------------------------------------------------------
    // conv_xyz_d50_to_lin_rgb (from XYZ (D50) to sRGB linear (D65) color space)
    // -------------------------------------------------------------------------
    static void conv_xyz_d50_to_lin_rgb(const double *in_xyz, double *out_rgb_l)
    {
      out_rgb_l[0] = 3.134187 * in_xyz[0] - 1.617209 * in_xyz[1] - 0.490694 * in_xyz[2];
      out_rgb_l[1] = -0.978749 * in_xyz[0] + 1.916130 * in_xyz[1] + 0.033433 * in_xyz[2];
      out_rgb_l[2] = 0.071964 * in_xyz[0] - 0.228994 * in_xyz[1] + 1.405754 * in_xyz[2];
    }
    // -------------------------------------------------------------------------
    // conv_rgb_to_lin_rgb (from sRGB to linear sRGB)
    // -------------------------------------------------------------------------
    static void conv_rgb_to_lin_rgb(const uint8_t *in_rgb, double *out_rgb_l)
    {
      double value;

      for (int i = 0; i < 3; i++)
      {
        value = (double) in_rgb[i] / 255.0;
        if (value <= 0.040450)
          value = value / 12.92;
        else
          value = pow((value + 0.055) / 1.055, 2.4);
        out_rgb_l[i] = value;
      }
    }
    // -------------------------------------------------------------------------
    // conv_lin_rgb_to_rgb (from linear sRGB to sRGB)
    // -------------------------------------------------------------------------
    static void conv_lin_rgb_to_rgb(const double *in_rgb_l, uint8_t *out_rgb)
    {
      double value;

      for (int i = 0; i < 3; i++)
      {
        value = in_rgb_l[i];
        if (value <= 0.0031308)
          value = value * 12.92;
        else
          value = 1.055 * pow(value, 1.0 / 2.4) - 0.055;

        value = value * 255;
        if (value < 0)
          value = 0;
        if (value > 255)
          value = 255;
        out_rgb[i] = (uint8_t) value;
      }
    }

  private:
    // Constants ---------------------------------------------------------------
    enum ColorMapType
    {
      CMType_Linear = 1,
      CMType_Diverging
    };
    // Typedefs ----------------------------------------------------------------
    typedef struct
    {
      uint8_t R;
      uint8_t G;
      uint8_t B;
    } ColormapRGB;
    typedef struct
    {
      ColormapIndex index;
      double ratio;
      ColorMapType type;
      ColormapRGB rgb;
    } ColormapData;

    // Static Functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // get_d50_whitepoint_in_xyz
    // -------------------------------------------------------------------------
    static const double *get_d50_whitepoint_in_xyz()
    {
      static const double sD50WhitePoint[3] = {0.9642, 1.0, 0.8249};
      return sD50WhitePoint;
    }
    // -------------------------------------------------------------------------
    // get_d50_whitepoint_in_xyz
    // -------------------------------------------------------------------------
    static const double *get_d65_whitepoint_in_xyz()
    {
      static const double sD65WhitePoint[3] = {0.95047, 1.0, 1.08883};
      return sD65WhitePoint;
    }
    // -------------------------------------------------------------------------
    // lab_sub_func
    // -------------------------------------------------------------------------
    static double lab_sub_func(double inT)
    {
      if (inT > 0.008856)
        return pow(inT, (1.0 / 3.0));
      return 7.78703 * inT + 16.0 / 116.0;
    }
    // -------------------------------------------------------------------------
    // lab_sub_inv_func
    // -------------------------------------------------------------------------
    static double lab_sub_inv_func(double inT)
    {
      if (inT > 0.20689)
        return pow(inT, 3);
      return (inT - 16.0 / 116.0) / 7.78703;
    }
    // -------------------------------------------------------------------------
    // get_multi_colormap_data
    // -------------------------------------------------------------------------
    static void get_multi_colormap_data(ColormapIndex in_index,
                                        unsigned int in_multi_num,
                                        std::vector<ColormapData> &out_data)
    {
      const ColormapData *colormap_data;
      unsigned int i, j, data_num;
      double single_ratio;

      colormap_data = get_colormap_data(in_index);
      if (colormap_data == nullptr)
        return;

      data_num = 0;
      while (colormap_data[data_num].index == in_index)
        data_num++;
      single_ratio = 1.0 / (double) in_multi_num;

      out_data.clear();
      for (i = 0; i < in_multi_num; i++)
      {
        for (j = 0; j < data_num; j++)
        {
          out_data.push_back(colormap_data[j]);
          out_data.back().ratio = colormap_data[j].ratio * single_ratio + single_ratio * i;
        }
      }
    }
    // -------------------------------------------------------------------------
    // get_colormap_data
    // -------------------------------------------------------------------------
    static const ColormapData *get_colormap_data(ColormapIndex in_index)
    {
      static const ColormapData s_colormap_data[] =
              {
                      // GrayScale
                      {COLORMAP_GrayScale,      0.0,  CMType_Linear,    {0,   0,   0}},
                      {COLORMAP_GrayScale,      1.0,  CMType_Linear,    {255, 255, 255}},
                      // Jet
                      {COLORMAP_Jet,            0.0,  CMType_Linear,    {0,   0,   127}},
                      {COLORMAP_Jet,            0.1,  CMType_Linear,    {0,   0,   255}},
                      {COLORMAP_Jet,            0.35, CMType_Linear,    {0,   255, 255}},
                      {COLORMAP_Jet,            0.5,  CMType_Linear,    {0,   255, 0}},
                      {COLORMAP_Jet,            0.65, CMType_Linear,    {255, 255, 0}},
                      {COLORMAP_Jet,            0.9,  CMType_Linear,    {255, 0,   0}},
                      {COLORMAP_Jet,            1.0,  CMType_Linear,    {127, 0,   0}},
                      // Rainbow
                      {COLORMAP_Rainbow,        0.0,  CMType_Linear,    {0,   0,   255}},
                      {COLORMAP_Rainbow,        0.25, CMType_Linear,    {0,   255, 255}},
                      {COLORMAP_Rainbow,        0.5,  CMType_Linear,    {0,   255, 0}},
                      {COLORMAP_Rainbow,        0.75, CMType_Linear,    {255, 255, 0}},
                      {COLORMAP_Rainbow,        1.0,  CMType_Linear,    {255, 0,   0}},
                      // Rainbow Wide
                      {COLORMAP_RainbowWide,    0.0,  CMType_Linear,    {0,   0,   0}},
                      {COLORMAP_RainbowWide,    0.1,  CMType_Linear,    {0,   0,   255}},
                      {COLORMAP_RainbowWide,    0.3,  CMType_Linear,    {0,   255, 255}},
                      {COLORMAP_RainbowWide,    0.5,  CMType_Linear,    {0,   255, 0}},
                      {COLORMAP_RainbowWide,    0.7,  CMType_Linear,    {255, 255, 0}},
                      {COLORMAP_RainbowWide,    0.9,  CMType_Linear,    {255, 0,   0}},
                      {COLORMAP_RainbowWide,    1.0,  CMType_Linear,    {255, 255, 255}},
                      // Spectrum
                      {COLORMAP_Spectrum,       0.0,  CMType_Linear,    {255, 0,   255}},
                      {COLORMAP_Spectrum,       0.1,  CMType_Linear,    {0,   0,   255}},
                      {COLORMAP_Spectrum,       0.3,  CMType_Linear,    {0,   255, 255}},
                      {COLORMAP_Spectrum,       0.45, CMType_Linear,    {0,   255, 0}},
                      {COLORMAP_Spectrum,       0.6,  CMType_Linear,    {255, 255, 0}},
                      {COLORMAP_Spectrum,       1.0,  CMType_Linear,    {255, 0,   0}},
                      // Spectrum Wide
                      {COLORMAP_SpectrumWide,   0.0,  CMType_Linear,    {0,   0,   0}},
                      {COLORMAP_SpectrumWide,   0.1,  CMType_Linear,    {150, 0,   150}},
                      {COLORMAP_SpectrumWide,   0.2,  CMType_Linear,    {0,   0,   255}},
                      {COLORMAP_SpectrumWide,   0.35, CMType_Linear,    {0,   255, 255}},
                      {COLORMAP_SpectrumWide,   0.5,  CMType_Linear,    {0,   255, 0}},
                      {COLORMAP_SpectrumWide,   0.6,  CMType_Linear,    {255, 255, 0}},
                      {COLORMAP_SpectrumWide,   0.9,  CMType_Linear,    {255, 0,   0}},
                      {COLORMAP_SpectrumWide,   1.0,  CMType_Linear,    {255, 255, 255}},
                      // Thermal
                      {COLORMAP_Thermal,        0.0,  CMType_Linear,    {0,   0,   255}},
                      {COLORMAP_Thermal,        0.5,  CMType_Linear,    {255, 0,   255}},
                      {COLORMAP_Thermal,        1.0,  CMType_Linear,    {255, 255, 0}},
                      // Thermal Wide
                      {COLORMAP_ThermalWide,    0.0,  CMType_Linear,    {0,   0,   0}},
                      {COLORMAP_ThermalWide,    0.05, CMType_Linear,    {0,   0,   255}},
                      {COLORMAP_ThermalWide,    0.5,  CMType_Linear,    {255, 0,   255}},
                      {COLORMAP_ThermalWide,    0.95, CMType_Linear,    {255, 255, 0}},
                      {COLORMAP_ThermalWide,    1.0,  CMType_Linear,    {255, 255, 255}},

                      // Cool Warm
                      {COLORMAP_CoolWarm,       0.0,  CMType_Diverging, {59,  76,  192}},
                      {COLORMAP_CoolWarm,       1.0,  CMType_Diverging, {180, 4,   38}},
                      // PurpleOrange
                      {COLORMAP_PurpleOrange,   0.0,  CMType_Diverging, {111, 78,  161}},
                      {COLORMAP_PurpleOrange,   1.0,  CMType_Diverging, {193, 85,  11}},
                      // GreenPurple
                      {COLORMAP_GreenPurple,    0.0,  CMType_Diverging, {21,  135, 51}},
                      {COLORMAP_GreenPurple,    1.0,  CMType_Diverging, {111, 78,  161}},
                      // Blue DarkYellow
                      {COLORMAP_BlueDarkYellow, 0.0,  CMType_Diverging, {55,  133, 232}},
                      {COLORMAP_BlueDarkYellow, 1.0,  CMType_Diverging, {172, 125, 23}},
                      // Green Red
                      {COLORMAP_GreenRed,       0.0,  CMType_Diverging, {21,  135, 51}},
                      {COLORMAP_GreenRed,       1.0,  CMType_Diverging, {193, 54,  59}},

                      // End mark (Don't remove this)
                      {COLORMAP_END,            0.0,  CMType_Linear,    {0,   0,   0}}
              };
      const ColormapData *colormap_data = s_colormap_data;

      while (colormap_data->index != in_index)
      {
        colormap_data++;
        if (colormap_data->index == COLORMAP_END)
          return nullptr;
      }
      return colormap_data;
    }
  };

  // ===========================================================================
  //  PerfCounter class
  // ===========================================================================
  class PerfCounter
  {
  public:
    PerfCounter()
    {
      m_start_count = 0;
    }
    virtual ~PerfCounter() = default;
    uint64_t  get_count()
    {
      struct timespec tp;
      if (clock_gettime(CLOCK_REALTIME, &tp) < 0)
      {
        SHL_ERROR_OUT("clock_gettime() returned error: %d", errno);
        return 0;
      }
      uint64_t v = (uint64_t )tp.tv_nsec;
      v += (uint64_t )tp.tv_sec * 1000000000L;
      return v;
    }
    void start_count()
    {
      m_start_count = get_count();
    }
    uint64_t  elapsed_time()
    {
      return get_count() - m_start_count;
    }

  private:
    uint64_t  m_start_count;
  };

  // ===========================================================================
  //  Data class
  // ===========================================================================
  class Data
  {
  public:
    // -------------------------------------------------------------------------
    // Data destructor
    // -------------------------------------------------------------------------
    virtual ~Data()
    {
      delete m_allocated_buffer_ptr;
    }

    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // allocate
    // -------------------------------------------------------------------------
    /**
     * Allocates the image buffer internally.
     *
     * @param in_width      The width of the image buffer
     * @param in_height     The height of the image
     * @param in_is_mono    The type of the image
     *  - true : Allocates the image buffer for the monochrome image (8bit x 1ch = MONO8)
     *  - false : Allocates the image buffer for the color image (8bit x 3ch = RGB8)
     * @return  The result of the function call
     *  - true : The allocation was successful
     *  - false : The allocation was failed
     */
    bool allocate(int in_width, int in_height, bool in_is_mono = false)
    {
      if (in_width == 0 || in_height == 0)
      {
        cleanup_buffers();
        return false;
      }
      //
      m_external_buffer_ptr = nullptr;
      m_width = in_width;
      m_height = in_height;
      m_is_mono = in_is_mono;
      update_image_buffer_size();
      m_allocated_buffer_ptr = new uint8_t[m_buffer_size];
      if (m_allocated_buffer_ptr == nullptr)
      {
        cleanup_buffers();
        return false;
      }
      ::memset(m_allocated_buffer_ptr, 0, m_buffer_size);
      return true;
    }
    // -------------------------------------------------------------------------
    // set_external_buffer
    // -------------------------------------------------------------------------
    /**
     * Specifies the external image buffer.
     *
     * @param in_buffer_ptr     The pointer for the external image buffer
     * @param in_width          The height of the image
     * @param in_height         The height of the image
     * @param in_is_mono        The type of the image
     *  - true : Allocates the image buffer for the monochrome image (8bit x 1ch = MONO8)
     *  - false : Allocates the image buffer for the color image (8bit x 3ch = RGB8)
     * @param in_skip_frame_counter_update
     *  - true : Skips incrementing the frame counter. The frame counter will be unchanged.
     *  - false : The frame counter will be incremented (updated).
     * @return  The result of the function call
     *  - true : Changing the image buffer was successful
     *  - false : An error has occurred. The parameter specified was wrong.
     */
    bool set_external_buffer(uint8_t *in_buffer_ptr, int in_width, int in_height, bool in_is_mono = false,
                             bool in_skip_frame_counter_update = false)
    {
      if (in_buffer_ptr == nullptr || in_width == 0 || in_height == 0)
      {
        cleanup_buffers();
        return false;
      }
      //
      if (m_allocated_buffer_ptr != nullptr)
      {
        delete m_allocated_buffer_ptr;
        m_allocated_buffer_ptr = nullptr;
      }
      m_external_buffer_ptr = in_buffer_ptr;
      m_width = in_width;
      m_height = in_height;
      m_is_mono = in_is_mono;
      update_image_buffer_size();
      mark_as_modified(in_skip_frame_counter_update);
      return true;
    }
    // -------------------------------------------------------------------------
    // get_image
    // -------------------------------------------------------------------------
    /**
     * Retrieves the pointer for the image buffer.
     * @note The image buffer needs to be allocated beforehand.
     *
     * @return The pointer for the image buffer
     */
    [[nodiscard]] uint8_t *get_image() const
    {
      if (m_allocated_buffer_ptr != nullptr)
        return m_allocated_buffer_ptr;
      return m_external_buffer_ptr;
    }
    // -------------------------------------------------------------------------
    // get_pixel_value
    // -------------------------------------------------------------------------
    /**
     * Retrieves the pixel value of the image buffer.
     * @note The image buffer needs to be allocated beforehand.
     *
     * @param in_x          The x position of the pixel
     * @param in_y          The y position of the pixel
     * @param out_is_mono   The type of the image
     *  - true : Allocates the image buffer for the monochrome image (8bit x 1ch = MONO8)
     *  - false : Allocates the image buffer for the color image (8bit x 3ch = RGB8)
     * @param out_r         The pixel value of the specified location (The r component or monochrome pixel value)
     * @param out_g         The pixel value of the specified location (The g component of the pixel value)
     * @param out_b         The pixel value of the specified location (The b component of the pixel value)
     * @return  The result of the function call
     *  - true : Changing the image buffer was successful
     *  - false : An error has occurred. The parameter specified was wrong.
     */
    virtual bool get_pixel_value(int in_x, int in_y,
                         bool *out_is_mono, int *out_r, int *out_g, int *out_b)
    {
      if (is_valid() == false)
        return false;
      if (in_x < 0 || in_x > get_width() ||
          in_y < 0 || in_y > get_height())
        return false;
      //
      *out_is_mono = is_mono();
      unsigned char *pixBuf = get_image();
      if (*out_is_mono)
      {
        *out_r = pixBuf[in_y * get_width() + in_x];
        *out_g = 0;
        *out_b = 0;
      }
      else
      {
        size_t index = (in_y * get_width() + in_x) * 3;
        *out_r = pixBuf[index];
        *out_g = pixBuf[index+1];
        *out_b = pixBuf[index+2];
      }
      return true;
    }
    // -------------------------------------------------------------------------
    // mark_as_modified
    // -------------------------------------------------------------------------
    /**
     * Marks the image buffer as modified. Once the modified flag of the image
     * is set by calling this function and if there is a window displaying
     * the image buffer, the window will be redrawed to display the updated
     * image buffer.
     *
     * @param in_skip_frame_counter_update
     *  - true : Will skip incrementing the frame counter
     *  - false : Will not increment the frame counter
     */
    void mark_as_modified(bool in_skip_frame_counter_update = false)
    {
      m_is_image_modified = true;
      if (in_skip_frame_counter_update == false)
        increment_frame_counter();
    }
    // -------------------------------------------------------------------------
    // is_modified
    // -------------------------------------------------------------------------
    /**
     * Retrieves the modified flag of the image buffer
     *
     * @return  The state of the modified flag
     *  - true : The image buffer is marked as modified
     *  - false : The image buffer is not marked as modified
     */
    [[nodiscard]] bool is_modified() const
    {
      return m_is_image_modified;
    }
    // -------------------------------------------------------------------------
    // clear_modified_flag
    // -------------------------------------------------------------------------
    /**
     * Clears the modified flag of the image buffer.
     */
    void clear_modified_flag()
    {
      m_is_image_modified = false;
    }
    // -------------------------------------------------------------------------
    // set_frame_counter
    // -------------------------------------------------------------------------
    /**
     * Sets the frame counter of the image buffer.
     *
     * @param in_frame_counter  The new frame counter value
     */
    void set_frame_counter(unsigned int in_frame_counter)
    {
      m_frame_counter_initialized = true;
      m_frame_counter = in_frame_counter;
    }
    // -------------------------------------------------------------------------
    // get_frame_counter
    // -------------------------------------------------------------------------
    /**
     * Retrieves the frame counter of the image buffer.
     *
     * @return  The frame counter value of the image buffer
     */
    [[nodiscard]] unsigned int get_frame_counter() const
    {
      if (m_frame_counter_initialized == false)
        return 0;
      return m_frame_counter;
    }
    // -------------------------------------------------------------------------
    // increment_frame_counter
    // -------------------------------------------------------------------------
    /**
     * Increments the frame counter of the image buffer.
     */
    void increment_frame_counter()
    {
      if (m_frame_counter_initialized == false)
      {
        m_frame_counter_initialized = true;
        m_frame_counter = 0;
        return;
      }
      m_frame_counter++;
    }
    // -------------------------------------------------------------------------
    // reset_frame_counter
    // -------------------------------------------------------------------------
    /**
     * Resets the frame counter to zero.
     */
    void reset_frame_counter()
    {
      m_frame_counter_initialized = false;
      m_frame_counter = 0;
    }
    // -------------------------------------------------------------------------
    // get_colormap_index
    // -------------------------------------------------------------------------
    /**
     * Retrieves the colormap index of the image buffer. The colormap index
     * is used only when the type of the image buffer is monochrome.
     *
     * @return  The colormap index of the image buffer
     */
    [[nodiscard]] Colormap::ColormapIndex get_colormap_index() const
    {
      return m_colormap_index;
    }
    // -------------------------------------------------------------------------
    // set_colormap_index
    // -------------------------------------------------------------------------
    /**
     * Set the colormap index of the image buffer. The colormap index
     * is used only when the type of the image buffer is monochrome.
     * @note The modified flag of the image buffer will bet set
     * by calling this function.
     *
     * @param in_index  The new colormap index of the image buffer
     * @param in_skip_frame_counter_update
     *  - true : Will skip incrementing the frame counter
     *  - false : Will not increment the frame counter
     */
    void set_colormap_index(Colormap::ColormapIndex in_index,
                            bool in_skip_frame_counter_update = true)
    {
      if (m_colormap_index == in_index)
        return;
      m_colormap_index = in_index;
      mark_as_modified(in_skip_frame_counter_update);
    }
    // -------------------------------------------------------------------------
    // get_width
    // -------------------------------------------------------------------------
    /**
     * Retrieves the width of the image buffer.
     *
     * @return The width of the image buffer
     */
    [[nodiscard]] int get_width() const
    {
      return m_width;
    }
    // -------------------------------------------------------------------------
    // get_height
    // -------------------------------------------------------------------------
    /**
     * Retries the height of the image buffer.
     *
     * @return The height of the image buffer
     */
    [[nodiscard]] int get_height() const
    {
      return m_height;
    }
    // -------------------------------------------------------------------------
    // is_mono
    // -------------------------------------------------------------------------
    /**
     * Retrieves the type of the image buffer (Mono or Color).
     *
     * @return The image buffer type
     *  - true : The image buffer type is monochrome (MONO8)
     *  - false : The image buffer type is color (RGB8)
     */
    [[nodiscard]] bool is_mono() const
    {
      return m_is_mono;
    }
    // -------------------------------------------------------------------------
    // get_buffer_size
    // -------------------------------------------------------------------------
    /**
     * Retrieves the size of the image buffer on the memory.
     *
     * @return The size of the image (bytes)
     */
    [[nodiscard]] size_t get_buffer_size() const
    {
      return m_buffer_size;
    }
    // -------------------------------------------------------------------------
    // is_valid
    // -------------------------------------------------------------------------
    /**
     * Checks whether the object holds a valid image buffer or not.
     * @note To become the valid state, allocate() or set_external_buffer()
     * needs to be called and returns with the success
     *
     * @return
     *  - true : The image buffer is valid.
     *  - false : The image buffer is invalid.
     */
    [[nodiscard]] bool is_valid() const
    {
      if (m_allocated_buffer_ptr == nullptr && m_external_buffer_ptr == nullptr)
        return false;
      if (m_width == 0 || m_height == 0 || m_buffer_size == 0)
        return false;
      return true;
    }

  protected:
    // -------------------------------------------------------------------------
    // Data constructor
    // -------------------------------------------------------------------------
    Data()
    {
      m_allocated_buffer_ptr = nullptr;
      m_external_buffer_ptr = nullptr;
      m_buffer_size = 0;
      m_width = 0;
      m_height = 0;
      m_is_mono = false;
      m_is_image_modified = false;
      m_colormap_index = Colormap::COLORMAP_GrayScale;
      reset_frame_counter();
    }

    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // cleanup_buffers
    // -------------------------------------------------------------------------
    void cleanup_buffers()
    {
      if (m_allocated_buffer_ptr != nullptr)
      {
        delete m_allocated_buffer_ptr;
        m_allocated_buffer_ptr = nullptr;
      }
      m_external_buffer_ptr = nullptr;
      m_buffer_size = 0;
      m_width = 0;
      m_height = 0;
      m_is_mono = false;
      m_is_image_modified = false;
    }
    // -------------------------------------------------------------------------
    // update_image_buffer_size
    // -------------------------------------------------------------------------
    void update_image_buffer_size()
    {
      m_buffer_size = m_width * m_height;
      if (m_is_mono == false)
        m_buffer_size *= 3;
      //
    }

  private:
    // member variables --------------------------------------------------------
    uint8_t *m_allocated_buffer_ptr;
    uint8_t *m_external_buffer_ptr;
    size_t m_buffer_size;
    int m_width;
    int m_height;
    bool m_is_mono;
    Colormap::ColormapIndex m_colormap_index;
    bool m_frame_counter_initialized;
    unsigned int m_frame_counter;

    bool m_is_image_modified;

    // friend classes ----------------------------------------------------------
    friend class View;
  };

  // ===========================================================================
  //  UpdateHandlerInterface class
  // ===========================================================================
  class UpdateHandlerInterface
  {
  public:
    virtual void view_zoom_updated(double in_zoom, bool in_best_fit) = 0;
    virtual void view_image_info_updated(bool in_is_valid_image_info,
                                int in_image_width, int in_image_height,
                                bool in_is_image_mono) = 0;
    virtual void view_mouse_info_updated(bool in_is_valid_mouse_info,
                                            int in_mouse_x, int in_mouse_y,
                                            bool in_is_mouse_mono,
                                            int in_mouse_r, int in_mouse_g, int in_mouse_b) = 0;
    virtual void view_frame_info_updated(bool in_is_valid_frame_info,
                                        unsigned int in_frame_count, double in_fps) = 0;
  };

  // ===========================================================================
  // View class
  // ===========================================================================
  // Note: Order of Scrollable -> Widget is very important
  //
  class View : virtual public Gtk::Scrollable, virtual public Gtk::Widget
  {
    // Class related macros
#define IM_VIEW_COLORMAP_COLOR_NUM      256
#define IM_VIEW_COLORMAP_DATA_SIZE      (IM_VIEW_COLORMAP_COLOR_NUM * 3)

  public:
    // -------------------------------------------------------------------------
    // View destructor
    // -------------------------------------------------------------------------
    ~View() override
    {
      SHL_DBG_OUT("View was deleted");
    }

  protected:
    // -------------------------------------------------------------------------
    // View constructor
    // -------------------------------------------------------------------------
    View() :
            Glib::ObjectBase("View"),
            m_h_adjustment(*this, "hadjustment"),
            m_v_adjustment(*this, "vadjustment"),
            m_h_scroll_policy(*this, "hscroll-policy", Gtk::SCROLL_NATURAL),
            m_v_scroll_policy(*this, "vscroll-policy", Gtk::SCROLL_NATURAL)
    {
      set_has_window(true);
      property_hadjustment().signal_changed().connect(sigc::mem_fun(
              *this, &shl::gtk::image::View::h_adjustment_changed));
      property_vadjustment().signal_changed().connect(sigc::mem_fun(
              *this, &shl::gtk::image::View::v_adjustment_changed));

      m_width = 0;
      m_height = 0;
      m_org_width = 0;
      m_org_height = 0;
      m_mouse_x = 0;
      m_mouse_y = 0;
      m_mouse_info_x = 0;
      m_mouse_info_y = 0;
      m_mouse_info_updated = false;
      m_offset_x = 0;
      m_offset_y = 0;
      m_offset_x_max = 0;
      m_offset_y_max = 0;
      m_offset_x_org = 0;
      m_offset_y_org = 0;
      m_window_x = 0;
      m_window_y = 0;
      m_window_width = 0;
      m_window_height = 0;
      m_zoom = 1.0;
      m_zoom_best_fit = false;
      m_mouse_l_pressed = false;
      m_adjustments_modified = false;

      m_image_data_ptr = nullptr;
      m_is_image_size_changed = false;

      m_fps = 0;
      m_fps_sum = 0;
      m_fps_sum_num = 0;

      m_colormap_index = Colormap::COLORMAP_NOT_SPECIFIED;
      std::memset(m_colormap, 0, IM_VIEW_COLORMAP_DATA_SIZE);

      add_events(Gdk::SCROLL_MASK |
                 Gdk::BUTTON_MOTION_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
                 Gdk::POINTER_MOTION_MASK | Gdk::LEAVE_NOTIFY_MASK);
    }

    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // set_image_data
    // -------------------------------------------------------------------------
    void set_image_data(Data *inImageDataPtr)
    {
      m_image_data_ptr = inImageDataPtr;
      queue_draw();
    }
    // -------------------------------------------------------------------------
    // get_image_data
    // -------------------------------------------------------------------------
    Data *get_image_data()
    {
      return m_image_data_ptr;
    }
    // -------------------------------------------------------------------------
    // add_update_handler
    // -------------------------------------------------------------------------
    void add_update_handler(UpdateHandlerInterface *in_handler)
    {
      m_update_handlers.push_back(in_handler);
    }
    // -------------------------------------------------------------------------
    // remove_update_handler
    // -------------------------------------------------------------------------
    void remove_update_handler(UpdateHandlerInterface *in_handler)
    {
      auto it = std::find(m_update_handlers.begin(),
                          m_update_handlers.end(),
                          in_handler);
      if (it == m_update_handlers.end())
        return;
      m_update_handlers.erase(it);
    }
    // -------------------------------------------------------------------------
    // get_zoom
    // -------------------------------------------------------------------------
    double get_zoom()
    {
      return m_zoom;
    }
    // -------------------------------------------------------------------------
    // set_zoom
    // -------------------------------------------------------------------------
    void set_zoom(double in_zoom)
    {
      set_zoom(in_zoom, m_window_width / 2.0, m_window_height / 2.0);
    }
    // -------------------------------------------------------------------------
    // get_zoom_best_fit
    // -------------------------------------------------------------------------
    bool get_zoom_best_fit() const
    {
      return m_zoom_best_fit;
    }
    // -------------------------------------------------------------------------
    // set_zoom_best_fit
    // -------------------------------------------------------------------------
    void set_zoom_best_fit(bool in_enable)
    {
      m_zoom_best_fit = in_enable;
      if (m_zoom_best_fit == false)
        return;
      adjust_zoom_best_fit();
    }
    // -------------------------------------------------------------------------
    // set_zoom
    // -------------------------------------------------------------------------
    void set_zoom(double in_zoom, double in_x, double in_y)
    {
      double v;
      double prev_zoom = m_zoom;
      m_zoom = in_zoom;

      if (fabs(m_zoom - 1.0) <= 0.01)
        m_zoom = 1.0;
      if (m_zoom <= 0.01)
        m_zoom = 0.01;

      double prev_width = m_width;
      double prev_height = m_height;
      v = m_org_width * m_zoom;
      m_width = v;
      v = m_org_height * m_zoom;
      m_height = v;

      if (m_width <= m_window_width)
        m_offset_x = 0;
      else
      {
        if (prev_width <= m_window_width)
          v = -1 * (m_window_width - prev_width) / 2.0;
        else
          v = 0;
        v = (in_x + m_offset_x + v) / prev_zoom;
        v = v * m_zoom - in_x;
        m_offset_x = v;
        m_offset_x_max = m_width - m_window_width;
        if (m_offset_x > m_offset_x_max)
          m_offset_x = m_offset_x_max;
        if (m_offset_x < 0)
          m_offset_x = 0;
      }
      configure_h_adjustment(); // calling here is important

      if (m_height <= m_window_height)
        m_offset_y = 0;
      else
      {
        if (prev_height <= m_window_height)
          v = -1 * (m_window_height - prev_height) / 2.0;
        else
          v = 0;
        v = (in_y + m_offset_y + v) / prev_zoom;
        v = v * m_zoom - in_y;
        m_offset_y = v;
        m_offset_y_max = m_height - m_window_height;
        if (m_offset_y > m_offset_y_max)
          m_offset_y = m_offset_y_max;
        if (m_offset_y < 0)
          m_offset_y = 0;
      }
      configure_v_adjustment();
      queue_draw();
      invoke_zoom_updated_handlers();
    }
    // -------------------------------------------------------------------------
    // update_pixbuf
    // -------------------------------------------------------------------------
    bool update_pixbuf()
    {
      if (m_image_data_ptr == nullptr)
        return false;
      if (m_image_data_ptr->is_valid() == false)
        return false;
      bool need_to_create = true;
      if (m_pixbuf)
      {
        if (m_pixbuf->get_width() == m_image_data_ptr->get_width() &&
            m_pixbuf->get_height() == m_image_data_ptr->get_height())
          need_to_create = false;
      }
      if (need_to_create)
      {
        m_org_width = m_image_data_ptr->get_width();
        m_org_height = m_image_data_ptr->get_height();
        m_width = m_org_width;
        m_height = m_org_height;
        configure_h_adjustment();
        configure_v_adjustment();
        m_pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8,
                                       m_image_data_ptr->get_width(),
                                       m_image_data_ptr->get_height());
        if (!m_pixbuf)
          return false;
        invoke_image_info_updated_handlers();
      } else
      {
        if (m_image_data_ptr->is_modified() == false)
          return true;
      }
      if (m_image_data_ptr->get_frame_counter() == 0)
      {
        m_fps_counter.start_count();
        m_fps = 0;
        m_fps_sum_counter.start_count();
        m_fps_sum = 0;
        m_fps_sum_num = 0;
      }
      else
      {
        double v;
        v = (double )m_fps_counter.elapsed_time();
        m_fps_counter.start_count();
        if (v != 0)
          v = 1000000000.0 / v;
        m_fps_sum += v;
        m_fps_sum_num++;
        if (m_fps_sum_counter.elapsed_time() > 1000000000)
        {
          m_fps = m_fps_sum / m_fps_sum_num;
          m_fps_sum_counter.start_count();
          m_fps_sum = 0;
          m_fps_sum_num = 0;
        }
      }
      update_mouse_info();
      invoke_frame_info_updated_handlers(true, m_fps);
      if (m_image_data_ptr->is_mono())
      {
        if (m_colormap_index != m_image_data_ptr->get_colormap_index())
        {
          m_colormap_index = m_image_data_ptr->get_colormap_index();
          Colormap::get_colormap(m_colormap_index, IM_VIEW_COLORMAP_COLOR_NUM,
                                 m_colormap);
        }
        size_t data_size = m_image_data_ptr->get_buffer_size();
        uint8_t *src = m_image_data_ptr->get_image();
        uint8_t *dst = m_pixbuf->get_pixels();
        for (int i = 0; i < data_size; i++)
        {
          size_t index = *src * 3;
          src++;
          //
          *dst = m_colormap[index];
          dst++;
          index++;
          *dst = m_colormap[index];
          dst++;
          index++;
          *dst = m_colormap[index];
          dst++;
        }
      } else
      {
        ::memcpy(m_pixbuf->get_pixels(),
                 m_image_data_ptr->get_image(),
                 m_image_data_ptr->get_buffer_size());
      }
      m_image_data_ptr->clear_modified_flag();
      return true;
    }
    // -------------------------------------------------------------------------
    // on_draw
    // -------------------------------------------------------------------------
    bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override
    {
      if (update_pixbuf() == false)
        return false;
      //
      double x, y;
      if (m_width <= m_window_width)
        x = (m_window_width - m_width) / 2;
      else
        x = -1 * m_offset_x;
      if (m_height <= m_window_height)
        y = (m_window_height - m_height) / 2;
      else
        y = -1 * m_offset_y;
      //
      if (m_zoom >= 1)
      {
        //cr->set_identity_matrix();
        cr->translate(x, y);
        cr->scale(m_zoom, m_zoom);
        Gdk::Cairo::set_source_pixbuf(cr, m_pixbuf, 0, 0);
        Cairo::SurfacePattern pattern(cr->get_source()->cobj());
        pattern.set_filter(Cairo::Filter::FILTER_NEAREST);
      } else
      {
        Gdk::Cairo::set_source_pixbuf(cr,
                                      m_pixbuf->scale_simple(
                                              (int) m_width, (int) m_height,
                                              Gdk::INTERP_NEAREST),
                                      x, y);
      }
      cr->paint();
      return true;
    }
    // -------------------------------------------------------------------------
    // on_realize
    // -------------------------------------------------------------------------
    void on_realize() override
    {
      // Do not call base class Gtk::Widget::on_realize().
      // It's intended only for widgets that set_has_window(false).
      set_realized();

      if (!m_window)
      {
        GdkWindowAttr attributes;

        std::memset(&attributes, 0, sizeof(decltype(attributes)));
        const Gtk::Allocation allocation(get_allocation());
        attributes.x = allocation.get_x();
        attributes.y = allocation.get_y();
        attributes.width = allocation.get_width();
        attributes.height = allocation.get_height();
        attributes.event_mask = get_events() | Gdk::EXPOSURE_MASK;
        attributes.window_type = GDK_WINDOW_CHILD;
        attributes.wclass = GDK_INPUT_OUTPUT;

        m_window = Gdk::Window::create(get_parent_window(), &attributes, Gdk::WA_X | Gdk::WA_Y);
        set_window(m_window);

        //make the widget receive expose events
        m_window->set_user_data(Gtk::Widget::gobj());
      }
    }
    // -------------------------------------------------------------------------
    // on_unrealize
    // -------------------------------------------------------------------------
    void on_unrealize() override
    {
      m_window.reset();

      //Call base class:
      Gtk::Widget::on_unrealize();
    }
    // -------------------------------------------------------------------------
    // on_button_press_event
    // -------------------------------------------------------------------------
    bool on_button_press_event(GdkEventButton *button_event) override
    {
      m_mouse_l_pressed = true;
      m_mouse_x = button_event->x;
      m_mouse_y = button_event->y;
      m_offset_x_org = m_offset_x;
      m_offset_y_org = m_offset_y;
      return true;
    }
    // -------------------------------------------------------------------------
    // on_motion_notify_event
    // -------------------------------------------------------------------------
    bool on_motion_notify_event(GdkEventMotion *motion_event) override
    {
      update_mouse_info(motion_event->x, motion_event->y);
      //
      if (m_mouse_l_pressed == false)
        return false;

      if (m_width > m_window_width)
      {
        double d = m_offset_x_org + (m_mouse_x - motion_event->x);
        if (d > m_offset_x_max)
          d = m_offset_x_max;
        if (d < 0)
          d = 0;
        if (d != m_offset_x)
        {
          m_offset_x = d;
          const auto v = property_hadjustment().get_value();
          v->freeze_notify();
          v->set_value(m_offset_x);
          m_adjustments_modified = true;
          v->thaw_notify();
        }
      }
      if (m_height > m_window_height)
      {
        double d = m_offset_y_org + (m_mouse_y - motion_event->y);
        if (d > m_offset_y_max)
          d = m_offset_y_max;
        if (d < 0)
          d = 0;
        if (d != m_offset_y)
        {
          m_offset_y = d;
          const auto v = property_vadjustment().get_value();
          v->freeze_notify();
          v->set_value(m_offset_y);
          m_adjustments_modified = true;
          v->thaw_notify();
        }
      }
      return true;
    }
    // -------------------------------------------------------------------------
    // on_leave_notify_event
    // -------------------------------------------------------------------------
    bool  on_leave_notify_event(GdkEventCrossing *crossing_event) override
    {
      m_mouse_info_updated = false;
      m_mouse_info_x = 0;
      m_mouse_info_y = 0;
      invoke_mouse_info_updated_handlers(false,
                                         0, 0,
                                         false, 0, 0, 0);
      return true;
    }
    // -------------------------------------------------------------------------
    // on_button_release_event
    // -------------------------------------------------------------------------
    bool on_button_release_event(GdkEventButton *release_event) override
    {
      m_mouse_l_pressed = false;
      return true;
    }
    // -------------------------------------------------------------------------
    // on_scroll_event
    // -------------------------------------------------------------------------
    bool on_scroll_event(GdkEventScroll *event) override
    {
      //std::cout << "wheel event\n"
      //          << "time = " << event->time << std::endl
      //          << "x = " << event->x << std::endl
      //          << "y = " << event->y << std::endl
      //          << "state = " << event->state << std::endl
      //          << "direction = " << event->direction << std::endl
      //          << "delta_x = " << event->delta_x << std::endl
      //          << "delta_y = " << event->delta_y << std::endl;

      double v;

      if (event->direction == 0)
        v = 0.02;
      else
        v = -0.02;
      v = log10(m_zoom) + v;
      v = pow(10, v);
      //
      set_zoom_best_fit(false);
      set_zoom(v, event->x, event->y);
      update_mouse_info(event->x, event->y);
      return true;
    }
    // -------------------------------------------------------------------------
    // on_size_allocate
    // -------------------------------------------------------------------------
    void on_size_allocate(Gtk::Allocation &allocation) override
    {
      //Do something with the space that we have actually been given:
      //(We will not be given heights or widths less than we have requested, though
      // we might get more)

      //Use the offered allocation for this container:
      set_allocation(allocation);
      m_window_x = allocation.get_x();
      m_window_y = allocation.get_y();
      m_window_width = allocation.get_width();
      m_window_height = allocation.get_height();

      if (m_window)
      {
        m_window->move_resize((int) m_window_x, (int) m_window_y,
                              (int) m_window_width, (int) m_window_height);
        if (m_zoom_best_fit)
        {
          adjust_zoom_best_fit();
        }
        else
        {
          configure_h_adjustment();
          configure_v_adjustment();
        }
      }
    }
    // -------------------------------------------------------------------------
    // h_adjustment_changed
    // -------------------------------------------------------------------------
    void h_adjustment_changed()
    {
      const auto v = property_hadjustment().get_value();
      if (!v)
        return;
      m_h_adjustment_connection.disconnect();
      m_h_adjustment_connection = v->signal_value_changed().connect(sigc::mem_fun(
              *this, &shl::gtk::image::View::adjustment_value_changed));
      configure_h_adjustment();
    }
    // -------------------------------------------------------------------------
    // v_adjustment_changed
    // -------------------------------------------------------------------------
    void v_adjustment_changed()
    {
      const auto v = property_vadjustment().get_value();
      if (!v)
        return;
      m_v_adjustment_connection.disconnect();
      m_v_adjustment_connection = v->signal_value_changed().connect(sigc::mem_fun(
              *this, &shl::gtk::image::View::adjustment_value_changed));
      configure_v_adjustment();
    }
    // -------------------------------------------------------------------------
    // configure_h_adjustment
    // -------------------------------------------------------------------------
    virtual void configure_h_adjustment()
    {
      const auto v = property_hadjustment().get_value();
      if (!v || m_window_width == 0)
        return;
      v->freeze_notify();
      if (m_width <= m_window_width)
      {
        m_offset_x = 0;
        v->set_value(0);
        v->set_upper(0);
        v->set_step_increment(0);
        v->set_page_size(0);
      } else
      {
        m_offset_x_max = m_width - m_window_width;
        if (m_offset_x > m_offset_x_max)
          m_offset_x = m_offset_x_max;
        m_adjustments_modified = true;
        v->set_upper(m_offset_x_max);
        v->set_value(m_offset_x);
        v->set_step_increment(1);
        v->set_page_size(10);
      }
      v->thaw_notify();
    }
    // -------------------------------------------------------------------------
    // configure_v_adjustment
    // -------------------------------------------------------------------------
    virtual void configure_v_adjustment()
    {
      const auto v = property_vadjustment().get_value();
      if (!v || m_window_height == 0)
        return;
      v->freeze_notify();
      if (m_height <= m_window_height)
      {
        m_offset_y = 0;
        v->set_value(0);
        v->set_upper(0);
        v->set_step_increment(0);
        v->set_page_size(0);
      } else
      {
        m_offset_y_max = m_height - m_window_height;
        if (m_offset_y > m_offset_y_max)
          m_offset_y = m_offset_y_max;
        m_adjustments_modified = true;
        v->set_upper(m_offset_y_max);
        v->set_value(m_offset_y);
        v->set_step_increment(1);
        v->set_page_size(10);
      }
      v->thaw_notify();
    }
    // -------------------------------------------------------------------------
    // adjustment_value_changed
    // -------------------------------------------------------------------------
    virtual void adjustment_value_changed()
    {
      if (m_width > m_window_width && m_adjustments_modified == false)
      {
        const auto v = property_hadjustment().get_value();
        m_offset_x = v->get_value();
      }
      if (m_height > m_window_height && m_adjustments_modified == false)
      {
        const auto v = property_vadjustment().get_value();
        m_offset_y = v->get_value();
      }
      m_adjustments_modified = false;
      queue_draw();
    }
    // -------------------------------------------------------------------------
    // adjust_zoom_best_fit
    // -------------------------------------------------------------------------
    void adjust_zoom_best_fit()
    {
      double src, dst, v, vx, vy;

      vx = m_window_width / m_org_width;
      vy = m_window_height / m_org_height;
      if (vx < vy)
        v = vx;
      else
        v = vy;
      //
      set_zoom(v);
    }
    // -------------------------------------------------------------------------
    // save_pixbuf
    // -------------------------------------------------------------------------
    bool save_pixbuf(const std::string &in_filename, const Glib::ustring &in_type)
    {
      try
      {
        m_pixbuf->save(in_filename, in_type);
      }
      catch (Glib::FileError &ex)
      {
        printf("The exception occurred : %s\n", ex.what().c_str());
        return false;
      }
      catch (Gdk::PixbufError &ex)
      {
        printf("The exception occurred : %s\n", ex.what().c_str());
        return false;
      }
      return true;
    }
    // -------------------------------------------------------------------------
    // save_as_raw
    // -------------------------------------------------------------------------
    bool save_as_raw(const std::string &in_filename)
    {
      uint8_t *image = m_image_data_ptr->get_image();
      size_t  size = m_image_data_ptr->get_buffer_size();
      if (image == nullptr || size == 0 || in_filename.empty())
        return false;
      FILE *fp = fopen(in_filename.c_str(), "wb");
      if (fp == nullptr)
        return false;
      fwrite(image, sizeof(uint8_t), size, fp);
      fclose(fp);
      return true;
    }
    // -------------------------------------------------------------------------
    // invoke_zoom_updated_handlers
    // -------------------------------------------------------------------------
    void invoke_zoom_updated_handlers()
    {
      for (auto handler : m_update_handlers)
        handler->view_zoom_updated(m_zoom, m_zoom_best_fit);
    }
    // -------------------------------------------------------------------------
    // invoke_image_info_updated_handlers
    // -------------------------------------------------------------------------
    void invoke_image_info_updated_handlers()
    {
      for (auto handler : m_update_handlers)
      {
        if (m_image_data_ptr == nullptr || !m_pixbuf)
          handler->view_image_info_updated(false, 0, 0, false);
        else
          handler->view_image_info_updated(
                  m_image_data_ptr->is_valid(),
                  m_image_data_ptr->get_width(),
                  m_image_data_ptr->get_height(),
                  m_image_data_ptr->is_mono());
      }
    }
    // -------------------------------------------------------------------------
    // invoke_mouse_info_updated_handlers
    // -------------------------------------------------------------------------
    void invoke_mouse_info_updated_handlers(bool in_is_valid_mouse_info,
                                            int in_mouse_x, int in_mouse_y,
                                            bool in_is_mouse_mono,
                                            int in_mouse_r, int in_mouse_g, int in_mouse_b)
    {
      for (auto handler : m_update_handlers)
        handler->view_mouse_info_updated(
                in_is_valid_mouse_info,
                in_mouse_x, in_mouse_y,
                in_is_mouse_mono,
                in_mouse_r, in_mouse_g, in_mouse_b);
    }
    // -------------------------------------------------------------------------
    // invoke_frame_info_updated_handlers
    // -------------------------------------------------------------------------
    void invoke_frame_info_updated_handlers(bool in_is_valid_frame_info, double in_fps)
    {
      for (auto handler : m_update_handlers)
      {
        if (m_image_data_ptr == nullptr || !m_pixbuf)
          handler->view_frame_info_updated(false, 0, 0);
        else
          handler->view_frame_info_updated(in_is_valid_frame_info,
                                           m_image_data_ptr->get_frame_counter(),
                                           in_fps);
      }
    }
    // -------------------------------------------------------------------------
    // update_mouse_info
    // -------------------------------------------------------------------------
    void update_mouse_info()
    {
      if (m_mouse_info_updated == false)
        return;
      update_mouse_info(m_mouse_info_x, m_mouse_info_y);
    }
    // -------------------------------------------------------------------------
    // update_mouse_info
    // -------------------------------------------------------------------------
    void update_mouse_info(double in_mouse_x, double in_mouse_y)
    {
      bool is_valid = true;
      double t;
      int x, y;

      if (m_width <= m_window_width)
      {
        t = (m_window_width - m_width) / 2;
        if (in_mouse_x < t || in_mouse_x > t + m_width)
          is_valid = false;
        t = (in_mouse_x - t) / m_zoom;
        x = (int )t;
      }
      else
      {
        t = (m_offset_x + in_mouse_x) / m_zoom;
        x = (int )t;
      }
      if (m_height <= m_window_height)
      {
        t = (m_window_height - m_height) / 2;
        if (in_mouse_y < t || in_mouse_y > t + m_height)
          is_valid = false;
        t = (in_mouse_y- t) / m_zoom;
        y = (int )t;
      }
      else
      {
        t = (m_offset_y + in_mouse_y) / m_zoom;
        y = (int )t;
      }

      bool is_mono;
      int pix_r, pix_g, pix_b;
      if (m_image_data_ptr->get_pixel_value(x, y,
              &is_mono, &pix_r, &pix_g, &pix_b) == false)
        is_valid = false;
      //
      m_mouse_info_x = in_mouse_x;
      m_mouse_info_y = in_mouse_y;
      m_mouse_info_updated = true;
      invoke_mouse_info_updated_handlers(is_valid,
             x, y,
             is_mono, pix_r, pix_g, pix_b);
    }

  private:
    // Member variables --------------------------------------------------------
    Glib::Property<Glib::RefPtr<Gtk::Adjustment>> m_h_adjustment;
    Glib::Property<Glib::RefPtr<Gtk::Adjustment>> m_v_adjustment;
    Glib::Property<Gtk::ScrollablePolicy> m_h_scroll_policy;
    Glib::Property<Gtk::ScrollablePolicy> m_v_scroll_policy;
    sigc::connection m_h_adjustment_connection;
    sigc::connection m_v_adjustment_connection;

    double m_window_x, m_window_y, m_window_width, m_window_height;
    double m_width, m_height;
    double m_org_width, m_org_height;
    double m_mouse_x, m_mouse_y;
    bool  m_mouse_info_updated;
    double m_mouse_info_x, m_mouse_info_y;
    double m_offset_x, m_offset_y;
    double m_offset_x_max, m_offset_y_max;
    double m_offset_x_org, m_offset_y_org;
    double m_zoom;
    bool m_zoom_best_fit;
    bool m_mouse_l_pressed;
    bool m_adjustments_modified;

    PerfCounter m_fps_counter;
    PerfCounter m_fps_sum_counter;
    double m_fps;
    double m_fps_sum;
    int m_fps_sum_num;

    Data *m_image_data_ptr;
    bool m_is_image_size_changed;

    Colormap::ColormapIndex m_colormap_index;
    uint8_t m_colormap[IM_VIEW_COLORMAP_DATA_SIZE] = {};

    Glib::RefPtr<Gdk::Window> m_window;
    Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;

    std::vector<UpdateHandlerInterface *>  m_update_handlers;

    friend class MainWindow;
  };

  // ---------------------------------------------------------------------------
  //  MainWindow class
  // ---------------------------------------------------------------------------
  class MainWindow : public Gtk::Window, protected UpdateHandlerInterface
  {
  public:
    // -------------------------------------------------------------------------
    // MainWindow destructor
    // -------------------------------------------------------------------------
    ~MainWindow() override
    {
      SHL_DBG_OUT("MainWindow was deleted");
    }

  protected:
    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // on_zoom_best_fit
    // -------------------------------------------------------------------------
    void on_zoom_best_fit()
    {
      Glib::VariantBase variant = m_base_fit_action->get_state_variant();
      if (variant.is_of_type(Glib::VARIANT_TYPE_BOOL) == false)
        return;
      Glib::Variant<bool> value;
      value = Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(variant);
      if (value.get())
      {
        m_base_fit_action->set_state(Glib::Variant<bool>::create(false));
        m_image_view.set_zoom_best_fit(false);
      }
      else
      {
        m_base_fit_action->set_state(Glib::Variant<bool>::create(true));
        m_image_view.set_zoom_best_fit(true);
      }
    }
    // -------------------------------------------------------------------------
    // on_zoom
    // -------------------------------------------------------------------------
    void on_zoom(const Glib::VariantBase &parameter)
    {
      if (parameter.is_of_type(Glib::VARIANT_TYPE_INT32) == false)
        return;
      //
      m_image_view.set_zoom_best_fit(false);
      Glib::Variant<gint32> value;
      value = Glib::VariantBase::cast_dynamic<Glib::Variant<gint32>>(parameter);
      m_image_view.set_zoom(value.get() / 100.0);
    }
    // -------------------------------------------------------------------------
    // on_zooom_entry_key_release
    // -------------------------------------------------------------------------
    bool on_zooom_entry_key_release(GdkEventKey* event)
    {
      if (event->type != GDK_KEY_RELEASE || event->keyval != GDK_KEY_Return)
        return false;
      Glib::ustring text = m_zoom_entry.get_text();
      char *end_ptr;
      int value = (int )strtol(text.c_str(), &end_ptr, 0);
      if (value == 0)
        value = 1;
      m_image_view.set_zoom_best_fit(false);
      m_image_view.set_zoom(value / 100.0);
      return true;
    }
    // -------------------------------------------------------------------------
    // on_menu_save
    // -------------------------------------------------------------------------
    void on_menu_save()
    {
      char buf[512];
      sprintf(buf, "%s-%d.bmp", m_title.get_text().c_str(), m_file_save_index);
      m_image_view.save_pixbuf(buf, "bmp");
      m_file_save_index++;
    }
    // -------------------------------------------------------------------------
    // on_menu_save_as
    // -------------------------------------------------------------------------
    void on_menu_save_as()
    {
      typedef struct
      {
        const char *name;
        const char *mime_type;
        const char *save_type;
      } type_data;
      const type_data type_table[] = {
          {"TIFF (*.tiff)", "image/tiff", "tiff"},
          {"PNG (*.png)", "image/png", "png"},
          {"JPEG (*.jpeg)", "image/jpeg", "jpeg"},
          {"Windows icon (*.ico)", "image/ico", "ico"},
          {"BMP (*.bmp)", "image/bmp", "bmp"},
          {"RAW (*.raw)", "image/raw", "raw"},
          {nullptr, nullptr, nullptr},
      };
      //
      Gtk::FileChooserDialog dialog("Save As...",
                                    Gtk::FILE_CHOOSER_ACTION_SAVE);
      dialog.set_transient_for(*this);
      dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
      dialog.add_button("Save", Gtk::RESPONSE_OK);
      Glib::RefPtr<Gtk::FileFilter> filter;
      filter = Gtk::FileFilter::create();
      filter->set_name("All files");
      filter->add_pattern("*.*");
      dialog.add_filter(filter);
      filter = Gtk::FileFilter::create();
      filter->set_name("Supported image files");
      for (int i = 0; type_table[i].name != nullptr; i++)
        filter->add_mime_type(type_table[i].mime_type);
      dialog.add_filter(filter);
      for (int i = 0; type_table[i].name != nullptr; i++)
      {
        filter = Gtk::FileFilter::create();
        filter->set_name(type_table[i].name);
        filter->add_mime_type(type_table[i].mime_type);
        dialog.add_filter(filter);
      }
      if (dialog.run() != Gtk::RESPONSE_OK)
        return;
      //
      bool result;
      Glib::RefPtr< Gio::File > file = dialog.get_file();
      std::string mime_type = Gio::content_type_guess(file->get_basename(), std::string(), result).c_str();
      std::string save_type = "raw";
      for (int i = 0; type_table[i].name != nullptr; i++)
        if (mime_type == type_table[i].mime_type)
          save_type = type_table[i].save_type;
      if (save_type == "raw")
        m_image_view.save_as_raw(file->get_path());
      else
        m_image_view.save_pixbuf(file->get_path(), save_type.c_str());
    }
    // -------------------------------------------------------------------------
    // on_menu_about
    // -------------------------------------------------------------------------
    void on_menu_about()
    {
      Gtk::AboutDialog  dialog;
      dialog.set_program_name("ImageWindows-GTK");
      dialog.set_version(SHL_IMAGE_WINDOW_GTK_BASE_VERSION);
      dialog.set_copyright("Copyright (c) 2021-2024 Dairoku Sekiguchi");
      dialog.set_transient_for(*this);
      dialog.run();
    }
    // -------------------------------------------------------------------------
    // on_button_zoom_entry
    // -------------------------------------------------------------------------
    void on_button_zoom_entry(Gtk::EntryIconPosition icon_position, const GdkEventButton* event)
    {
      if (!m_zoom_menu->get_attach_widget())
        m_zoom_menu->attach_to_widget(*this);
      m_zoom_menu->popup_at_widget(&m_zoom_entry, Gdk::GRAVITY_SOUTH_WEST, Gdk::GRAVITY_NORTH_WEST, nullptr);
    }
    // -------------------------------------------------------------------------
    // on_button_zoom_out
    // -------------------------------------------------------------------------
    void on_button_zoom_out()
    {
      gint32 v = (gint32 )(m_image_view.get_zoom() * 100.0);
      const gint32 *zoom_list = get_zoom_list();
      int index = 0;
      while (zoom_list[index] != 0)
        index++;
      index--;
      while (index != 0)
      {
        if (v > zoom_list[index])
          break;
        index--;
      }
      m_image_view.set_zoom_best_fit(false);
      m_image_view.set_zoom(zoom_list[index] / 100.0);
    }
    // -------------------------------------------------------------------------
    // on_button_zoom_in
    // -------------------------------------------------------------------------
    void on_button_zoom_in()
    {
      gint32 v = (gint32 )(m_image_view.get_zoom() * 100.0);
      const gint32 *zoom_list = get_zoom_list();
      while (*zoom_list != 0)
      {
        if (*zoom_list > v)
          break;
        zoom_list++;
      }
      if (*zoom_list == 0)
        zoom_list--;
      m_image_view.set_zoom_best_fit(false);
      m_image_view.set_zoom(*zoom_list / 100.0);
    }
    // -------------------------------------------------------------------------
    // on_button_full
    // -------------------------------------------------------------------------
    void on_button_full()
    {
      //m_status_bar.hide();
      this->fullscreen();
    }
    // -------------------------------------------------------------------------
    // on_key_release
    // -------------------------------------------------------------------------
    bool on_key_release(GdkEventKey* event)
    {
      if (event->type != GDK_KEY_RELEASE || event->keyval != GDK_KEY_Escape)
        return false;
      //m_status_bar.show();
      this->unfullscreen();
      return true;
    }
    // -------------------------------------------------------------------------
    // view_zoom_updated
    // -------------------------------------------------------------------------
    void view_zoom_updated(double in_zoom, bool in_best_fit) override
    {
      char buf[256];
      sprintf(buf, "%d%%", (int )(in_zoom * 100.0));
      m_zoom_entry.set_text(buf);
      update_status_left();
      //
      Glib::VariantBase variant = m_base_fit_action->get_state_variant();
      if (variant.is_of_type(Glib::VARIANT_TYPE_BOOL) == false)
        return;
      Glib::Variant<bool> value;
      value = Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(variant);
      if (value.get() == in_best_fit)
        return;
      m_base_fit_action->set_state(Glib::Variant<bool>::create(in_best_fit));
    }
    // -------------------------------------------------------------------------
    // view_image_info_updated
    // -------------------------------------------------------------------------
    void view_image_info_updated(bool in_is_valid_image_info,
                                int in_image_width, int in_image_height,
                                bool in_is_image_mono) override
    {
      if (m_image_view.get_image_data() == nullptr)
        return;
      update_status_left(in_is_valid_image_info, in_image_width, in_image_height,
                         in_is_image_mono, m_image_view.get_zoom());
    }
    // -------------------------------------------------------------------------
    // view_mouse_info_updated
    // -------------------------------------------------------------------------
    void view_mouse_info_updated(bool in_is_valid_mouse_info,
                                int in_mouse_x, int in_mouse_y,
                                bool in_is_mouse_mono,
                                int in_mouse_r, int in_mouse_g, int in_mouse_b) override
    {
      update_status_center(in_is_valid_mouse_info,
                           in_mouse_x, in_mouse_y,
                           in_is_mouse_mono,
                           in_mouse_r, in_mouse_g, in_mouse_b);
    }
    // -------------------------------------------------------------------------
    // view_frame_info_updated
    // -------------------------------------------------------------------------
    void view_frame_info_updated(bool in_is_valid_frame_info,
                                unsigned in_frame_count, double in_fps) override
    {
      update_status_right(in_is_valid_frame_info, in_frame_count, in_fps);
    }
    // -------------------------------------------------------------------------
    // MainWindow constructor
    // -------------------------------------------------------------------------
    MainWindow(Data *in_image_data_ptr, const char *in_title) :
            m_header_left_box(Gtk::Orientation::ORIENTATION_HORIZONTAL),
            m_header_right_box(Gtk::Orientation::ORIENTATION_HORIZONTAL),
            m_box(Gtk::Orientation::ORIENTATION_VERTICAL, 0),
            m_status_box(Gtk::Orientation::ORIENTATION_HORIZONTAL, 0)
    {
      m_file_save_index = 0;
      //
      m_zoom_out_button.set_image_from_icon_name("zoom-out-symbolic");
      m_zoom_out_button.signal_clicked().connect(
              sigc::mem_fun(*this, &MainWindow::on_button_zoom_out));
      m_zoom_entry.set_input_purpose(Gtk::INPUT_PURPOSE_DIGITS);
      m_zoom_entry.set_max_length(5);
      m_zoom_entry.set_width_chars(9);
      m_zoom_entry.set_icon_from_icon_name("go-down-symbolic", Gtk::ENTRY_ICON_SECONDARY);
      m_zoom_entry.set_alignment(1);
      m_zoom_entry.signal_icon_press().connect(
              sigc::mem_fun(*this, &MainWindow::on_button_zoom_entry));
      m_zoom_entry.signal_key_release_event().connect(
              sigc::mem_fun(*this, &MainWindow::on_zooom_entry_key_release));
      m_zoom_in_button.set_image_from_icon_name("zoom-in-symbolic");
      m_zoom_in_button.signal_clicked().connect(
              sigc::mem_fun(*this, &MainWindow::on_button_zoom_in));
      m_header_left_box.pack_start(m_zoom_out_button);
      m_header_left_box.add(m_zoom_entry);
      m_header_left_box.pack_end(m_zoom_in_button);
      //
      m_action_group = Gio::SimpleActionGroup::create();
      m_action_group->add_action("save",sigc::mem_fun(*this, &MainWindow::on_menu_save));
      m_action_group->add_action("save_as",sigc::mem_fun(*this, &MainWindow::on_menu_save_as));
      m_action_group->add_action("about",sigc::mem_fun(*this, &MainWindow::on_menu_about));
      m_base_fit_action = m_action_group->add_action_bool("best_fit",
                                                sigc::mem_fun(*this, &MainWindow::on_zoom_best_fit));
      m_action_group->add_action_with_parameter("zoom", Glib::VARIANT_TYPE_INT32,
                                                sigc::mem_fun(*this, &MainWindow::on_zoom));
      insert_action_group("main", m_action_group);
      //
      m_gio_menu = Gio::Menu::create();
      m_gio_menu->append_item(Gio::MenuItem::create("Save", "main.save"));
      m_gio_menu->append_item(Gio::MenuItem::create("Save As...", "main.save_as"));
      m_gio_menu->append_item(Gio::MenuItem::create("About", "main.about"));
      m_menu_button.set_menu_model(m_gio_menu);
      //
      Glib::RefPtr<Gio::MenuItem> item;
      m_gio_zoom_menu = Gio::Menu::create();
      m_gio_zoom_menu->append_item(Gio::MenuItem::create("Best Fit", "main.best_fit"));
      const gint32 *zoom_list = get_zoom_list();
      while (*zoom_list != 0)
      {
        char label_buf[256];
        sprintf(label_buf, "Zoom %d%%", *zoom_list);
        Glib::RefPtr<Gio::MenuItem> item = Gio::MenuItem::create(label_buf, "main.zoom");
        item->set_action_and_target("main.zoom", Glib::Variant<gint32>::create(*zoom_list));
        m_gio_zoom_menu->append_item(item);
        zoom_list++;
      }
      m_zoom_menu = Gtk::manage(new Gtk::Menu(m_gio_zoom_menu));
      //
      m_title.set_label(in_title);
      m_full_button.set_image_from_icon_name("view-fullscreen-symbolic");
      m_full_button.signal_clicked().connect(
              sigc::mem_fun(*this, &MainWindow::on_button_full));
      m_menu_button.set_use_popover(true);
      m_menu_button.set_image_from_icon_name("open-menu-symbolic");
      m_header_right_box.pack_start(m_full_button);
      m_header_right_box.pack_end(m_menu_button);
      //
      this->set_titlebar(m_header);
      m_header.set_show_close_button(true);
      m_header.set_custom_title(m_title);
      m_header.pack_start(m_header_left_box);
      m_header.pack_end(m_header_right_box);
      //
      update_status_left(false, 0, 0, false, 0, true);
      update_status_center(false, 0, 0, false, 0, 0, 0, true);
      update_status_right(false, 0, 0, true);
      m_status_left.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_FILL);
      m_status_center.set_alignment(Gtk::ALIGN_CENTER, Gtk::ALIGN_FILL);
      m_status_right.set_alignment(Gtk::ALIGN_END, Gtk::ALIGN_FILL);
      m_status_left.set_margin_left(5);
      m_status_right.set_margin_right(5);
      m_status_box.pack_start(m_status_left, true, true, 0);
      m_status_box.pack_start(m_status_center, true, true, 0);
      m_status_box.pack_start(m_status_right, true, true, 0);
      //
      this->add(m_box);
      m_scr_win.add(m_image_view);
      m_box.pack_start(m_scr_win, true, true, 0);
      m_box.pack_start(m_status_box, false, true, 0);
      //
      m_image_view.add_update_handler(this);
      m_image_view.set_image_data(in_image_data_ptr);
      view_zoom_updated(m_image_view.get_zoom(), false);
      //
      this->signal_key_release_event().connect(
              sigc::mem_fun(*this, &MainWindow::on_key_release));
      //
      resize(300, 300);
      show_all_children();
    }

    // Member functions ----------------------------------------------------------
    // ---------------------------------------------------------------------------
    // update
    // ---------------------------------------------------------------------------
    void update()
    {
      //m_image_view.update_widget();
      m_image_view.queue_draw();
    }

  private:
    // member variables --------------------------------------------------------
    Glib::RefPtr<Gio::SimpleActionGroup> m_action_group;
    Glib::RefPtr<Gio::Menu> m_gio_menu;
    Glib::RefPtr<Gio::Menu> m_gio_zoom_menu;
    Glib::RefPtr<Gio::SimpleAction> m_base_fit_action;

    Gtk::Menu *m_zoom_menu;

    Gtk::Button m_zoom_in_button;
    Gtk::Entry m_zoom_entry;
    Gtk::Button m_zoom_out_button;
    Gtk::Box m_header_left_box;
    Gtk::Label m_title;
    Gtk::Button m_full_button;
    Gtk::MenuButton m_menu_button;
    Gtk::Box m_header_right_box;
    Gtk::HeaderBar m_header;
    Gtk::Box m_box;
    Gtk::ScrolledWindow m_scr_win;
    Gtk::Box  m_status_box;
    Gtk::Label m_status_left;
    Gtk::Label m_status_center;
    Gtk::Label m_status_right;

    shl::gtk::image::View m_image_view;

    int m_file_save_index;

    // status displayed values
    bool m_status_is_valid_image_info;
    int m_status_image_width;
    int m_status_image_height;
    bool m_status_is_image_mono;
    double m_status_image_zoom;
    bool m_status_is_valid_mouse_info;
    int m_status_mouse_x;
    int m_status_mouse_y;
    bool m_status_is_mouse_mono;
    int m_status_mouse_r;
    int m_status_mouse_g;
    int m_status_mouse_b;
    bool m_is_status_valid_frame_info;
    unsigned int m_status_frame_count;
    double m_status_image_fps;

    friend class shl::gtk::ImageWindow;

    // Member functions ----------------------------------------------------------
    // -------------------------------------------------------------------------
    // update_status_left
    // -------------------------------------------------------------------------
    void update_status_left()
    {
      if (m_image_view.get_image_data() == nullptr)
        return;
      update_status_left(m_image_view.get_image_data()->is_valid(),
                         m_image_view.get_image_data()->get_width(),
                         m_image_view.get_image_data()->get_height(),
                         m_image_view.get_image_data()->is_mono(),
                         m_image_view.get_zoom());
    }
    // -------------------------------------------------------------------------
    // update_status_left
    // -------------------------------------------------------------------------
    void update_status_left(bool in_is_valid_image_info,
                            int in_image_width, int in_image_height,
                            bool in_is_image_mono, double in_image_zoom,
                            bool in_force_update = false)
    {
      if (in_is_valid_image_info == false &&
          m_status_is_valid_image_info == false &&
          in_force_update == false)
      {
        return;
      }
      if (m_status_is_valid_image_info == in_is_valid_image_info &&
          m_status_image_width == in_image_width &&
          m_status_image_height == in_image_height &&
          m_status_is_image_mono == in_is_image_mono &&
          m_status_image_zoom == in_image_zoom &&
          in_force_update == false)
      {
        return;
      }
      m_status_is_valid_image_info = in_is_valid_image_info;
      m_status_image_width = in_image_width;
      m_status_image_height = in_image_height;
      m_status_is_image_mono = in_is_image_mono;
      m_status_image_zoom = in_image_zoom;

      char buf[256];
      const char *formatStr[] = {"RGB8", "MONO8"};
      int formatIndex = 0;
      if (in_is_image_mono)
        formatIndex = 1;
      if (m_status_is_valid_image_info == false)
      {
        buf[0] = 0;
      }
      else
      {
        sprintf(buf, "%s  %d x %d pixels %d%%",
                formatStr[formatIndex],
                m_status_image_width,
                m_status_image_height,
                (int) (m_status_image_zoom * 100.0));
      }
      m_status_left.set_text(buf);
    }
    // -------------------------------------------------------------------------
    // update_status_center
    // -------------------------------------------------------------------------
    void update_status_center(bool in_is_valid_mouse_info,
            int in_mouse_x, int in_mouse_y,
            bool in_is_mouse_mono,
            int in_mouse_r, int in_mouse_g, int in_mouse_b,
            bool in_force_update = false)
    {
      if (in_is_valid_mouse_info == false &&
          m_status_is_valid_mouse_info == false &&
          in_force_update == false)
      {
        return;
      }
      if (m_status_is_valid_mouse_info == in_is_valid_mouse_info &&
          m_status_mouse_x == in_mouse_x &&
          m_status_mouse_y == in_mouse_y &&
          m_status_is_image_mono == in_is_mouse_mono &&
          m_status_mouse_r == in_mouse_r &&
          m_status_mouse_g == in_mouse_g &&
          m_status_mouse_b == in_mouse_b &&
          in_force_update == false)
      {
        return;
      }
      m_status_is_valid_mouse_info = in_is_valid_mouse_info;
      m_status_mouse_x = in_mouse_x;
      m_status_mouse_y = in_mouse_y;
      m_status_is_image_mono = in_is_mouse_mono;
      m_status_mouse_r = in_mouse_r;
      m_status_mouse_g = in_mouse_g;
      m_status_mouse_b = in_mouse_b;

      char buf[256];
      if (m_status_is_valid_mouse_info == false)
      {
        buf[0] = 0;
      }
      else
      {
        if (m_status_is_image_mono)
        {
          sprintf(buf, "[%d,%d] = %d",
                  m_status_mouse_x,
                  m_status_mouse_y,
                  m_status_mouse_r);
        }
        else
        {
          sprintf(buf, "[%d,%d] = %d,%d,%d",
                  m_status_mouse_x,
                  m_status_mouse_y,
                  m_status_mouse_r,
                  m_status_mouse_g,
                  m_status_mouse_b);
        }
      }
      m_status_center.set_text(buf);
    }
    // -------------------------------------------------------------------------
    // update_status_right
    // -------------------------------------------------------------------------
    void update_status_right(bool in_is_valid_frame_info,
            unsigned int in_frame_count, double in_fps,
            bool in_force_update = false)
    {
      if (in_is_valid_frame_info == false &&
          m_is_status_valid_frame_info == false &&
          in_force_update == false)
      {
        return;
      }
      if (m_is_status_valid_frame_info == in_is_valid_frame_info &&
          m_status_frame_count == in_frame_count &&
          m_status_image_fps == in_fps &&
          in_force_update == false)
      {
        return;
      }
      m_is_status_valid_frame_info = in_is_valid_frame_info;
      m_status_frame_count = in_frame_count;
      m_status_image_fps = in_fps;

      char buf[256];
      if (m_is_status_valid_frame_info == false)
      {
        buf[0] = 0;
      }
      else
      {
        sprintf(buf, "%d    %.2ffps",
                m_status_frame_count,
                m_status_image_fps);
      }
      m_status_right.set_text(buf);
    }
    // -------------------------------------------------------------------------
    // get_zoom_list
    // -------------------------------------------------------------------------
    const gint32 *get_zoom_list()
    {
      static const gint32 zoom_list[] = {33,50,100,
                                         133,200,500,1000,
                                         1500,2000,0};
      return zoom_list;
    }
  };
};

  // ---------------------------------------------------------------------------
  //  ImageWindow class
  // ---------------------------------------------------------------------------
  class ImageWindow :
      public shl::gtk::image::Data,
      public shl::gtk::base::WindowBase
  {
  public:
    // constructors and destructor ---------------------------------------------
    // -------------------------------------------------------------------------
    // ImageWindow
    // -------------------------------------------------------------------------
    ImageWindow()
    {
      m_window = nullptr;
    }
    // -------------------------------------------------------------------------
    // ~ImageWindow
    // -------------------------------------------------------------------------
    ~ImageWindow() override
    {
      SHL_DBG_OUT("ImageWindow was deleted");
    }

  protected:
    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // create_window_object
    // -------------------------------------------------------------------------
    Gtk::Window *create_window_object(const char *in_title) override
    {
      m_window = new shl::gtk::image::MainWindow(this, in_title);
      return m_window;
    }
    // -------------------------------------------------------------------------
    // get_window_object
    // -------------------------------------------------------------------------
    Gtk::Window *get_window_object() override
    {
      return m_window;
    }
    // -------------------------------------------------------------------------
    // delete_window_object
    // -------------------------------------------------------------------------
    void delete_window_object() override
    {
      delete m_window;
      m_window = nullptr;
    }
    // -------------------------------------------------------------------------
    // is_window_object_null
    // -------------------------------------------------------------------------
    bool is_window_object_null() override
    {
      if (m_window == nullptr)
        return true;
      return false;
    }
    // -------------------------------------------------------------------------
    // get_default_window_title
    // -------------------------------------------------------------------------
    const char *get_default_window_title() override
    {
      return "ImageWindow";
    }
    // -------------------------------------------------------------------------
    // update_window
    // -------------------------------------------------------------------------
    void update_window() override
    {
      if (m_window == nullptr)
        return;
      // The following will call WindowData::update() at the end of the call chain
      mark_as_modified();
      m_window->update();
    }

  private:
    shl::gtk::image::MainWindow *m_window;
  };
} // namespace shl::gtk

#endif  // #ifdef IMAGE_WINDOW_GTK_H_
