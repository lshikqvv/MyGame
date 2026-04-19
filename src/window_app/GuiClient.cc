#include <gtkmm.h>

#include <algorithm>
#include <array>
#include <fstream>
#include <map>
#include <mutex>
#include <queue>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "../connection/Socket.h"

using namespace std;
using namespace connection;

namespace {
struct I18nText {
    string ja;
    string en;
};

string read_text_file(const string& path)
{
    ifstream fin(path);
    if (!fin) {
        throw runtime_error("missing resource file: " + path);
    }
    stringstream ss;
    ss << fin.rdbuf();
    return ss.str();
}

vector<char> parse_char_array(const string& json, const string& key)
{
    regex arr_re("\"" + key + "\"\\s*:\\s*\\[(.*?)\\]");
    smatch m;
    if (!regex_search(json, m, arr_re)) {
        throw runtime_error("missing array key: " + key);
    }
    string body = m[1];
    regex char_re("\"(.)\"");
    vector<char> out;
    for (sregex_iterator it(body.begin(), body.end(), char_re), end; it != end; ++it) {
        out.push_back((*it)[1].str()[0]);
    }
    if (out.empty()) throw runtime_error("empty array key: " + key);
    return out;
}

string parse_string(const string& json, const string& key)
{
    regex str_re("\"" + key + "\"\\s*:\\s*\"([^\"]+)\"");
    smatch m;
    if (!regex_search(json, m, str_re)) {
        throw runtime_error("missing string key: " + key);
    }
    return m[1];
}

map<string, I18nText> parse_i18n(const string& json)
{
    map<string, I18nText> out;
    regex item_re("\"([^\"]+)\"\\s*:\\s*\\{\\s*\"ja\"\\s*:\\s*\"([^\"]*)\"\\s*,\\s*\"en\"\\s*:\\s*\"([^\"]*)\"\\s*\\}");
    for (sregex_iterator it(json.begin(), json.end(), item_re), end; it != end; ++it) {
        out[(*it)[1]] = {(*it)[2], (*it)[3]};
    }
    if (out.empty()) throw runtime_error("invalid i18n json");
    return out;
}

class GuiClientWindow : public Gtk::Window {
public:
    explicit GuiClientWindow(int window_index)
        : m_root(Gtk::ORIENTATION_VERTICAL),
          m_home_box(Gtk::ORIENTATION_VERTICAL),
          m_home_controls(Gtk::ORIENTATION_HORIZONTAL),
          m_game_page(Gtk::ORIENTATION_VERTICAL),
          m_top_bar(Gtk::ORIENTATION_HORIZONTAL),
          m_cards_box(Gtk::ORIENTATION_HORIZONTAL),
          m_action_bar(Gtk::ORIENTATION_HORIZONTAL),
            m_rematch_bar(Gtk::ORIENTATION_HORIZONTAL),
          m_log_area(Gtk::ORIENTATION_VERTICAL),
          m_settings_page(Gtk::ORIENTATION_VERTICAL),
            m_lang_bar(Gtk::ORIENTATION_HORIZONTAL)
    {
        load_resources();

        set_title("Poker GUI Client - Player Window " + to_string(window_index + 1));
        set_default_size(1400, 780);

        build_home_page();
        build_game_page();
        build_settings_page();

        m_notebook.append_page(m_stack, "Game");
        m_notebook.append_page(m_settings_page, "Settings");
        m_root.pack_start(m_notebook, Gtk::PACK_EXPAND_WIDGET);
        add(m_root);
        show_all_children();

        m_dispatcher.connect(sigc::mem_fun(*this, &GuiClientWindow::on_dispatch));
        update_texts();
        go_home();

        m_socket.create("127.0.0.1", 8080);
        m_status.set_text(tr("status_connected"));
        m_reader = thread(&GuiClientWindow::reader_loop, this);
    }

    ~GuiClientWindow() override
    {
        m_running = false;
        m_socket.finish();
        if (m_reader.joinable()) m_reader.join();
    }

private:
    Gtk::Box m_root;
    Gtk::Notebook m_notebook;
    Gtk::Stack m_stack;
    Gtk::Paned m_main_pane;

    Gtk::Box m_home_box;
    Gtk::Image m_home_banner;
    Gtk::Label m_home_title;
    Gtk::Box m_home_controls;
    Gtk::Label m_home_game_label;
    Gtk::ComboBoxText m_home_game_select;
    Gtk::Button m_home_start;
    Gtk::Button m_home_exit;

