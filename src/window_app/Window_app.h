#pragma once

#include <gtkmm.h>
#include <vector>

#include "Window_app_exception.h"
#include "struct/struct.h"

namespace window_app
{
    class WindowApp : public Gtk::Window
    {
    public:
        WindowApp(int width, int height);
        virtual ~WindowApp() = default;

        void callback_button();

    private:
        bool m_show_flag;
        Gtk::Label m_label;
        Gtk::Image m_image;
        Gtk::Button m_button;
        // Gtk::Box m_box;
        Gtk::Fixed m_fixed;
    };
}
