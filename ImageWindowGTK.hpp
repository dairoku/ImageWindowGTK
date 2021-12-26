// =============================================================================
//  ImageWindow.hpp
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
  \file     ImageWindow.h
  \author   Dairoku Sekiguchi
  \version  1.0.0
  \date     2021/11/14
*/
#ifndef IMAGE_WINDOW_GTK_H_
#define IMAGE_WINDOW_GTK_H_
#define SHL_IMAGE_WINDOW_GTK_BASE_VERSION   1

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <cstring>
#include <gtkmm.h>


// Namespace -------------------------------------------------------------------
namespace shl::gtk
{
// -----------------------------------------------------------------------------
//	shl base gtk classes
// -----------------------------------------------------------------------------
// Macros ----------------------------------------------------------------------
#ifndef SHL_BASE_GTK_CLASS_
#define SHL_BASE_GTK_CLASS_
#define SHL_BASE_GTK_CLASS_VERSION  SHL_IMAGE_WINDOW_GTK_BASE_VERSION
//
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

  // ---------------------------------------------------------------------------
  //	BaseWindowFactoryInterface class
  // ---------------------------------------------------------------------------
  class BaseWindowFactoryInterface
  {
  public:
    virtual Gtk::Window *create_window() = 0;
    virtual void wait_new_window() = 0;
    virtual Gtk::Window *get_window() = 0;
    virtual void delete_window() = 0;
    virtual bool is_window_deleted() = 0;
    virtual void wait_delete_window() = 0;
  };

  // ---------------------------------------------------------------------------
  //	BaseApp class
  // ---------------------------------------------------------------------------
  class BaseApp : public Gtk::Application
  {
  public:
    // constructors and destructor ---------------------------------------------
    // -------------------------------------------------------------------------
    // ~BaseApp
    // -------------------------------------------------------------------------
    ~BaseApp() override
    {
      SHL_DBG_OUT("BaseApp was deleted");
    }

  protected:
    // constructors and destructor ---------------------------------------------
    // -------------------------------------------------------------------------
    // BaseApp
    // -------------------------------------------------------------------------
    BaseApp() :
            Gtk::Application("org.gtkmm.examples.application",
                             Gio::APPLICATION_NON_UNIQUE),
                             m_quit(false)
    {
      Glib::signal_idle().connect( sigc::mem_fun(*this, &BaseApp::on_idle) );
    }

    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // create_window
    // -------------------------------------------------------------------------
    // [Note] this function will be called from another thread
    //
    void create_window(BaseWindowFactoryInterface *in_interface)
    {
      std::lock_guard<std::mutex> lock(m_create_win_queue_mutex);
      m_create_win_queue.push(in_interface);
    }
    // -------------------------------------------------------------------------
    // delete_window
    // -------------------------------------------------------------------------
    // [Note] this function will be called from another thread
    //
    void delete_window(BaseWindowFactoryInterface *in_interface)
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
        BaseWindowFactoryInterface *interface = m_create_win_queue.front();
        auto it = std::find(m_window_list.begin(), m_window_list.end(), interface);
        if (it == m_window_list.end())
        {
          Gtk::Window *win = interface->create_window();
          add_window(*win);
          win->signal_hide().connect(sigc::bind<Gtk::Window *>(
                  sigc::mem_fun(*this,
                                &BaseApp::on_hide_window), win));
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
        BaseWindowFactoryInterface *interface = m_delete_win_queue.front();
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
    std::queue<BaseWindowFactoryInterface *> m_create_win_queue;
    std::mutex m_delete_win_queue_mutex;
    std::queue<BaseWindowFactoryInterface *> m_delete_win_queue;
    std::vector<BaseWindowFactoryInterface *> m_window_list;
    std::condition_variable m_window_cond;
    std::mutex  m_window_mutex;
    bool m_quit;

    // friend classes ----------------------------------------------------------
    friend class BaseAppRunner;
  };