    Gtk::Box m_game_page;
    Gtk::Box m_top_bar;
    Gtk::Label m_status;
    Gtk::Label m_result;
    Gtk::Image m_result_banner;
    Gtk::Box m_cards_box;
    Gtk::Box m_action_bar;
    Gtk::Button m_exchange_button;
    Gtk::Label m_detail;
    Gtk::Box m_rematch_bar;
    Gtk::Button m_rematch_yes;
    Gtk::Button m_rematch_no;

    array<Gtk::Box, 5> m_card_slots;
    array<Gtk::Label, 5> m_card_labels;
    array<Gtk::EventBox, 5> m_card_click_areas;
    array<Gtk::Image, 5> m_card_images;
    array<bool, 5> m_card_selected {};

    Gtk::Box m_log_area;
    Gtk::Label m_log_title;
    Gtk::ScrolledWindow m_log_scroll;
    Gtk::TextView m_log;

    Gtk::Box m_settings_page;
    Gtk::Box m_lang_bar;
    Gtk::Label m_language_label;
    Gtk::ComboBoxText m_language_combo;

    Socket m_socket;
    thread m_reader;
    bool m_running = true;
    bool m_waiting_exchange = false;
    bool m_waiting_rematch = false;
    bool m_waiting_game_options = false;
    bool m_start_requested = false;

    Glib::Dispatcher m_dispatcher;
    mutex m_mutex;
    queue<string> m_messages;

    std::map<string, I18nText> m_i18n;
    string m_lang = "ja";
    vector<char> m_suit_order;
    vector<char> m_rank_order;
    string m_images_dir;
    string m_banner_start;
    string m_banner_win;
    string m_banner_lose;
    string m_banner_draw;

    static vector<string> split(const string& s, char delim)
    {
        vector<string> out;
        stringstream ss(s);
        string item;
        while (getline(ss, item, delim)) if (!item.empty()) out.push_back(item);
        return out;
    }

    string tr(const string& key) const
    {
        auto it = m_i18n.find(key);
        if (it == m_i18n.end()) return key;
        return (m_lang == "en") ? it->second.en : it->second.ja;
    }

    string resource_path(const string& filename) const
    {
        return m_images_dir + "/" + filename;
    }

    int index_of(const vector<char>& order, char v) const
    {
        for (size_t i = 0; i < order.size(); ++i) {
            if (order[i] == v) return static_cast<int>(i);
        }
        return -1;
    }

    void load_resources()
    {
        m_i18n = parse_i18n(read_text_file("resources/common/i18n/gui_strings.json"));
        string cfg = read_text_file("resources/poker/gui_config.json");
        m_suit_order = parse_char_array(cfg, "suits");
        m_rank_order = parse_char_array(cfg, "ranks");
        m_images_dir = parse_string(cfg, "images_dir");
        m_banner_start = parse_string(cfg, "banner_start");
        m_banner_win = parse_string(cfg, "banner_win");
        m_banner_lose = parse_string(cfg, "banner_lose");
        m_banner_draw = parse_string(cfg, "banner_draw");
    }

    void build_home_page()
    {
        m_home_box.set_spacing(16);
        m_home_controls.set_spacing(12);
        m_home_start.signal_clicked().connect(sigc::mem_fun(*this, &GuiClientWindow::on_home_start));
        m_home_exit.signal_clicked().connect(sigc::mem_fun(*this, &GuiClientWindow::on_home_exit));

        m_home_game_select.append("POKER");
        m_home_game_select.set_active_text("POKER");

        m_home_box.pack_start(m_home_title, Gtk::PACK_SHRINK);
        m_home_box.pack_start(m_home_banner, Gtk::PACK_SHRINK);
        m_home_controls.pack_start(m_home_game_label, Gtk::PACK_SHRINK);
        m_home_controls.pack_start(m_home_game_select, Gtk::PACK_SHRINK);
        m_home_controls.pack_start(m_home_start, Gtk::PACK_SHRINK);
        m_home_controls.pack_start(m_home_exit, Gtk::PACK_SHRINK);
        m_home_box.pack_start(m_home_controls, Gtk::PACK_SHRINK);

        m_stack.add(m_home_box, "home");
    }

