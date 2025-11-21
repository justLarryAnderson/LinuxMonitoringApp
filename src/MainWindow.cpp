#include "MainWindow.h"
#include <iostream>

MainWindow::MainWindow() :
    m_MainBox(Gtk::ORIENTATION_VERTICAL),
    m_HeaderBox(Gtk::ORIENTATION_HORIZONTAL),
    m_ContentBox(Gtk::ORIENTATION_HORIZONTAL),
    m_CommandBox(Gtk::ORIENTATION_HORIZONTAL),
    m_TitleLabel("Системный Монитор Процессов"),
    m_RefreshButton("Обновить"),
    m_StartCommandButton("Выполнить команду"),
    m_StopCommandButton("Стоп"),
    m_HelpButton("Помощь"),
    m_StatusLabel("")
{
    set_title("Linux System Monitor");
    set_default_size(800, 600);
    
    try {
        set_icon_from_file("resources/icons/app_icon.png");
    } catch (const Gtk::IconThemeError& ex) {
        std::cerr << "Icon file not found: " << ex.what() << std::endl;
    } catch (const Glib::FileError& ex) {
        std::cerr << "File error: " << ex.what() << std::endl;
    }
    
    setupStyles();
    setupProcessTreeView(); //make table   
    m_HeaderBox.set_border_width(10);
    m_HeaderBox.pack_start(m_TitleLabel, Gtk::PACK_EXPAND_WIDGET);
    m_HeaderBox.pack_start(m_RefreshButton, Gtk::PACK_SHRINK);
    m_HeaderBox.pack_start(m_HelpButton, Gtk::PACK_SHRINK);
    
    m_CommandBox.set_border_width(10);
    m_CommandBox.pack_start(m_CommandEntry, Gtk::PACK_EXPAND_WIDGET);
    m_CommandBox.pack_start(m_StartCommandButton, Gtk::PACK_SHRINK);
    m_CommandBox.pack_start(m_StopCommandButton, Gtk::PACK_SHRINK);
    m_CommandEntry.set_placeholder_text("Введите команду (например: grep 'pattern' /path/to/file)");
    m_CommandEntry.set_margin_right(10);
    
    m_SearchEntry.set_placeholder_text("Поиск процессов...");
    m_SearchEntry.set_margin_top(10);
    m_SearchEntry.set_margin_bottom(10);
    m_SearchEntry.set_margin_left(10);
    m_SearchEntry.set_margin_right(10);
    
    m_ScrolledWindow.add(m_ProcessTreeView);
    m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    
    m_OutputTextView.set_editable(false);
    m_OutputTextView.set_wrap_mode(Gtk::WRAP_WORD);
    Gtk::ScrolledWindow* outputScroll = Gtk::manage(new Gtk::ScrolledWindow());
    outputScroll->add(m_OutputTextView);
    outputScroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    
    m_StatusLabel.set_margin_top(5);
    m_StatusLabel.set_margin_bottom(5);
    
    m_ContentBox.set_border_width(10);
    m_ContentBox.pack_start(m_ScrolledWindow, Gtk::PACK_EXPAND_WIDGET);
    m_ContentBox.pack_start(*outputScroll, Gtk::PACK_EXPAND_WIDGET);
    
    m_MainBox.pack_start(m_HeaderBox, Gtk::PACK_SHRINK);
    m_MainBox.pack_start(m_SearchEntry, Gtk::PACK_SHRINK);
    m_MainBox.pack_start(m_CommandBox, Gtk::PACK_SHRINK);
    m_MainBox.pack_start(m_ContentBox, Gtk::PACK_EXPAND_WIDGET);
    m_MainBox.pack_start(m_ProgressBar, Gtk::PACK_SHRINK);
    m_MainBox.pack_start(m_StatusLabel, Gtk::PACK_SHRINK);
    m_MainBox.pack_start(m_Statusbar, Gtk::PACK_SHRINK);
    
    add(m_MainBox);
    
    m_RefreshButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onRefreshClicked));
    m_StartCommandButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onStartCommandClicked));
    m_StopCommandButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onStopCommandClicked));
    m_HelpButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onHelpClicked));
    m_SearchEntry.signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onSearchChanged));
    
    // Initial update
    updateProcessList();
    show_all_children();
}