  // ---------------------------------------------------------------------------
  //	BaseAppRunner class
  // ---------------------------------------------------------------------------
  class BaseAppRunner
  {
  public:
    // constructors and destructor ---------------------------------------------
    // -------------------------------------------------------------------------
    // ~BaseAppRunner
    // -------------------------------------------------------------------------
    virtual ~BaseAppRunner()
    {
      if (m_app != nullptr)
        m_app->post_quit_app();
      if (m_thread != nullptr)
      {
        m_thread->join();
        delete m_thread;
        delete m_app;
      }
      SHL_DBG_OUT("BaseAppRunner was deleted");
    }

  protected:
    // constructors and destructor ---------------------------------------------
    // -------------------------------------------------------------------------
    // BaseAppRunner
    // -------------------------------------------------------------------------
    BaseAppRunner() :
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
    // is_window_close_all
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
    void create_window(BaseWindowFactoryInterface *in_interface)
    {
      std::lock_guard<std::mutex> lock(m_function_call_mutex);
      if (m_app == nullptr)
      {
        m_app = new BaseApp();
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
    void delete_window(BaseWindowFactoryInterface *in_interface)
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
    static BaseAppRunner *get_runner()
    {
      static BaseAppRunner s_runner;
      return &s_runner;
    }

  private:
    // member variables --------------------------------------------------------
    BaseApp *m_app;
    std::thread *m_thread;
    std::mutex m_function_call_mutex;

    // static functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // thread_func
    // -------------------------------------------------------------------------
    static void thread_func(BaseAppRunner *in_obj)
    {
      SHL_TRACE_OUT("thread started");
      in_obj->m_app->run();
      SHL_TRACE_OUT("thread ended");
    }

    // friend classes ----------------------------------------------------------
    friend class BaseObject;
    friend class ImageWindow;
  };

  // ---------------------------------------------------------------------------
  //	BaseObject class
  // ---------------------------------------------------------------------------
  class BaseObject : protected BaseWindowFactoryInterface
  {
  public:
    // constructors and destructor ---------------------------------------------
    // -------------------------------------------------------------------------
    // ~BaseObject
    // -------------------------------------------------------------------------
    virtual ~BaseObject()
    {
      m_app_runner->delete_window(this);
    }
    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // wait_window_closed
    // -------------------------------------------------------------------------
    virtual void wait_window_closed()
    {
      wait_delete_window();
    }
    // -------------------------------------------------------------------------
    // is_window_closed
    // -------------------------------------------------------------------------
    virtual bool is_window_closed()
    {
      return is_window_deleted();
    }
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
    // BaseObject
    // -------------------------------------------------------------------------
    BaseObject()
    {
      m_app_runner = BaseAppRunner::get_runner();
    }

  private:
    BaseAppRunner *m_app_runner;
  };

#else
  // If another shl file is already included, we need to check the base gtk class version
  #if SHL_BASE_GTK_CLASS_VERSION != SHL_IMAGE_WINDOW_GTK_BASE_VERSION
    #error invalid shl base class version (There is a version inconsistency between included shl files)
  #endif
  //
#endif  // SHL_BASE_GTK_CLASS_

  // ---------------------------------------------------------------------------
  //	ImageData class
  // ---------------------------------------------------------------------------
  class ImageData
  {
  public:
    // constructors and destructor ---------------------------------------------
    // -------------------------------------------------------------------------
    // ImageData
    // -------------------------------------------------------------------------
    ImageData()
    {
      m_allocated_image_buffer_ptr = nullptr;
      m_external_image_buffer_ptr = nullptr;
      m_image_buffer_size = 0;
      m_image_width = 0;
      m_image_height = 0;
      m_is_mono = false;
      m_is_image_modified = false;
    }
    // -------------------------------------------------------------------------
    // ~ImageData
    // -------------------------------------------------------------------------
    virtual ~ImageData()
    {
      delete m_allocated_image_buffer_ptr;
    }
    // Member functions --------------------------------------------------------
    bool allocate_image_buffer(int in_width, int in_height, bool in_is_mono = false)
    {
      if (in_width == 0 || in_height == 0)
      {
        cleanup_buffers();
        return false;
      }
      //
      m_external_image_buffer_ptr = nullptr;
      m_image_width = in_width;
      m_image_height = in_height;
      m_is_mono = in_is_mono;
      update_image_buffer_size();
      m_allocated_image_buffer_ptr = new uint8_t[m_image_buffer_size];
      if (m_allocated_image_buffer_ptr == nullptr)
      {
        cleanup_buffers();
        return false;
      }
      ::memset(m_allocated_image_buffer_ptr, 0, m_image_buffer_size);
      return true;
    }
    bool set_image_buffer_ptr(uint8_t *in_buffer_ptr, int in_width, int in_height, bool in_is_mono = false)
    {
      if (in_buffer_ptr == nullptr || in_width == 0 || in_height == 0)
      {
        cleanup_buffers();
        return false;
      }
      //
      if (m_allocated_image_buffer_ptr != nullptr)
      {
        delete m_allocated_image_buffer_ptr;
        m_allocated_image_buffer_ptr = nullptr;
      }
      m_external_image_buffer_ptr = in_buffer_ptr;
      m_image_width = in_width;
      m_image_height = in_height;
      m_is_mono = in_is_mono;
      update_image_buffer_size();
      m_is_image_modified = true;
      return true;
    }
    uint8_t *get_image_buffer_ptr()
    {
      if (m_allocated_image_buffer_ptr != nullptr)
        return m_allocated_image_buffer_ptr;
      return m_external_image_buffer_ptr;
    }
    bool update_image_buffer_ptr(uint8_t *in_buffer_ptr, bool in_redraw_image = true)
    {
      if (m_external_image_buffer_ptr == nullptr)
      {
        cleanup_buffers();
        return false;
      }
      //
      m_external_image_buffer_ptr = in_buffer_ptr;
      m_is_image_modified = true;
      if (in_redraw_image)
        redraw_image();
      return true;
    }
    bool update_pixbuf(bool in_force_update = false)
    {
      if (in_force_update == false && m_is_image_modified == false)
        return false;
      if (check_image_data() == false)
        return false;
      if (m_pixbuf->get_width() != m_image_width ||
          m_pixbuf->get_height() != m_image_height)
        return false;
      //
      //mActiveConverter->convert(getImageBufferPixelPtr(), m_pixbuf->get_pixels());
      m_is_image_modified = false;
      return true;
    }
    bool copy_to_image_buffer(const uint8_t *in_src_ptr, bool in_redraw_image = true)
    {
      uint8_t *buffer = get_image_buffer_ptr();
      if (in_src_ptr == nullptr || m_image_buffer_size == 0 || buffer == nullptr)
        return false;
      //
      ::memcpy(buffer, in_src_ptr, m_image_buffer_size);
      m_is_image_modified = true;
      if (in_redraw_image)
        redraw_image();
      return true;
    }
    void mark_as_image_modified()
    {
      m_is_image_modified = true;
    }
    void redraw_image(bool in_force = false)
    {
    }
    [[nodiscard]] bool is_image_modified() const
    {
      return m_is_image_modified;
    }
    [[nodiscard]] int get_image_width() const
    {
      return m_image_width;
    }
    [[nodiscard]] int get_image_height() const
    {
      return m_image_height;
    }
    [[nodiscard]] size_t get_image_buffer_size() const
    {
      return m_image_buffer_size;
    }
    [[nodiscard]] bool check_image_data() const
    {
      if (m_allocated_image_buffer_ptr == nullptr || m_external_image_buffer_ptr == nullptr)
        return false;
      if (m_image_width == 0 || m_image_height == 0 || m_image_buffer_size == 0)
        return false;
      if (!m_pixbuf)
        return false;
      return true;
    }

  protected:
    Glib::RefPtr<Gdk::Pixbuf>  m_pixbuf;

    // Member functions --------------------------------------------------------
    void cleanup_buffers()
    {
      if (m_allocated_image_buffer_ptr != nullptr)
      {
        delete m_allocated_image_buffer_ptr;
        m_allocated_image_buffer_ptr = nullptr;
      }
      m_external_image_buffer_ptr = nullptr;
      m_image_buffer_size = 0;
      m_image_width = 0;
      m_image_height = 0;
      m_is_mono = false;
      m_is_image_modified = false;
    }
    void update_image_buffer_size()
    {
      m_image_buffer_size = m_image_width * m_image_height;
      if (m_is_mono == false)
        m_image_buffer_size *= 3;
      //
      m_pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB , false, 8,
                                     m_image_width, m_image_height);
    }

  private:
    uint8_t *m_allocated_image_buffer_ptr;
    uint8_t *m_external_image_buffer_ptr;
    size_t m_image_buffer_size;
    int   m_image_width;
    int   m_image_height;
    bool  m_is_mono;
    bool  m_is_image_modified;

    // friend classes ----------------------------------------------------------
    friend class ImageView;
  };