    void build_game_page()
    {
        m_game_page.set_spacing(8);
        m_top_bar.set_spacing(8);
        m_cards_box.set_spacing(8);
        m_action_bar.set_spacing(8);
        m_rematch_bar.set_spacing(8);
        m_result_banner.set_no_show_all(true);
        m_result_banner.hide();
        m_rematch_bar.set_no_show_all(true);
        m_rematch_bar.hide();

        m_exchange_button.signal_clicked().connect(sigc::mem_fun(*this, &GuiClientWindow::on_exchange_clicked));
        m_rematch_yes.signal_clicked().connect(sigc::mem_fun(*this, &GuiClientWindow::on_rematch_yes));
        m_rematch_no.signal_clicked().connect(sigc::mem_fun(*this, &GuiClientWindow::on_rematch_no));

        for (int i = 0; i < 5; ++i) {
            m_card_slots[i].set_orientation(Gtk::ORIENTATION_VERTICAL);
            m_card_slots[i].set_spacing(4);
            m_card_labels[i].set_text(to_string(i + 1) + ": ?");
            m_card_click_areas[i].add(m_card_images[i]);
            m_card_click_areas[i].set_visible_window(true);
            m_card_click_areas[i].add_events(Gdk::BUTTON_PRESS_MASK);
            m_card_click_areas[i].signal_button_press_event().connect(
                sigc::bind(sigc::mem_fun(*this, &GuiClientWindow::on_card_clicked), i), false);

            m_card_slots[i].pack_start(m_card_labels[i], Gtk::PACK_SHRINK);
            m_card_slots[i].pack_start(m_card_click_areas[i], Gtk::PACK_EXPAND_WIDGET);
            m_cards_box.pack_start(m_card_slots[i], Gtk::PACK_EXPAND_WIDGET);
        }

        m_log.set_editable(false);
        m_log_scroll.add(m_log);
        m_log_area.pack_start(m_log_title, Gtk::PACK_SHRINK);
        m_log_area.pack_start(m_log_scroll, Gtk::PACK_EXPAND_WIDGET);

        m_top_bar.pack_start(m_status, Gtk::PACK_EXPAND_WIDGET);
        m_action_bar.pack_start(m_exchange_button, Gtk::PACK_SHRINK);
        m_action_bar.pack_start(m_result, Gtk::PACK_EXPAND_WIDGET);
        m_rematch_bar.pack_start(m_rematch_yes, Gtk::PACK_SHRINK);
        m_rematch_bar.pack_start(m_rematch_no, Gtk::PACK_SHRINK);

        m_game_page.pack_start(m_top_bar, Gtk::PACK_SHRINK);
        m_game_page.pack_start(m_result_banner, Gtk::PACK_SHRINK);
        m_game_page.pack_start(m_cards_box, Gtk::PACK_EXPAND_WIDGET);
        m_game_page.pack_start(m_action_bar, Gtk::PACK_SHRINK);
        m_game_page.pack_start(m_detail, Gtk::PACK_SHRINK);
        m_game_page.pack_start(m_rematch_bar, Gtk::PACK_SHRINK);

        m_main_pane.set_position(840);
        m_main_pane.pack1(m_game_page, true, false);
        m_main_pane.pack2(m_log_area, false, false);
        m_stack.add(m_main_pane, "game");
    }

    void build_settings_page()
    {
        m_settings_page.set_spacing(12);
        m_lang_bar.set_spacing(8);
        m_language_combo.append("ja");
        m_language_combo.append("en");
        m_language_combo.set_active_text("ja");
        m_language_combo.signal_changed().connect(sigc::mem_fun(*this, &GuiClientWindow::on_language_changed));
        m_lang_bar.pack_start(m_language_label, Gtk::PACK_SHRINK);
        m_lang_bar.pack_start(m_language_combo, Gtk::PACK_SHRINK);
        m_settings_page.pack_start(m_lang_bar, Gtk::PACK_SHRINK);
    }

    void update_texts()
    {
        m_home_title.set_text(tr("home_title"));
        m_home_game_label.set_text(tr("label_game"));
        m_home_start.set_label(tr("btn_home_start"));
        m_home_exit.set_label(tr("btn_home_exit"));
        m_log_title.set_text(tr("label_log"));
        m_exchange_button.set_label(tr("btn_exchange"));
        m_rematch_yes.set_label(tr("btn_rematch_yes"));
        m_rematch_no.set_label(tr("btn_rematch_no"));
        m_language_label.set_text(tr("label_language"));
        m_notebook.set_tab_label_text(m_stack, tr("tab_game"));
        m_notebook.set_tab_label_text(m_settings_page, tr("tab_settings"));
    }

