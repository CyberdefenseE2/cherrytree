/*
 * ct_config.cc
 * 
 * Copyright 2017 giuspen <giuspen@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

// https://developer.gnome.org/glibmm/stable/classGlib_1_1KeyFile.html
// https://developer.gnome.org/glibmm/stable/group__MiscUtils.html
// g++ ct_config.cc -o ct_config `pkg-config glibmm-2.4 --cflags --libs` -Wno-deprecated

#include <assert.h>
#include <iostream>
#include <glibmm.h>

#define MAX_RECENT_DOCS 10

class CTConfig
{
public:
    CTConfig();
    virtual ~CTConfig();

    // [state]
    Glib::ustring            m_file_dir;
    Glib::ustring            m_file_name;
    bool                     m_toolbar_visible;
    bool                     m_win_is_maximized;
    int                      m_win_rect[4];
    int                      m_hpaned_pos;
    Glib::ustring            m_node_path;
    int                      m_cursor_position;
    std::list<Glib::ustring> m_recent_docs;
    Glib::ustring            m_pick_dir_import;
    Glib::ustring            m_pick_dir_export;
    Glib::ustring            m_pick_dir_file;
    Glib::ustring            m_pick_dir_img;
    Glib::ustring            m_pick_dir_csv;
    Glib::ustring            m_pick_dir_cbox;

protected:
    void _populate_with_defaults();
    bool _populate_string_from_keyfile(const Glib::ustring &group, const gchar *key, Glib::ustring *p_target);
    bool _populate_bool_from_keyfile(const Glib::ustring &group, const gchar *key, bool *p_target);
    bool _populate_int_from_keyfile(const Glib::ustring &group, const gchar *key, int *p_target);
    void _populate_from_keyfile();
    bool _check_load_from_file();
    void _unexpected_keyfile_error(const gchar *key, const Glib::KeyFileError &kferror);

    Glib::ustring m_filepath;
    Glib::KeyFile *mp_key_file;
};

CTConfig::CTConfig() : m_filepath(Glib::build_filename(Glib::get_user_config_dir(), "cherrytree", "config.cfg")),
                       mp_key_file(nullptr)
{
    _populate_with_defaults();
    bool config_found = _check_load_from_file();
    std::cout << m_filepath << " " << (config_found ? "parsed":"missing") << std::endl;
}

CTConfig::~CTConfig()
{
    if (mp_key_file != nullptr)
    {
        delete mp_key_file;
    }
}

void CTConfig::_populate_with_defaults()
{
    // [state]
    m_toolbar_visible = true;
    m_win_is_maximized = false;
    m_win_rect[0] = 10;
    m_win_rect[1] = 10;
    m_win_rect[2] = 963;
    m_win_rect[3] = 630;
    m_hpaned_pos = 170;
}

bool CTConfig::_populate_string_from_keyfile(const Glib::ustring &group, const gchar *key, Glib::ustring *p_target)
{
    bool got_it = false;
    if (mp_key_file->has_key(group, key))
    {
        try
        {
            *p_target = mp_key_file->get_value(group, key);
            got_it = true;
        }
        catch (Glib::KeyFileError &kferror)
        {
            _unexpected_keyfile_error(key, kferror);
        }
    }
    return got_it;
}

bool CTConfig::_populate_bool_from_keyfile(const Glib::ustring &group, const gchar *key, bool *p_target)
{
    bool got_it = false;
    if (mp_key_file->has_key(group, key))
    {
        try
        {
            *p_target = mp_key_file->get_boolean(group, key);
            got_it = true;
        }
        catch (Glib::KeyFileError &kferror)
        {
            if (kferror.code() == Glib::KeyFileError::Code::INVALID_VALUE)
            {
                // booleans from python ConfigParser
                Glib::ustring bool_str = mp_key_file->get_value(group, key);
                *p_target = (bool_str == "True");
                got_it = true;
            }
            else
            {
                _unexpected_keyfile_error(key, kferror);
            }
        }
    }
    return got_it;
}

bool CTConfig::_populate_int_from_keyfile(const Glib::ustring &group, const gchar *key, int *p_target)
{
    bool got_it = false;
    if (mp_key_file->has_key(group, key))
    {
        try
        {
            *p_target = mp_key_file->get_integer(group, key);
            got_it = true;
        }
        catch (Glib::KeyFileError &kferror)
        {
            _unexpected_keyfile_error(key, kferror);
        }
    }
    return got_it;
}

void CTConfig::_unexpected_keyfile_error(const gchar *key, const Glib::KeyFileError &kferror)
{
    std::cerr << "!! " << key << " error code " << kferror.code() << std::endl;
}

void CTConfig::_populate_from_keyfile()
{
    Glib::ustring curr_group("state");
    _populate_string_from_keyfile(curr_group, "file_dir", &m_file_dir);
    _populate_string_from_keyfile(curr_group, "file_name", &m_file_name);
    _populate_bool_from_keyfile(curr_group, "toolbar_visible", &m_toolbar_visible);
    _populate_bool_from_keyfile(curr_group, "win_is_maximized", &m_win_is_maximized);
    _populate_int_from_keyfile(curr_group, "win_position_x", &m_win_rect[0]);
    _populate_int_from_keyfile(curr_group, "win_position_y", &m_win_rect[1]);
    _populate_int_from_keyfile(curr_group, "win_size_w", &m_win_rect[2]);
    _populate_int_from_keyfile(curr_group, "win_size_h", &m_win_rect[3]);
    _populate_int_from_keyfile(curr_group, "hpaned_pos", &m_hpaned_pos);
    if (_populate_string_from_keyfile(curr_group, "node_path", &m_node_path))
    {
        _populate_int_from_keyfile(curr_group, "cursor_position", &m_cursor_position);
    }
    for (int i=0; i<MAX_RECENT_DOCS; i++)
    {
        Glib::ustring recent_doc;
        gchar key[10];
        snprintf(key, 10, "doc_%d", i);
        if (!_populate_string_from_keyfile(curr_group, key, &recent_doc))
        {
            break;
        }
        m_recent_docs.push_back(recent_doc);
    }
    _populate_string_from_keyfile(curr_group, "pick_dir_import", &m_pick_dir_import);
    _populate_string_from_keyfile(curr_group, "pick_dir_export", &m_pick_dir_export);
    _populate_string_from_keyfile(curr_group, "pick_dir_file", &m_pick_dir_file);
    _populate_string_from_keyfile(curr_group, "pick_dir_img", &m_pick_dir_img);
    _populate_string_from_keyfile(curr_group, "pick_dir_csv", &m_pick_dir_csv);
    _populate_string_from_keyfile(curr_group, "pick_dir_cbox", &m_pick_dir_cbox);
}

bool CTConfig::_check_load_from_file()
{
    if (Glib::file_test(m_filepath, Glib::FILE_TEST_EXISTS))
    {
        mp_key_file = new Glib::KeyFile();
        mp_key_file->load_from_file(m_filepath);
        _populate_from_keyfile();
        return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    CTConfig ct_config;
}