  // ---------------------------------------------------------------------------
  // ImageView class
  // ---------------------------------------------------------------------------
  // Note: Order of Scrollable -> Widget is very important
  class ImageView : virtual public Gtk::Scrollable, virtual public Gtk::Widget
  {
  public:
    // Constructors and Destructor ---------------------------------------------
    // -------------------------------------------------------------------------
    // ImageView
    // -------------------------------------------------------------------------
    ImageView() :
            Glib::ObjectBase("ImageView"),
            m_h_adjustment(*this, "hadjustment"),
            m_v_adjustment(*this, "vadjustment"),
            m_h_scroll_policy(*this, "hscroll-policy", Gtk::SCROLL_NATURAL),
            m_v_scroll_policy(*this, "vscroll-policy", Gtk::SCROLL_NATURAL)
    {
      set_has_window(true);
      property_hadjustment().signal_changed().connect(sigc::mem_fun(*this, &shl::gtk::ImageView::h_adjustment_changed));
      property_vadjustment().signal_changed().connect(sigc::mem_fun(*this, &shl::gtk::ImageView::v_adjustment_changed));

      m_width = 0;
      m_height = 0;
      m_org_width = 0;
      m_org_height = 0;
      m_mouse_x = 0;
      m_mouse_y = 0;
      m_offset_x    = 0;
      m_offset_y    = 0;
      m_offset_x_max = 0;
      m_offset_y_max = 0;
      m_offset_x_org = 0;
      m_offset_y_org = 0;
      m_window_x    = 0;
      m_window_y    = 0;
      m_window_width  = 0;
      m_window_height = 0;
      m_zoom          = 1.0;
      m_mouse_l_pressed = false;
      m_adjustments_modified = false;

      m_image_data_ptr = nullptr;
      m_is_image_size_changed = false;

      add_events(Gdk::SCROLL_MASK |
                 Gdk::BUTTON_MOTION_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
                 Gdk::POINTER_MOTION_MASK);
    }
    // -------------------------------------------------------------------------
    // ~ImageView
    // -------------------------------------------------------------------------
    ~ImageView() override
    {
    }