    void show_banner_file(const string& file)
    {
        auto pix = Gdk::Pixbuf::create_from_file(resource_path(file));
        auto scaled = pix->scale_simple(680, 160, Gdk::INTERP_BILINEAR);
        m_result_banner.set(scaled);
        m_result_banner.show();
    }

    void go_home()
    {
        auto pix = Gdk::Pixbuf::create_from_file(resource_path(m_banner_start));
        auto scaled = pix->scale_simple(680, 160, Gdk::INTERP_BILINEAR);
        m_home_banner.set(scaled);
        m_stack.set_visible_child("home");
        m_result.set_text("");
        m_detail.set_text("");
        m_rematch_bar.hide();
        m_result_banner.hide();
        set_exchange_enabled(false);
    }

    void go_game()
    {
        m_stack.set_visible_child("game");
        m_result_banner.hide();
        m_rematch_bar.hide();
    }

    string image_path_for_card(const string& code) const
    {
        if (code.size() < 2) return "";
        int sidx = index_of(m_suit_order, code[1]);
        int ridx = index_of(m_rank_order, code[0]);
        if (sidx < 0 || ridx < 0) return "";
        int image_no = sidx * 13 + ridx + 1;
        return resource_path(to_string(image_no) + ".png");
    }

    void refresh_card_label(int i, const string& code)
    {
        string suffix = m_card_selected[i] ? " [*]" : "";
        m_card_labels[i].set_text(to_string(i + 1) + ": " + code + suffix);
    }

    void set_card_image(int pos, const string& code)
    {
        string path = image_path_for_card(code);
        auto pix = Gdk::Pixbuf::create_from_file(path);
        auto scaled = pix->scale_simple(140, 204, Gdk::INTERP_BILINEAR);
        m_card_images[pos].set(scaled);
        m_card_selected[pos] = false;
        refresh_card_label(pos, code);
    }

    void set_exchange_enabled(bool enabled)
    {
        m_waiting_exchange = enabled;
        m_exchange_button.set_sensitive(enabled);
        for (int i = 0; i < 5; ++i) {
            m_card_selected[i] = false;
            string txt = m_card_labels[i].get_text();
            auto p = txt.find(" [*]");
            if (p != string::npos) txt = txt.substr(0, p);
            m_card_labels[i].set_text(txt);
        }
    }

    void append_log(const string& line)
    {
        auto b = m_log.get_buffer();
        b->insert(b->end(), line + "\n");
    }

    void reader_loop()
    {
        while (m_running) {
            string line = m_socket.response();
            if (line.empty()) break;
            {
                lock_guard<mutex> lock(m_mutex);
                m_messages.push(line);
            }
            m_dispatcher.emit();
        }
    }

    void show_hand(const string& payload)
    {
        vector<string> cards = split(payload, '|');
        for (int i = 0; i < 5; ++i) {
            if (i < static_cast<int>(cards.size())) set_card_image(i, cards[i]);
        }
    }

    std::map<string, string> parse_kv_tokens(const string& payload)
    {
        std::map<string, string> out;
        stringstream ss(payload);
        string token;
        while (ss >> token) {
            auto eq = token.find('=');
            if (eq == string::npos) continue;
            out[token.substr(0, eq)] = token.substr(eq + 1);
        }
        return out;
    }

