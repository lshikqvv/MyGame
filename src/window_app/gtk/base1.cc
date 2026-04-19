#include <gtkmm.h>
// g++ base1.cc -o base1 `pkg-config --cflags --libs gtkmm-3.0`

class SampleWindow : public Gtk::Window
{
public:
    SampleWindow(int width, int height);
    virtual ~SampleWindow() = default;
};

SampleWindow::SampleWindow(int width, int height)
{
    set_title("title");
    set_default_size(width, height);
}

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create(argc, argv, "work.jitaku.gtkmm.sample");
    SampleWindow sample_window(320, 240);
    return app->run(sample_window);
}