  protected:
    // Member functions --------------------------------------------------------
    // -------------------------------------------------------------------------
    // set_image_data
    // -------------------------------------------------------------------------
    void  set_image_data(ImageData *inImageDataPtr)
    {
      m_image_data_ptr = inImageDataPtr;
      mark_as_image_size_changed();
    }
    // -------------------------------------------------------------------------
    // queue_redraw_widget
    // -------------------------------------------------------------------------
    void  queue_redraw_widget()
    {
      queue_draw();
    }
    // -------------------------------------------------------------------------
    // mark_as_image_size_changed
    // -------------------------------------------------------------------------
    void  mark_as_image_size_changed()
    {
      m_is_image_size_changed = true;
      queue_draw();
    }
    // -------------------------------------------------------------------------
    // is_image_size_changed
    // -------------------------------------------------------------------------
    bool  is_image_size_changed() const
    {
      return m_is_image_size_changed;
    }
    // -------------------------------------------------------------------------
    // update_size_using_image_data
    // -------------------------------------------------------------------------
    bool  update_size_using_image_data()
    {
      if (m_image_data_ptr == nullptr)
        return false;
      if (m_image_data_ptr->check_image_data() == false)
        return false;
      //
      m_org_width   = m_image_data_ptr->get_image_width();
      m_org_height  = m_image_data_ptr->get_image_height();
      m_width       = m_org_width;
      m_height      = m_org_height;
      configure_h_adjustment();
      configure_v_adjustment();
      return true;
    }
    // -------------------------------------------------------------------------
    // on_draw
    // -------------------------------------------------------------------------
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override
    {
      if (m_image_data_ptr == nullptr)
        return false;
      if (m_image_data_ptr->check_image_data() == false)
        return false;
      if (m_is_image_size_changed)
      {
        update_size_using_image_data();
        m_is_image_size_changed = false;
      }
      if (m_image_data_ptr->update_pixbuf() == false)
        return false;
      //
      double x = 0, y = 0;
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
        cr->set_identity_matrix();
        cr->translate(x, y);
        cr->scale(m_zoom, m_zoom);
        Gdk::Cairo::set_source_pixbuf(cr, m_image_data_ptr->m_pixbuf, 0, 0);
        Cairo::SurfacePattern pattern(cr->get_source()->cobj());
        pattern.set_filter(Cairo::Filter::FILTER_NEAREST);
      }
      else
      {
        Gdk::Cairo::set_source_pixbuf(cr,
                                      m_image_data_ptr->m_pixbuf->scale_simple(
                                              m_width, m_height,
                                              Gdk::INTERP_NEAREST),
                                      x, y);
      }
      cr->paint();
      return true;
    }
    // -------------------------------------------------------------------------
    // on_realize
    // -------------------------------------------------------------------------
    void  on_realize() override
    {
      // Do not call base class Gtk::Widget::on_realize().
      // It's intended only for widgets that set_has_window(false).
      set_realized();

      if(!m_window)
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
    void  on_unrealize() override
    {
      m_window.reset();

      //Call base class:
      Gtk::Widget::on_unrealize();
    }
    // -------------------------------------------------------------------------
    // on_button_press_event
    // -------------------------------------------------------------------------
    bool  on_button_press_event(GdkEventButton* button_event) override
    {
      m_mouse_l_pressed = true;
      m_mouse_x = button_event->x;
      m_mouse_y = button_event->y;
      m_offset_x_org= m_offset_x;
      m_offset_y_org = m_offset_y;
      return true;
    }
    // -------------------------------------------------------------------------
    // on_button_press_event
    // -------------------------------------------------------------------------
    bool  on_motion_notify_event(GdkEventMotion* motion_event) override
    {
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
    // on_button_release_event
    // -------------------------------------------------------------------------
    bool  on_button_release_event(GdkEventButton* release_event) override
    {
      m_mouse_l_pressed = false;
      return true;
    }
    // -------------------------------------------------------------------------
    // on_scroll_event
    // -------------------------------------------------------------------------
    bool  on_scroll_event(GdkEventScroll *event) override
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
      double prev_zoom = m_zoom;

      if (event->direction == 0)
        v = 0.02;
      else
        v = -0.02;
      v = log10(m_zoom) + v;
      m_zoom = pow(10, v);
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
        v = v + (m_offset_x + event->x) / prev_zoom;
        v = v * m_zoom - event->x;
        m_offset_x = v;
        m_offset_x_max = m_width - m_window_width;
        if (m_offset_x > m_offset_x_max)
          m_offset_x = m_offset_x_max;
        if (m_offset_x < 0)
          m_offset_x = 0;
      }

      if (m_height <= m_window_height)
        m_offset_y = 0;
      else
      {
        if (prev_height <= m_window_height)
          v = -1 * (m_window_height - prev_height) / 2.0;
        else
          v = 0;
        v = (m_offset_y + event->y) / prev_zoom;
        v = v * m_zoom - event->y;
        m_offset_y = v;
        m_offset_y_max = m_height - m_window_height;
        if (m_offset_y > m_offset_y_max)
          m_offset_y = m_offset_y_max;
        if (m_offset_y < 0)
          m_offset_y = 0;
      }

      configure_h_adjustment();
      configure_v_adjustment();
      queue_draw();

      return true;
    }
    // -------------------------------------------------------------------------
    // on_size_allocate
    // -------------------------------------------------------------------------
    void  on_size_allocate(Gtk::Allocation& allocation) override
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

