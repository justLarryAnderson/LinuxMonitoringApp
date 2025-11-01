#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gtkmm.h>
#include "ProcessMonitor.h"
#include "CommandExecutor.h"

class MainWindow : public Gtk::Window {
public:
    MainWindow();
    virtual ~MainWindow();

private:
    // Widgets
    Gtk::Box m_MainBox;
    Gtk::Box m_HeaderBox;
    Gtk::Box m_ContentBox;
    Gtk::Box m_CommandBox;
    Gtk::Label m_TitleLabel;
    Gtk::Button m_RefreshButton;
    Gtk::Button m_StartCommandButton;
    Gtk::Button m_StopCommandButton;
    Gtk::Button m_HelpButton;  // Новая кнопка Help
    Gtk::Entry m_CommandEntry;
    Gtk::Entry m_SearchEntry;
    Gtk::ScrolledWindow m_ScrolledWindow;
    Gtk::TreeView m_ProcessTreeView;
    Gtk::TextView m_OutputTextView;
    Gtk::ProgressBar m_ProgressBar;
    Gtk::Statusbar m_Statusbar;
    Gtk::Label m_StatusLabel;  // Новый Label для цветного статуса
    
    // Models
    Glib::RefPtr<Gtk::ListStore> m_ProcessListStore;
    
    // Business logic
    ProcessMonitor m_ProcessMonitor;
    CommandExecutor m_CommandExecutor;
    
    // Signal handlers
    void onRefreshClicked();
    void onStartCommandClicked();
    void onStopCommandClicked();
    void onHelpClicked();  // Новый обработчик для Help
    void onSearchChanged();
    void updateProcessList();
    void onCommandFinished(std::string output);
    
    // UI setup
    void setupProcessTreeView();
    void setupStyles();
    void showHelpDialog();  // Новый метод для показа справки
    
    // Tree model columns
    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        ModelColumns() {
            add(m_col_pid);
            add(m_col_name);
            add(m_col_cpu);
            add(m_col_memory);
            add(m_col_status);
        }
        
        Gtk::TreeModelColumn<Glib::ustring> m_col_pid;
        Gtk::TreeModelColumn<Glib::ustring> m_col_name;
        Gtk::TreeModelColumn<Glib::ustring> m_col_cpu;
        Gtk::TreeModelColumn<Glib::ustring> m_col_memory;
        Gtk::TreeModelColumn<Glib::ustring> m_col_status;
    };
    
    ModelColumns m_Columns;
};

#endif