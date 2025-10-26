#include "MainWindow.h"
#include <iostream>

MainWindow::MainWindow() :
    m_MainBox(Gtk::ORIENTATION_VERTICAL),
    m_HeaderBox(Gtk::ORIENTATION_HORIZONTAL),
    m_ContentBox(Gtk::ORIENTATION_HORIZONTAL),
    m_CommandBox(Gtk::ORIENTATION_HORIZONTAL),
    m_TitleLabel("System Process Monitor"),
    m_RefreshButton("Refresh"),
    m_StartCommandButton("Start Command"),
    m_StopCommandButton("Stop")
{
    set_title("Linux System Monitor");
    set_default_size(800, 600);
    
    // Setup UI
    setupStyles();
    setupProcessTreeView();
    
    // Header
    m_HeaderBox.set_border_width(10);  // Исправлено
    m_HeaderBox.pack_start(m_TitleLabel, Gtk::PACK_EXPAND_WIDGET);
    m_HeaderBox.pack_start(m_RefreshButton, Gtk::PACK_SHRINK);
    
    // Command section
    m_CommandBox.set_border_width(10);  // Исправлено
    m_CommandBox.pack_start(m_CommandEntry, Gtk::PACK_EXPAND_WIDGET);
    m_CommandBox.pack_start(m_StartCommandButton, Gtk::PACK_SHRINK);
    m_CommandBox.pack_start(m_StopCommandButton, Gtk::PACK_SHRINK);
    m_CommandEntry.set_placeholder_text("Enter command (e.g., grep 'pattern' /path/to/file)");
    m_CommandEntry.set_margin_right(10);
    
    // Search
    m_SearchEntry.set_placeholder_text("Search processes...");
    m_SearchEntry.set_margin_top(10);    // Исправлено
    m_SearchEntry.set_margin_bottom(10); // Исправлено
    m_SearchEntry.set_margin_left(10);   // Исправлено
    m_SearchEntry.set_margin_right(10);  // Исправлено
    
    // Process list
    m_ScrolledWindow.add(m_ProcessTreeView);
    m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    
    // Output
    m_OutputTextView.set_editable(false);
    m_OutputTextView.set_wrap_mode(Gtk::WRAP_WORD);
    Gtk::ScrolledWindow* outputScroll = Gtk::manage(new Gtk::ScrolledWindow());
    outputScroll->add(m_OutputTextView);
    outputScroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    
    // Layout
    m_ContentBox.set_border_width(10);  // Исправлено
    m_ContentBox.pack_start(m_ScrolledWindow, Gtk::PACK_EXPAND_WIDGET);
    m_ContentBox.pack_start(*outputScroll, Gtk::PACK_EXPAND_WIDGET);
    
    m_MainBox.pack_start(m_HeaderBox, Gtk::PACK_SHRINK);
    m_MainBox.pack_start(m_SearchEntry, Gtk::PACK_SHRINK);
    m_MainBox.pack_start(m_CommandBox, Gtk::PACK_SHRINK);
    m_MainBox.pack_start(m_ContentBox, Gtk::PACK_EXPAND_WIDGET);
    m_MainBox.pack_start(m_ProgressBar, Gtk::PACK_SHRINK);
    m_MainBox.pack_start(m_Statusbar, Gtk::PACK_SHRINK);
    
    add(m_MainBox);
    
    // Signals
    m_RefreshButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onRefreshClicked));
    m_StartCommandButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onStartCommandClicked));
    m_StopCommandButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onStopCommandClicked));
    m_SearchEntry.signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onSearchChanged));
    
    // Initial update
    updateProcessList();
    show_all_children();
}

MainWindow::~MainWindow() {}

void MainWindow::setupProcessTreeView() {
    m_ProcessListStore = Gtk::ListStore::create(m_Columns);
    m_ProcessTreeView.set_model(m_ProcessListStore);
    
    // Add columns
    m_ProcessTreeView.append_column("PID", m_Columns.m_col_pid);
    m_ProcessTreeView.append_column("Name", m_Columns.m_col_name);
    m_ProcessTreeView.append_column("CPU%", m_Columns.m_col_cpu);
    m_ProcessTreeView.append_column("Memory%", m_Columns.m_col_memory);
    m_ProcessTreeView.append_column("Status", m_Columns.m_col_status);
    
    // Set column properties
    for (int i = 0; i < 5; i++) {
        Gtk::TreeViewColumn* column = m_ProcessTreeView.get_column(i);
        if (column) {
            column->set_resizable(true);
            column->set_sort_column(i);
            
            // Устанавливаем черный цвет текста для каждой колонки
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
    m_Statusbar.push("Process list updated");
}

void MainWindow::onStartCommandClicked() {
    std::string command = m_CommandEntry.get_text();
    if (command.empty()) return;
    
    m_OutputTextView.get_buffer()->set_text("Executing: " + command + "\n\n");
    m_ProgressBar.pulse();
    m_Statusbar.push("Executing command...");
    
    m_CommandExecutor.executeCommandAsync(command, 
        sigc::mem_fun(*this, &MainWindow::onCommandFinished));
}

void MainWindow::onStopCommandClicked() {
    m_CommandExecutor.stopAllProcesses();
    m_Statusbar.push("Commands stopped");
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
    buffer->insert(buffer->end(), "Output:\n" + output + "\n\n");
    m_ProgressBar.set_fraction(0.0);
    m_Statusbar.push("Command execution completed");
}