      if(m_window)
      {
        m_window->move_resize(m_window_x, m_window_y, m_window_width, m_window_height);
        configure_h_adjustment();
        configure_v_adjustment();
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
      m_h_adjustment_connection = v->signal_value_changed().connect(sigc::mem_fun(*this,
                                                                                  &shl::gtk::ImageView::adjustment_value_changed));
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
      m_v_adjustment_connection = v->signal_value_changed().connect(sigc::mem_fun(*this,
                                                                                  &shl::gtk::ImageView::adjustment_value_changed));
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
      }
      else
      {
        m_offset_x_max = m_width - m_window_width;
        if (m_offset_x > m_offset_x_max)
          m_offset_x = m_offset_x_max;
        v->set_upper(m_offset_x_max);
        v->set_value(m_offset_x);
        v->set_step_increment(1);
        v->set_page_size(10);
        m_adjustments_modified = true;
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
      }
      else
      {
        m_offset_y_max = m_height - m_window_height;
        if (m_offset_y > m_offset_y_max)
          m_offset_y = m_offset_y_max;
        v->set_upper(m_offset_y_max);
        v->set_value(m_offset_y);
        v->set_step_increment(1);
        v->set_page_size(10);
        m_adjustments_modified = true;
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
    double m_offset_x, m_offset_y;
    double m_offset_x_max, m_offset_y_max;
    double m_offset_x_org, m_offset_y_org;
    double m_zoom;
    bool  m_mouse_l_pressed;
    bool  m_adjustments_modified;

    ImageData *m_image_data_ptr;
    bool  m_is_image_size_changed;

    Glib::RefPtr<Gdk::Pixbuf>  m_pixbuf;
    Glib::RefPtr<Gdk::Window> m_window;
  };

