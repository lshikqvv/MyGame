#include "Window_app.h"

using namespace window_app;

WindowApp::WindowApp(int width, int height)
{
    set_title("title");
    set_default_size(width, height);
    // fullscreen();

    m_label.set_text("Hello, World!");
    m_label.show();

    m_show_flag = true;

    Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file("resources/poker/images/start.png");
    pixbuf = pixbuf->scale_simple(240, 120, Gdk::INTERP_BILINEAR);
    m_image.set(pixbuf);
    m_image.show();

    m_button.set_label("hide");
    m_button.set_size_request(256,20);
    m_button.signal_clicked().connect(sigc::mem_fun(*this, &WindowApp::callback_button));

    // m_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
    // m_box.pack_start(m_label, Gtk::PACK_SHRINK);
    // m_box.pack_start(m_image, Gtk::PACK_SHRINK);
    // m_box.pack_start(m_button, Gtk::PACK_SHRINK);
    // m_box.show();

    // add(m_box);

    m_fixed.put(m_label, 0, 0);
    m_fixed.put(m_image, 0, 100);
    m_fixed.put(m_button, 0, 940);
    m_fixed.show();

    add(m_fixed);
}

void WindowApp::callback_button()
{
    if (m_show_flag) {
        m_show_flag = false;
        m_image.hide();
        m_button.set_label("show");
    } else {
        m_show_flag = true;
        m_image.show();
        m_button.set_label("hide");
    }
}
