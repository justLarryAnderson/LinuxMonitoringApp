#include "MainWindow.h"
#include <gtkmm/application.h>

int main(int argc, char *argv[]) {
    auto app = Gtk::Application::create(argc, argv, "org.linux.systemmonitor");
    
    MainWindow window;
    window.show_all();
    
    return app->run(window);
}