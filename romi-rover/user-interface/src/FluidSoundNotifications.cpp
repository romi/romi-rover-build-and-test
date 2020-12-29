/*
  romi-rover

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */

#include <r.h>
#include <string>
#include <stdexcept>
#include "FluidSoundNotifications.h"

namespace romi {

        FluidSoundNotifications::FluidSoundNotifications(JsonCpp& config)
                : _settings(0), _synth(0), _adriver(0)
        {
                std::string sound_font_file;

                try {
                        init_settings();
                        start_synth();
                        get_sound_font_file(config, sound_font_file);
                        load_sound_font(sound_font_file);
                        try_add_sounds(config);
                 
                } catch (std::runtime_error& e) {
                        r_err("FluidSoundNotification: init failed: %s", e.what());
                        clean_up();
                        throw e;
                }
        }

        FluidSoundNotifications::~FluidSoundNotifications()
        {
                clean_up();
        }
        
        void FluidSoundNotifications::clean_up()
        {
                if (_adriver)
                        delete_fluid_audio_driver(_adriver);
                if (_synth)
                        delete_fluid_synth(_synth);
                if (_settings)
                        delete_fluid_settings(_settings);
        }

        void FluidSoundNotifications::init_settings()
        {
                _settings = new_fluid_settings();
                fluid_settings_setstr(_settings, "audio.driver", "alsa");
                fluid_settings_setint(_settings, "audio.period-size", 512);
                fluid_settings_setint(_settings, "audio.periods", 4);
                fluid_settings_setnum(_settings, "synth.gain", 4.0f);
        }

        void FluidSoundNotifications::start_synth()
        {
                _synth = new_fluid_synth(_settings);
                _adriver = new_fluid_audio_driver(_settings, _synth);
        }

        void FluidSoundNotifications::get_sound_font_file(JsonCpp& config, std::string& path)
        {
                try {
                        path = (const char *) config["user-interface"]["fluid-sounds"]["sound-font"];
                } catch (JSONError& je) {
                        r_err("FluidSoundNotification: Failed to read the config: %s",
                              je.what());
                        throw je;
                }
        }

        void FluidSoundNotifications::load_sound_font(std::string& path)
        {
                r_info("FluidSoundNotification: loading soundfont %s", path.c_str());
                
                _sfont_id = fluid_synth_sfload(_synth, path.c_str(), 1);
                if (_sfont_id == FLUID_FAILED) {
                        r_err("Failed to load the soundfont: %s", path.c_str());
                        throw std::runtime_error("Failed to load the soundfont");
                }
        }

        void FluidSoundNotifications::try_add_sounds(JsonCpp& config)
        {
                try {
                        add_sounds(config);
                        
                } catch (JSONError& je) {
                        r_err("FluidSoundNotification: Failed to get "
                              "user-interface.fluid-sounds.sounds: %s", je.what());
                        throw je;
                }
        }
        
        void FluidSoundNotifications::add_sounds(JsonCpp& config)
        {
                JsonCpp sounds = config["user-interface"]["fluid-sounds"]["sounds"];
                sounds.foreach(add_sound, this);
        }
        
        int32_t FluidSoundNotifications::add_sound(const char* notification,
                                                  json_object_t value,
                                                  void *data)
        {
                FluidSoundNotifications *fluid = (FluidSoundNotifications *) data;
                if (json_isobject(value)
                    && json_object_has(value, "preset")
                    && json_object_has(value, "volume")) {
                        int preset = (int) json_object_getnum(value, "preset");
                        int volume = (int) json_object_getnum(value, "volume");
                        
                        fluid->add_sound(notification, preset, volume);
                        
                } else {
                        r_err("FluidSoundNotification: Value '%s' in list of sounds "
                                "is not valid");
                }
                return 0;
        }

        void FluidSoundNotifications::add_sound(const char *notification,
                                                int preset, int volume)
        {
                r_info("FluidSoundNotification: %s -> preset %d", notification, preset);
                _sounds.insert(std::pair<std::string, Sound>(notification, Sound(preset, volume)));
        }
                
        Sound& FluidSoundNotifications::get_sound(const char *name)
        {
                return _sounds.at(name);
        }

        void FluidSoundNotifications::notify(const char *name)
        {
                try {
                        play(name);
                } catch (...) {
                        r_warn("FluidSoundNotification: Failed to play '%s'", name);
                }
        }

        void FluidSoundNotifications::play(const char *name)
        {
                Sound& sound = get_sound(name);
                r_debug("FluidSoundNotification: play %s, preset %d, volume %d",
                        name, sound.preset, sound.volume);
                fluid_synth_program_select(_synth, 0, _sfont_id, 0, sound.preset); // TODO
                fluid_synth_noteon(_synth, 0, 60, sound.volume);
        }
        
        void FluidSoundNotifications::stop(const char *name)
        {
                fluid_synth_noteoff(_synth, 0, 60);
        }
}