    void on_dispatch()
    {
        queue<string> local;
        {
            lock_guard<mutex> lock(m_mutex);
            swap(local, m_messages);
        }

        while (!local.empty()) {
            string line = local.front();
            local.pop();

            if (line.rfind("HELLO ", 0) == 0) {
                m_status.set_text(tr("status_connected") + " P" + line.substr(6));
                append_log("HELLO " + line.substr(6));
                continue;
            }
            if (line.rfind("GAME_OPTIONS ", 0) == 0) {
                m_waiting_game_options = true;
                if (m_start_requested) {
                    string game = m_home_game_select.get_active_text();
                    if (game.empty()) game = "POKER";
                    m_socket.request("GAME " + game);
                    m_start_requested = false;
                    m_waiting_game_options = false;
                    go_game();
                }
                continue;
            }
            if (line.rfind("HAND ", 0) == 0) {
                show_hand(line.substr(5));
                set_exchange_enabled(false);
                continue;
            }
            if (line == "REQUEST_EXCHANGE") {
                set_exchange_enabled(true);
                append_log("REQUEST_EXCHANGE");
                continue;
            }
            if (line.rfind("INFO ", 0) == 0) {
                append_log(line.substr(5));
                continue;
            }
            if (line.rfind("RESULT ", 0) == 0) {
                string r = line.substr(7);
                m_result.set_text("Result: " + r);
                if (r == "WIN") show_banner_file(m_banner_win);
                else if (r == "LOSE") show_banner_file(m_banner_lose);
                else show_banner_file(m_banner_draw);
                continue;
            }
            if (line.rfind("RESULT_DETAIL ", 0) == 0) {
                auto kv = parse_kv_tokens(line.substr(14));
                m_detail.set_text(
                    tr("detail_prefix") + "Self=" + kv["SELF_ROLE"] + "(" + kv["SELF_KEY"] + ") "
                    + "Opp=" + kv["OPP_ROLE"] + "(" + kv["OPP_KEY"] + ")"
                );
                continue;
            }
            if (line == "REMATCH_PROMPT") {
                m_waiting_rematch = true;
                m_rematch_yes.set_sensitive(true);
                m_rematch_no.set_sensitive(true);
                m_rematch_bar.show();
                continue;
            }
            if (line == "ROUND_NEXT") {
                m_waiting_rematch = false;
                m_rematch_bar.hide();
                m_result_banner.hide();
                m_result.set_text("");
                m_detail.set_text("");
                continue;
            }
            if (line == "SESSION_END") {
                m_waiting_rematch = false;
                m_rematch_bar.hide();
                go_home();
                continue;
            }
            append_log(line);
        }
    }

    bool on_card_clicked(GdkEventButton* event, int idx)
    {
        if (!event || event->type != GDK_BUTTON_PRESS || !m_waiting_exchange) return false;
        m_card_selected[idx] = !m_card_selected[idx];
        string label = m_card_labels[idx].get_text();
        auto p = label.find(": ");
        string code = (p != string::npos) ? label.substr(p + 2) : "?";
        auto q = code.find(" [*]");
        if (q != string::npos) code = code.substr(0, q);
        refresh_card_label(idx, code);
        return true;
    }

    void on_exchange_clicked()
    {
        if (!m_waiting_exchange) return;
        vector<int> selected;
        for (int i = 0; i < 5; ++i) if (m_card_selected[i]) selected.push_back(i + 1);
        if (selected.empty()) {
            m_socket.request("EXCHANGE 0");
        } else {
            stringstream ss;
            ss << "EXCHANGE";
            for (int idx : selected) ss << " " << idx;
            m_socket.request(ss.str());
        }
        set_exchange_enabled(false);
    }

    void on_rematch_yes()
    {
        if (!m_waiting_rematch) return;
        m_socket.request("REMATCH YES");
        m_waiting_rematch = false;
        m_rematch_bar.hide();
    }

    void on_rematch_no()
    {
        if (!m_waiting_rematch) return;
        m_socket.request("REMATCH NO");
        m_waiting_rematch = false;
        m_rematch_bar.hide();
    }

    void on_language_changed()
    {
        string l = m_language_combo.get_active_text();
        if (l == "ja" || l == "en") {
            m_lang = l;
            update_texts();
        }
    }

    void on_home_start()
    {
        m_start_requested = true;
        if (m_waiting_game_options) {
            string game = m_home_game_select.get_active_text();
            if (game.empty()) game = "POKER";
            m_socket.request("GAME " + game);
            m_waiting_game_options = false;
            m_start_requested = false;
            go_game();
        }
    }

    void on_home_exit()
    {
        if (auto app = get_application()) {
            app->quit();
        } else {
            hide();
        }
    }
};
}

int main(int argc, char* argv[])
{
    int clients = 2;
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--clients" && i + 1 < argc) clients = max(1, stoi(argv[++i]));
    }

    auto app = Gtk::Application::create(argc, argv, "work.jitaku.poker.gui_client");
    vector<unique_ptr<GuiClientWindow>> windows;
    bool initialized = false;

    app->signal_activate().connect([&]() {
        if (initialized) return;
        initialized = true;
        windows.reserve(clients);
        for (int i = 0; i < clients; ++i) {
            windows.push_back(make_unique<GuiClientWindow>(i));
            app->add_window(*windows.back());
            windows.back()->show();
        }
    });

    return app->run();
}