  // ---------------------------------------------------------------------------
  //	ImageMainWindow class
  // ---------------------------------------------------------------------------
  class ImageMainWindow : public Gtk::Window
  {
  public:
    // constructors and destructor ---------------------------------------------
    // -------------------------------------------------------------------------
    // ImageMainWindow
    // -------------------------------------------------------------------------
    ImageMainWindow() :
      m_box(Gtk::Orientation::ORIENTATION_VERTICAL , 0),
      m_status_bar("Statusbar")
    {
      m_image_data_ptr = nullptr;
      m_scr_win.add(mImageView);
      m_box.pack_start(m_scr_win, true, true, 0);
      m_box.pack_end(m_status_bar, false, false, 0);
      add(m_box);
      resize(300, 300);
      show_all_children();
    }
    // -------------------------------------------------------------------------
    // ~ImageMainWindow
    // -------------------------------------------------------------------------
    ~ImageMainWindow() override
    {
      SHL_DBG_OUT("ImageMainWindow was deleted");
    }
    // Member functions ----------------------------------------------------------
    // ---------------------------------------------------------------------------
    // set_image_data
    // ---------------------------------------------------------------------------
    void set_image_data(ImageData *in_image_data_ptr)
    {
      m_image_data_ptr = in_image_data_ptr;
    }

    Gtk::Box            m_box;
    Gtk::ScrolledWindow m_scr_win;
    Gtk::Label          m_status_bar;
    shl::gtk::ImageView   mImageView;
    //
    ImageData   *m_image_data_ptr;

    friend class ImageWindow;
  };

  // ---------------------------------------------------------------------------
  //	ImageWindow class
  // ---------------------------------------------------------------------------
  class ImageWindow : public ImageData, public BaseObject
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
    // create_window
    // -------------------------------------------------------------------------
    Gtk::Window *create_window() override
    {
      m_window = new ImageMainWindow();
      m_window->set_image_data(this);
      m_new_window_cond.notify_all();
      return m_window;
    }
    void wait_new_window() override
    {
      std::unique_lock<std::mutex> lock(m_new_window_mutex);
      m_new_window_cond.wait(lock);
    }
    Gtk::Window *get_window() override
    {
      return m_window;
    }
    void delete_window() override
    {
      delete m_window;
      m_window = nullptr;
      m_delete_window_cond.notify_all();
    }
    bool is_window_deleted() override
    {
      if (m_window == nullptr)
        return true;
      return false;
    }
    void wait_delete_window() override
    {
      std::unique_lock<std::mutex> lock(m_delete_window_mutex);
      m_delete_window_cond.wait(lock);
    }

  private:
    ImageMainWindow *m_window;
    std::condition_variable m_new_window_cond;
    std::mutex  m_new_window_mutex;
    std::condition_variable m_delete_window_cond;
    std::mutex  m_delete_window_mutex;
  };
} // namespace shl::gtk

#endif	// #ifdef IMAGE_WINDOW_GTK_H_