MainWindow::~MainWindow() {}

void MainWindow::setupProcessTreeView() {
    m_ProcessListStore = Gtk::ListStore::create(m_Columns);
    m_ProcessTreeView.set_model(m_ProcessListStore);
    
    m_ProcessTreeView.append_column("PID", m_Columns.m_col_pid);
    m_ProcessTreeView.append_column("Имя", m_Columns.m_col_name);
    m_ProcessTreeView.append_column("CPU%", m_Columns.m_col_cpu);
    m_ProcessTreeView.append_column("Память%", m_Columns.m_col_memory);
    m_ProcessTreeView.append_column("Статус", m_Columns.m_col_status);
    
    for (int i = 0; i < 5; i++) {
        Gtk::TreeViewColumn* column = m_ProcessTreeView.get_column(i);
        if (column) {
            column->set_resizable(true);
            column->set_sort_column(i);
            
            auto cell_renderer = column->get_first_cell();
            if (auto text_renderer = dynamic_cast<Gtk::CellRendererText*>(cell_renderer)) {
                text_renderer->property_foreground() = "black";
                text_renderer->property_foreground_set() = true;
            }
        }
    }
}

void MainWindow::setupStyles() {
    auto css = Gtk::CssProvider::create();
    try {
        css->load_from_data(
            "window { background-color: #f5f5f5; }"
            "button { background-color: #4CAF50; color: white; border-radius: 4px; padding: 5px 10px; }"
            "button:hover { background-color: #45a049; }"
            "entry { padding: 5px; border-radius: 4px; border: 1px solid #ccc; }"
            "treeview { background-color: white; color: black; }"
            "treeview:selected { background-color: #4CAF50; color: white; }"
            "textview { background-color: white; color: black; }"
            "label { color: black; }"
            ".success { color: #4CAF50; font-weight: bold; }"
            ".error { color: #f44336; font-weight: bold; }"
        );
        get_style_context()->add_provider_for_screen(
            Gdk::Screen::get_default(), css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    } catch (const Glib::Error& ex) {
        std::cerr << "CSS error: " << ex.what() << std::endl;
    }
}

void MainWindow::onRefreshClicked() {
    m_ProgressBar.pulse();
    updateProcessList();
    m_ProgressBar.set_fraction(0.0);
    m_Statusbar.push("Список процессов обновлен");
}

void MainWindow::onStartCommandClicked() {
    std::string command = m_CommandEntry.get_text();
    if (command.empty()) return;
    
    m_OutputTextView.get_buffer()->set_text("Выполняется: " + command + "\n\n");
    m_ProgressBar.pulse();
    m_Statusbar.push("Выполнение команды...");
    
    m_StatusLabel.set_text("");
    
    m_CommandExecutor.executeCommandAsync(command, 
        sigc::mem_fun(*this, &MainWindow::onCommandFinished));
}

void MainWindow::onStopCommandClicked() {
    m_CommandExecutor.stopAllProcesses();
    m_Statusbar.push("Команды остановлены");
}

void MainWindow::onHelpClicked() {
    showHelpDialog();
}

void MainWindow::onSearchChanged() {
    updateProcessList();
}

void MainWindow::updateProcessList() {
    m_ProcessListStore->clear();
    
    std::vector<ProcessInfo> processes;
    std::string search_pattern = m_SearchEntry.get_text();
    
    if (search_pattern.empty()) {
        processes = m_ProcessMonitor.getRunningProcesses();
    } else {
        processes = m_ProcessMonitor.findProcesses(search_pattern);
    }
    
    for (const auto& process : processes) {
        Gtk::TreeModel::Row row = *(m_ProcessListStore->append());
        row[m_Columns.m_col_pid] = process.pid;
        row[m_Columns.m_col_name] = process.name;
        row[m_Columns.m_col_cpu] = process.cpu;
        row[m_Columns.m_col_memory] = process.memory;
        row[m_Columns.m_col_status] = process.status;
    }
}

void MainWindow::onCommandFinished(std::string output) {
    auto buffer = m_OutputTextView.get_buffer();
    buffer->insert(buffer->end(), "Вывод:\n" + output + "\n\n");
    m_ProgressBar.set_fraction(0.0);
    
    // Определяем успешность выполнения команды и устанавливаем соответствующий цвет
    bool success = (output.find("Error:") == std::string::npos);
    
    if (success) {
        m_StatusLabel.set_text("Команда выполнена успешно");
        m_StatusLabel.get_style_context()->add_class("success");
        m_StatusLabel.get_style_context()->remove_class("error");
    } else {
        m_StatusLabel.set_text("Ошибка выполнения команды");
        m_StatusLabel.get_style_context()->add_class("error");
        m_StatusLabel.get_style_context()->remove_class("success");
    }
    
    m_Statusbar.push("Выполнение команды завершено");
}

void MainWindow::showHelpDialog() {
    Gtk::Dialog dialog("Помощь - Системный Монитор", *this, true);
    dialog.set_default_size(500, 400);
    dialog.set_border_width(10);
    
    Gtk::Box* content_box = dialog.get_content_area();
    
    Gtk::TextView text_view;
    text_view.set_editable(false);
    text_view.set_cursor_visible(false);
    text_view.set_wrap_mode(Gtk::WRAP_WORD);
    
    Glib::RefPtr<Gtk::TextBuffer> buffer = text_view.get_buffer();
    std::string help_text = 
        "Системный Монитор Процессов\n\n"
        "Это приложение позволяет вам:\n"
        "• Мониторить запущенные процессы в вашей системе\n"
        "• Выполнять системные команды\n"
        "• Искать и фильтровать процессы\n\n"
        "Описание столбцов:\n"
        "• PID - Идентификатор процесса\n"
        "• Имя - Название процесса или команды\n"
        "• CPU% - Использование процессора в процентах\n"
        "• Память% - Использование памяти в процентах\n"
        "• Статус - Статус процесса (R=Запущен, S=Спит, и т.д.)\n\n"
        "Использование:\n"
        "• Используйте кнопку 'Обновить' для обновления списка процессов\n"
        "• Вводите текст в поле поиска для фильтрации процессов\n"
        "• Вводите команды в поле команд для их выполнения";
    
    buffer->set_text(help_text);
    
    Gtk::ScrolledWindow scrolled_window;
    scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolled_window.add(text_view);
    scrolled_window.set_size_request(480, 350);
    
    content_box->pack_start(scrolled_window, true, true, 0);
    
    dialog.add_button("OK", Gtk::RESPONSE_OK);
    
    auto css = Gtk::CssProvider::create();
    try {
        css->load_from_data(
            ".help-dialog {"
            "   background-color: #2d2d2d;"
            "   color: white;"
            "}"
            ".help-textview {"
            "   background-color: #2d2d2d;"
            "   color: white;"
            "   font-size: 12px;"
            "   padding: 10px;"
            "   font-family: 'Arial', 'Helvetica', sans-serif;"
            "}"
            ".help-button {"
            "   background-color: #4CAF50;"
            "   color: white;"
            "   border-radius: 4px;"
            "   padding: 5px 15px;"
            "   font-family: 'Arial', 'Helvetica', sans-serif;"
            "}"
            ".help-button:hover {"
            "   background-color: #45a049;"
            "}"
        );
        
        dialog.get_style_context()->add_provider(css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        dialog.get_style_context()->add_class("help-dialog");
        
        text_view.get_style_context()->add_provider(css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        text_view.get_style_context()->add_class("help-textview");
        
        auto button = dialog.get_widget_for_response(Gtk::RESPONSE_OK);
        if (button) {
            button->get_style_context()->add_provider(css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            button->get_style_context()->add_class("help-button");
        }
        
    } catch (const Glib::Error& ex) {
        std::cerr << "CSS error in help dialog: " << ex.what() << std::endl;
        
        content_box->override_background_color(Gdk::RGBA("#2d2d2d"));
        text_view.override_background_color(Gdk::RGBA("#2d2d2d"));
        text_view.override_color(Gdk::RGBA("white"));
        
        auto button = dialog.get_widget_for_response(Gtk::RESPONSE_OK);
        if (button) {
            button->override_background_color(Gdk::RGBA("#4CAF50"));
            button->override_color(Gdk::RGBA("white"));
        }
    }
    
    dialog.show_all();
    dialog.run();
}